/*
 * Copyright (c) 2014-2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#include "io_socket_af.hpp"
#include "io_socket_so.hpp"

namespace io = ucpf::zivug::io;

namespace {

#include "symbols/socket_cmd_map.hpp"

void socket_cmd_bind(
	io::descriptor const &d, char const *first, char const *last,
	void const *ctx
)
{
	auto af(reinterpret_cast<io::detail::family_base const *>(ctx));
	af->bind(d.native(), first, last);
}

void socket_cmd_option(
	io::descriptor const &d, char const *first, char const *last,
	void const *ctx
)
{
	/* socket_level.option = val */
	auto sol_last(first);
	while ((sol_last != last) && (*sol_last != '.'))
		++sol_last;
	auto sol(io::socket_level::from_string(first, sol_last));

	auto opt_first(sol_last);
	if (opt_first != last)
		++opt_first;

	auto opt_last(opt_first);
	while (
		(opt_last != last)
		&& !std::isspace(*opt_last)
		&& (*opt_last != '=')
	)
		++opt_last;
	auto opt(sol->option_from_string(opt_first, opt_last));

	auto val_first(opt_last);
	if (val_first != last)
		++val_first;

	while ((val_first != last) && std::isspace(*val_first))
		++val_first;

	opt->set(d, val_first, last);
}

constexpr void (*registry[])(
	io::descriptor const &, char const *first, char const *last,
	void const *ctx
) = {
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
