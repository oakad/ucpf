/*
 * Copyright (c) 2014-2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

namespace {

#include "symbols/socket_cmd_map.hpp"

void socket_cmd_bind(
	descriptor const &, char const *first, char const *last,
	void const *ctx
)
{
	auto af(reinterpret_cast<detail::family_base const *>(ctx));
	af->bind(d.native(), first, last);
}

void socket_cmd_option(
	descriptor const &, char const *first, char const *last,
	void const *ctx
)
{
	/* socket_level.option = val */
	
}

constexpr void (*registry)(
	descriptor const &, char const *first, char const *last,
	void const *ctx
) const = {
	socket_cmd_bind,
	socket_cmd_option
};

}

namespace ucpf { namespace zivug { namespace io {

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

	auto af(address_family::from_string(first, x_s0));

	++x_s0;
	auto s_type(socket_type::from_string(x_s0, x_s1));

	*ctx = reinterpret_cast<void const *>(af);

	if (x_s1 != last)
		++x_s1;

	return af->create(s_type, x_s1, last);
}

void socket_configurator::apply_setting(
	descriptor const &d, char const *first, char const *last,
	void const *ctx
)
{

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
