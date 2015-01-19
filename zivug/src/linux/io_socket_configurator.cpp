/*
 * Copyright (c) 2014-2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#include <zivug/linux/io_socket_configurator.hpp>

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
	af->bind(d, first, last);
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
	auto af_last(first);
	while ((af_last != last) && (*af_last != '.'))
		++af_last;

	auto af(address_family::from_string(first, af_last));

	auto type_first(af_last);
	if (af_last != last)
		++type_first;
	else
		throw std::system_error(EINVAL, std::system_category());

	auto type_last(type_first);
	while ((type_last != last) && (*type_last != '.'))
		++type_last;

	auto s_type(socket_type::from_string(type_first, type_last));

	auto proto_first(type_last);
	if (proto_first != last)
		++proto_first;

	*ctx = reinterpret_cast<void const *>(af);

	return af->create(s_type, proto_first, last);
}

void socket_configurator::apply_setting(
	descriptor const &d, char const *first, char const *last,
	void const *ctx
)
{
	auto cmd_last(first);
	while (
		(cmd_last != last)
		&& !std::isspace(*cmd_last)
		&& (*cmd_last != ':')
	)
		++cmd_last;

	auto idx(socket_cmd_map::find(first, cmd_last));
	if (!idx)
		throw std::system_error(EINVAL, std::system_category());

	auto cmd(registry[idx - 1]);

	auto arg_first(cmd_last);
	while ((arg_first != last) && std::isspace(*arg_first))
		++arg_first;

	if ((arg_first != last) && (*arg_first == ':'))
		++arg_first;
	else
		throw std::system_error(EINVAL, std::system_category());

	while ((arg_first != last) && std::isspace(*arg_first))
		++arg_first;

	cmd(d, arg_first, last, ctx);
}

}}}
