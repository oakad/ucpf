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

struct string_tag {
};

template <typename ValueType>
struct ro_tag {
	typedef ValueType value_type;
};

struct option_base {
	virtual void set(
		descriptor const &d, char const *first, char const *last
	) const = 0;
};

template <typename ValueType, int Level, int OptName>
struct option : option_base {
	virtual void set(
		descriptor const &d, char const *first, char const *last
	) const
	{
		throw std::system_error(
			ENOPROTOOPT, std::system_category()
		);
	}
};

template <int Level, int OptName>
struct option<int, Level, OptName> : option_base {
	virtual void set(
		descriptor const &d, char const *first, char const *last
	) const
	{
		using ucpf::mina::detail::from_ascii_decimal_converter;
		int x_val(0);

		if (!from_ascii_decimal_converter<int, false>::parse_signed(
			first, last, x_val
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

template <int Level, int OptName>
struct option<string_tag, Level, OptName> : option_base {
	virtual void set(
		descriptor const &d, char const *first, char const *last
	) const
	{
		auto rc(::setsockopt(
			d.native(), Level, OptName, first, last - first
		));

		if (rc < 0)
			throw std::system_error(
				errno, std::system_category()
			);
	}
};

template <typename ValueType, int Level, int OptName>
struct option_entry {
	constexpr static option<ValueType, Level, OptName> sock_option = {};
	constexpr static option_base const *impl = &sock_option;
};

template <typename ValueType, int Level, int OptName>
constexpr option<ValueType, Level, OptName> option_entry<
	ValueType, Level, OptName
>::sock_option;

struct option_level_base {
	virtual void set(
		descriptor const &d, char const *first, char const *last
	) const = 0;
};

template <int Level>
struct option_level : option_level_base {
	virtual void set(
		descriptor const &d, char const *first, char const *last
	) const
	{
		throw std::system_error(
			ENOPROTOOPT, std::system_category()
		);
	}
};

template <>
struct option_level<SOL_SOCKET> : option_level_base {
	constexpr static int level = SOL_SOCKET;
	constexpr static uint32_t seed = 0x01e8;
	constexpr static uint32_t order = 7;
	constexpr static uint32_t mask = (uint32_t(1) << order) - 1;

	struct entry {
		char const *name;
		std::size_t name_sz;
		option_base const *sock_option;
	};

	constexpr static std::array<
		entry, (std::size_t(1) << order)
	> symbols = {{
#include "symbols/sol_socket_options.hpp"
	}};

	virtual void set(
		descriptor const &d, char const *first, char const *last
	) const
	{
		/* option = val */
		auto name_last(first);
		for (; name_last != last; ++name_last)
			if (*name_last == '=')
				break;

		if (name_last == last)
			throw std::system_error(
				EINVAL, std::system_category()
			);

		auto val_first(name_last + 1);
		for (; val_first != last; ++val_first)
			if (!std::isspace(*val_first))
				break;

		if (val_first == last)
			throw std::system_error(
				EINVAL, std::system_category()
			);

		for (; name_last > first; --name_last)
			if (!std::isspace(*(name_last - 1)))
				break;

		if (name_last == first)
			throw std::system_error(
				EINVAL, std::system_category()
			);

		ucpf::yesod::coder::xxhash<> h(seed);
		h.update(first, name_last);
		auto key(h.digest() & mask);
		auto const &e(symbols[key]);

		if (!(e.name && std::equal(
			e.name, e.name + e.name_sz, first, name_last
		)))
			throw std::system_error(
				ENOPROTOOPT, std::system_category()
			);

		e.sock_option->set(d, val_first, last);
	}
};

template <int Level>
struct option_level_entry {
	constexpr static option_level<Level> sock_option_level = {};
	constexpr static option_level_base const *impl = &sock_option_level;
};

template <int Level>
constexpr option_level<Level> option_level_entry<Level>::sock_option_level;

struct level_symbols {
	constexpr static uint32_t seed = 0x97cc1370;
	constexpr static uint32_t order = 6;
	constexpr static uint32_t mask = (uint32_t(1) << order) - 1;

	struct entry {
		char const *name;
		std::size_t name_sz;
		option_level_base const *sock_level;
	};

	constexpr static std::array<
		entry, (std::size_t(1) << order)
	> symbols = {{
#include "symbols/socket_levels.hpp"
	}};

	static void set(
		ucpf::zivug::io::descriptor const &d,
		char const *first, char const *last
	)
	{
		/* socket_level.option = val */
		auto pref_last(first);
		for (; pref_last != last; ++pref_last)
			if (*pref_last == '.')
				break;

		if ((last - pref_last) < 3)
			throw std::system_error(
				EINVAL, std::system_category()
			);
		
		ucpf::yesod::coder::xxhash<> h(seed);
		h.update(first, pref_last);
		auto key(h.digest() & mask);
		auto const &e(symbols[key]);
		if (!(e.name && std::equal(
			e.name, e.name + e.name_sz,
			first, pref_last
		)))
			throw std::system_error(
				ENOPROTOOPT, std::system_category()
			);

		e.sock_level->set(d, pref_last + 1, last);
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
	descriptor const &d, char const *first, char const *last,
	void const *ctx
)
{
	/* option: socket_level.option = val */
	/* bind: val */
	auto cmd_last(first);
	for (; cmd_last != last; ++cmd_last)
		if (*cmd_last == ':')
			break;

	if ((last - cmd_last) < 5)
		throw std::system_error(EINVAL, std::system_category());

	auto val_first(cmd_last + 1);
	for (; val_first != last; ++val_first)
		if (!std::isspace(*val_first))
			break;

	if (val_first == last)
		throw std::system_error(EINVAL, std::system_category());

	for (; cmd_last > first; --cmd_last)
		if (!std::isspace(*(cmd_last - 1)))
			break;

	switch (*first) {
	case 'b': {
		constexpr static char const *name = "bind";
		if (std::equal(name, name + 4, first, cmd_last)) {
			auto fb(reinterpret_cast<detail::family_base const *>(
				ctx
			));
			fb->bind(d.native(), val_first, last);
			return;
		} else
			break;
	}
	case 'o': {
		constexpr static char const *name = "option";
		if (std::equal(name, name + 6, first, cmd_last)) {
			detail::level_symbols::set(d, val_first, last);
			return;
		} else
			break;
	}
	}

	throw std::system_error(EINVAL, std::system_category());
}

}}}
