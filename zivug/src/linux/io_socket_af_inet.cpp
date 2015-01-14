/*
 * Copyright (c) 2014-2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#include "io_socket_af.hpp"

extern "C" {

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

}

namespace {

std::pair<int, bool> inet_protocol_id_impl(
	char const *p_name, std::size_t buf_sz
)
{
	char buf[buf_sz];
	::protoent p_entry, *rv;

	auto rc(::getprotobyname_r(p_name, &p_entry, buf, buf_sz, &rv));
	if (rc == ERANGE)
		return std::make_pair(-1, true);
	else if (!rc && rv)
		return std::make_pair(rv->p_proto, false);
	else
		throw std::system_error(
			rc, std::system_category()
		);
}

int inet_protocol_id(char const *proto_first, char const *proto_last)
{
	auto p_sz(proto_last - proto_first);
	char p_name[p_sz + 1];
	__builtin_memcpy(p_name, proto_first, p_sz);
	p_name[p_sz] = 0;
	std::size_t buf_sz(128);

	while (true) {
		auto p(inet_protocol_id_impl(p_name, buf_sz));
		if (!p.second)
			return p.first;
		buf_sz = (buf_sz * 3) >> 1;
	}
}

}

namespace ucpf { namespace zivug { namespace io { namespace detail {

template <>
struct family<AF_INET> : family_base {
	virtual int create(
		int type, char const *first, char const *last
	) const
	{
		int proto((first != last) ? inet_protocol_id(first, last) : 0);

		return ::socket(AF_INET, type, proto);
	}

	virtual void bind(
		int fd, char const *first, char const *last
	) const
	{
	}
};

template <>
struct family<AF_INET6> : family_base {
	virtual int create(
		int type, char const *first, char const *last
	) const
	{
		int proto((first != last) ? inet_protocol_id(first, last) : 0);

		return ::socket(AF_INET6, type, proto);
	}

	virtual void bind(
		int fd, char const *first, char const *last
	) const
	{
	}
};

}}}}
