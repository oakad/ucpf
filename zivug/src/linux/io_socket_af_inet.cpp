/*
 * Copyright (c) 2014-2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#include <mina/detail/from_ascii_numeric_i.hpp>

#include <zivug/detail/ipv4_addr_parse.hpp>
#include <zivug/detail/ipv6_addr_parse.hpp>

#include <zivug/arch/io/address_family.hpp>

extern "C" {

#include <netdb.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>

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
	if (!p_sz)
		return 0;

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
struct family<AF_INET> : address_family {
	typedef ::sockaddr_in addr_type;

	virtual void bind(
		descriptor const &d, char const *addr_first,
		char const *addr_last
	) const
	{
		using zivug::detail::ipv4_addr_parse;

		auto ip_last(addr_first);
		for (; ip_last != addr_last; ++ip_last) {
			if (*ip_last == ':')
				break;
		}

		auto port_first(ip_last);
		if (port_first == addr_last)
			throw std::system_error(
				EINVAL, std::system_category()
			);


		++port_first;

		::sockaddr_in addr = {0};
		addr.sin_family = AF_INET;
		if (!(
			ipv4_addr_parse(addr.sin_addr, addr_first, ip_last)
			&& mina::detail::from_ascii_numeric_u(
				addr.sin_port, port_first, addr_last
			)
		))
			throw std::system_error(
				EINVAL, std::system_category()
			);

		addr.sin_port = htons(addr.sin_port);
		if (0 > ::bind(
			d.native(), reinterpret_cast<::sockaddr *>(&addr),
			sizeof(addr)
		))
			throw std::system_error(
				errno, std::system_category()
			);
	}

protected:
	virtual descriptor create(
		int type, char const *proto_first, char const *proto_last
	) const
	{
		int proto(inet_protocol_id(proto_first, proto_last));

		return descriptor([type, proto]() -> int {
			return ::socket(AF_INET, type, proto);
		}, static_cast<address_family const *>(this));
	}
};

template <>
struct family<AF_INET6> : address_family {
	typedef ::sockaddr_in6 addr_type;

	virtual void bind(
		descriptor const &d, char const *addr_first,
		char const *addr_last
	) const
	{
		using zivug::detail::ipv6_addr_parse;

		if ((addr_first == addr_last) || (*addr_first != '['))
			throw std::system_error(
				EINVAL, std::system_category()
			);

		++addr_first;

		auto ip_last(addr_first);
		for (; ip_last != addr_last; ++ip_last) {
			if ((*ip_last == ']') || (*ip_last == '%'))
				break;
		}

		::sockaddr_in6 addr = {0};

		if ((ip_last == addr_last) || !ipv6_addr_parse(
			addr.sin6_addr, addr_first, ip_last
		))
			throw std::system_error(
				EINVAL, std::system_category()
			);

		auto port_first(ip_last);
		if (*port_first == '%') {
			auto scope_first(port_first);
			++scope_first;


			auto scope_last(scope_first);
			for (; scope_last != addr_last; ++scope_last)
				if (*scope_last == ']')
					break;

			if (*scope_last != ']')
				throw std::system_error(
					EINVAL, std::system_category()
				);

			addr.sin6_scope_id = scope_parse(
				scope_first, scope_last
			);

			port_first = scope_last;
		}

		if ((port_first == addr_last) || (*port_first != ']'))
			throw std::system_error(
				EINVAL, std::system_category()
			);

		++port_first;
		if ((port_first == addr_last) || (*port_first != ':'))
			throw std::system_error(
				EINVAL, std::system_category()
			);
		
		++port_first;

		if (!mina::detail::from_ascii_numeric_u(
			addr.sin6_port, port_first, addr_last
		))
			throw std::system_error(
				EINVAL, std::system_category()
			);

		addr.sin6_port = htons(addr.sin6_port);
		if (0 > ::bind(
			d.native(), reinterpret_cast<::sockaddr *>(&addr),
			sizeof(addr)
		))
			throw std::system_error(
				errno, std::system_category()
			);
	}

protected:
	virtual descriptor create(
		int type, char const *proto_first, char const *proto_last
	) const
	{
		int proto(inet_protocol_id(proto_first, proto_last));

		return descriptor([type, proto]() -> int {
			return ::socket(AF_INET6, type, proto);
		}, static_cast<address_family const *>(this));
	}

private:
	static uint32_t scope_parse(
		char const *scope_first, char const *scope_last
	) {
		uint32_t scope_id(0);
		auto x_first(scope_first);
		if (mina::detail::from_ascii_numeric_u(
			scope_id, x_first, scope_last
		) && (x_first == scope_last))
			return scope_id;

		if (scope_first == scope_last)
			throw std::system_error(
				EINVAL, std::system_category()
			);

		auto s_len(scope_last - scope_first + 1);
		char s_name[s_len];
		__builtin_memcpy(s_name, scope_first, s_len - 1);
		s_name[s_len - 1] = 0;

		scope_id = ::if_nametoindex(s_name);
		if (scope_id)
			return scope_id;
		else
			throw std::system_error(
				errno, std::system_category()
			);
	}
};

}}}}
