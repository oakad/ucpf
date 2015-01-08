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

#include "symbols/socket_type_map.hpp"

struct socket_type {
	template <typename Iterator>
	static int get(Iterator &first, Iterator last)
	{
		auto idx(socket_type_map::find(first, last));
		if (idx)
			return values[idx - 1];
		else
			throw std::system_error(
				ESOCKTNOSUPPORT, std::system_category()
			);
	}

private:
	constexpr static int values[] = {
		SOCK_STREAM, SOCK_DGRAM, SOCK_RAW, SOCK_RDM,
		SOCK_SEQPACKET, SOCK_DCCP, SOCK_PACKET
	};
};

constexpr int socket_type::values[];

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

	++x_s0;
	auto s_type(detail::socket_type::get(x_s0, x_s1));

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
