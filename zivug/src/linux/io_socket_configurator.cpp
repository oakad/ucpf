/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#include "io_socket_private.hpp"

#include <array>
#include <yesod/coder/xxhash.hpp>
#include <mina/from_ascii_decimal.hpp>
#include <zivug/linux/io_event_dispatcher.hpp>
#include <zivug/linux/io_socket_configurator.hpp>

namespace ucpf { namespace zivug { namespace io {
namespace detail {

template <typename ValueType, int Level, int OptName>
struct option {
	static void set(
		ucpf::zivug::io::descriptor &d,
		char const *val_first, char const *val_last
	)
	{
		throw std::system_error(
			ENOPROTOOPT, std::system_category()
		);
	}
};

template <int Level, int OptName>
struct option<int, Level, OptName> {
	static void set(
		ucpf::zivug::io::descriptor &d,
		char const *val_first, char const *val_last
	)
	{
		using ucpf::mina::detail::from_ascii_decimal_converter;
		int x_val(0);

		for (; val_first != val_last; ++val_first)
			if (!std::isspace(*val_first))
				break;

		if (!from_ascii_decimal_converter<int, false>::parse_signed(
			val_first, val_last, x_val
		))
			throw std::system_error(
				EINVAL, std::system_category()
			);

		auto rc(::setsockopt(
			d.native(), Level, OptName, &x_val, sizeof(x_val)
		));
		if (rc < 0)
			throw std::system_error(
				errno, std::system_category()
			);
	}
};

template <int Level>
struct option_symbols {
	static void set(
		ucpf::zivug::io::descriptor &d,
		char const *val_first, char const *val_last
	)
	{
		throw std::system_error(
			ENOPROTOOPT, std::system_category()
		);
	}
};

template <>
struct option_symbols<SOL_SOCKET> {
	constexpr static int level = SOL_SOCKET;
	constexpr static uint32_t seed = 0x13e1d776;
	constexpr static uint32_t order = 5;
	constexpr static uint32_t mask = (uint32_t(1) << order) - 1;

	struct entry {
		char const *name;
		std::size_t name_sz;
		void (*func)(
			ucpf::zivug::io::descriptor &d,
			char const *val_first, char const *val_last
		);
	};

	constexpr static std::array<
		entry, (std::size_t(1) << order)
	> symbols = {{
#include "symbols/socket_common_options.hpp"
	}};

	static void set(
		ucpf::zivug::io::descriptor &d,
		char const *val_first, char const *val_last
	)
	{
		/* option = val */
		auto *last(val_first);
		for (; last != val_last; ++last)
			if (*last == '=')
				break;

		auto x_last(last);
		for (; x_last != val_first; --x_last)
			if (!std::isspace(*x_last)) {
				++x_last;
				break;
			}

		ucpf::yesod::coder::xxhash<> h(seed);
		h.update(val_first, x_last);
		auto key(h.digest() & mask);
		auto const &e(symbols[key]);

		if (!std::equal(
			e.name, e.name + e.name_sz, val_first, last
		) || (last == val_last))
			throw std::system_error(
				ENOPROTOOPT, std::system_category()
			);
			return;

		e.func(d, last + 1, val_last);
	}
};

struct level_symbols {
	constexpr static uint32_t seed = 0x97cc1370;
	constexpr static uint32_t order = 6;
	constexpr static uint32_t mask = (uint32_t(1) << order) - 1;

	struct entry {
		char const *name;
		std::size_t name_sz;
		void (*func)(
			ucpf::zivug::io::descriptor &d,
			char const *val_first, char const *val_last
		);
	};

	constexpr static std::array<
		entry, (std::size_t(1) << order)
	> symbols = {{
#include "symbols/socket_levels.hpp"
	}};

