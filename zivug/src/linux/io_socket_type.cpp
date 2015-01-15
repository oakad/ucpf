/*
 * Copyright (c) 2014-2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

namespace {

#include "socket_type_map.hpp"

constexpr static int registry[] = {
	SOCK_STREAM,
	SOCK_DGRAM,
	SOCK_RAW,
	SOCK_RDM,
	SOCK_SEQPACKET,
	SOCK_DCCP,
	SOCK_PACKET
};

}

namespace ucpf { namespace zivug { namespace io { namespace socket_type {

int from_string(char const *first, char const *last)
{
	auto idx(socket_type_map::find(first, last));
	if (idx)
		return registry[idx - 1];
	else
		throw std::system_error(
			ESOCKTNOSUPPORT, std::system_category()
		);
}

}}}}