	static void set(
		ucpf::zivug::io::descriptor &d,
		char const *val_first, char const *val_last
	)
	{
		/* socket_level.option = val */
		for (; val_first != val_last; ++val_first)
			if (!std::isspace(*val_first))
				break;

		char const *last(val_first);
		for (; last != val_last; ++last)
			if (*last == '.')
				break;

		ucpf::yesod::coder::xxhash<> h(seed);
		h.update(val_first, last);
		auto key(h.digest() & mask);
		auto const &e(symbols[key]);
		if (!std::equal(
			e.name, e.name + e.name_sz,
			val_first, last
		) || (last == val_last))
			throw std::system_error(
				ENOPROTOOPT, std::system_category()
			);

		e.func(d, last, val_last);
	}
};

struct type_symbols {
	constexpr static uint32_t seed = 0x7c1148ec;
	constexpr static uint32_t order = 3;
	constexpr static uint32_t mask = (uint32_t(1) << order) - 1;

	struct entry {
		char const *name;
		std::size_t name_sz;
		int socket_type;
	};

	constexpr static std::array<
		entry, (std::size_t(1) << order)
	> symbols = {{
		{nullptr, 0, -1},
		{"dccp", 4, SOCK_DCCP},
		{"raw", 3, SOCK_RAW},
		{"rdm", 3, SOCK_RDM},
		{"stream", 6, SOCK_STREAM},
		{"seqpacket", 9, SOCK_SEQPACKET},
		{"dgram", 5, SOCK_DGRAM},
		{"packet", 6, SOCK_PACKET}
	}};

	static int get(char const *first, char const *last)
	{
		ucpf::yesod::coder::xxhash<> h(seed);
		h.update(first, last);
		auto key(h.digest() & mask);
		auto const &e(symbols[key]);

		if (!e.name)
			throw std::system_error(
				ESOCKTNOSUPPORT, std::system_category()
			);

		return e.socket_type;
	}
};

constexpr std::array<
	type_symbols::entry, (std::size_t(1) << type_symbols::order)
> type_symbols::symbols;

template <int AddrFamily>
struct family_entry {
	constexpr static family<AddrFamily> addr_family = {};
	constexpr static family_base const *impl = &addr_family;
};

template <int AddrFamily>
constexpr family<AddrFamily> family_entry<AddrFamily>::addr_family;

struct family_symbols {
	constexpr static uint32_t seed = 0x0042b748;
	constexpr static uint32_t order = 6;
	constexpr static uint32_t mask = (uint32_t(1) << order) - 1;

	struct entry {
		char const *name;
		std::size_t name_sz;
		family_base const *addr_family;
	};

		constexpr static std::array<
		entry, (std::size_t(1) << order)
	> symbols = {{
#include "symbols/address_families.hpp"
	}};

	static family_base const *get(char const *first, char const *last)
	{
		ucpf::yesod::coder::xxhash<> h(seed);
		h.update(first, last);
		auto key(h.digest() & mask);
		auto const &e(symbols[key]);

		if (!e.addr_family)
			throw std::system_error(
				EAFNOSUPPORT, std::system_category()
			);

		return e.addr_family;
	}
};

constexpr std::array<
	family_symbols::entry, (std::size_t(1) << family_symbols::order)
> family_symbols::symbols;

}

descriptor socket_configurator::make_descriptor(
	char const *first, char const *last, void const **ctx
)
{
	/* family.type || family.type.protocol */
	for (; first != last; ++first)
		if (!std::isspace(*first))
			break;

	auto x_s0(first);
	for (; x_s0 != last; ++x_s0)
		if (*x_s0 == '.')
			break;

	if (x_s0 == last)
		throw std::system_error(EINVAL, std::system_category());

	auto x_s1(x_s0 + 1);
	for (; x_s1 != last; ++x_s1)
		if (*x_s1 == '.')
			break;

	auto fb(detail::family_symbols::get(first, x_s0));
	if (!fb)
		throw std::system_error(EINVAL, std::system_category());

	auto s_type(detail::type_symbols::get(x_s0 + 1, x_s1));

	*ctx = reinterpret_cast<void const *>(fb);

	if (x_s1 != last)
		++x_s1;

	return descriptor([fb, s_type, x_s1, last]() -> int {
		return fb->create(s_type, x_s1, last);
	});
}

void socket_configurator::apply_setting(
	descriptor &d, char const *s_first, char const *s_last,
	void const *ctx
)
{
	/* option: socket_level.option = val */
	/* bind: socket_level.option = val */
}

}}}
