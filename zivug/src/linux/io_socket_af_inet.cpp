/*
 * Copyright (c) 2014-2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#include <yesod/iterator/range_fill.hpp>
#include <mina/detail/from_ascii_numeric_i.hpp>

#include <zivug/detail/ipv4_address_utils.hpp>
#include <zivug/detail/ipv6_address_utils.hpp>

#include <zivug/arch/io/address_family.hpp>

extern "C" {

#include <netdb.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <arpa/inet.h>
}

namespace ucpf { namespace zivug { namespace io {

template <::sa_family_t AddrFamily>
struct address_family_inst : address_family
{};

}}}

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

namespace ucpf { namespace zivug { namespace io {

template <>
struct address_family_inst<AF_INET> : address_family {
	typedef ::sockaddr_in address_type;

	struct address : address_base {
		address()
		{
			__builtin_memset(&addr, 0, size());
		}

		virtual std::size_t size() const
		{
			return sizeof(address_type);
		}

		virtual std::size_t data(char *buf) const
		{
			__builtin_memcpy(buf, &addr, size());
			return size();
		}

		virtual std::size_t printable(
			std::function<
				void (char const *, char const *)
			> &&consumer
		) const
		{
			using zivug::detail::ipv4_in_addr_to_ascii;
			using ucpf::mina::detail::to_ascii_decimal_u;
			using ucpf::yesod::iterator::make_range_fill;

			constexpr std::size_t addr_buf_sz(INET_ADDRSTRLEN + 7);
			char addr_buf[addr_buf_sz];
			__builtin_memset(addr_buf, 0, addr_buf_sz);

			auto sink(make_range_fill(
				addr_buf, addr_buf + addr_buf_sz
			));
			ipv4_in_addr_to_ascii(sink, addr.sin_addr);
			*sink++ = ':';
			to_ascii_decimal_u<uint16_t>(sink, addr.sin_port);
			consumer(addr_buf, addr_buf + sink.distance());
			return sink.distance();
		}

	private:
		friend address_family_inst<AF_INET>;

		address(address_type const &other)
		: addr(other)
		{}

		address_type addr;
	};

	virtual void bind(
		descriptor const &d, char const *addr_first,
		char const *addr_last
	) const;

	virtual void connect(
		descriptor const &d, char const *addr_first,
		char const *addr_last
	) const;

	virtual void listen(descriptor const &d, int backlog) const;

	virtual descriptor accept(
		descriptor const &d, address_filter &flt
	) const;

protected:
	virtual descriptor create(
		int type, char const *proto_first, char const *proto_last
	) const;

private:
	static void address_parse(
		address_type &addr, char const *addr_first,
		char const *addr_last
	)
	{
		using zivug::detail::ipv4_ascii_to_in_addr;

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

		__builtin_memset(&addr, 0, sizeof(address_type));

		addr.sin_family = AF_INET;
		if (!(
			ipv4_ascii_to_in_addr(addr.sin_addr, addr_first, ip_last)
			&& mina::detail::from_ascii_numeric_u(
				addr.sin_port, port_first, addr_last
			)
		))
			throw std::system_error(
				EINVAL, std::system_category()
			);

		addr.sin_port = htons(addr.sin_port);
	}
};

void address_family_inst<AF_INET>::bind(
	descriptor const &d, char const *addr_first, char const *addr_last
) const
{
	address_type addr;
	address_parse(addr, addr_first, addr_last);

	if (0 > ::bind(
		d.native(), reinterpret_cast<::sockaddr *>(&addr), sizeof(addr)
	))
		throw std::system_error(errno, std::system_category());
}

void address_family_inst<AF_INET>::connect(
	descriptor const &d, char const *addr_first, char const *addr_last
) const
{
	printf("--1- %s\n", std::string(addr_first, addr_last).c_str());
	address_type addr;
	address_parse(addr, addr_first, addr_last);
	printf("--2- %s, %d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

	if (0 > ::connect(
		d.native(), reinterpret_cast<::sockaddr *>(&addr), sizeof(addr)
	)) {
		if (errno != EINPROGRESS)
			throw std::system_error(errno, std::system_category());
	}
}

void address_family_inst<AF_INET>::listen(
	descriptor const &d, int backlog
) const
{
	if (0 > ::listen(d.native(), backlog))
		throw std::system_error(errno, std::system_category());
}

descriptor address_family_inst<AF_INET>::accept(
	descriptor const &d, address_filter &flt
) const
{
	return descriptor([&d, &flt]() -> int {
		::socklen_t a_len(sizeof(address_type));
		address addr;
		auto n_fd(::accept4(
			d.native(), reinterpret_cast<::sockaddr *>(&addr.addr),
			&a_len, SOCK_NONBLOCK
		));

		if ((n_fd >= 0) && !flt.accept(addr)) {
			::close(n_fd);
			n_fd = -1;
		}

		return n_fd;
	});
}

descriptor address_family_inst<AF_INET>::create(
	int type, char const *proto_first, char const *proto_last
) const
{
	auto proto(inet_protocol_id(proto_first, proto_last));

	return descriptor([type, proto]() -> int {
		auto rv(::socket(AF_INET, type | SOCK_NONBLOCK, proto));
		if (rv < 0)
			throw std::system_error(errno, std::system_category());
		return rv;
	});
}

template <>
struct address_family_inst<AF_INET6> : address_family {
	typedef ::sockaddr_in6 address_type;

	struct address : address_base {
		address()
		{
			__builtin_memset(&addr, 0, size());
		}

		virtual std::size_t size() const
		{
			return sizeof(address_type);
		}

		virtual std::size_t data(char *buf) const
		{
			__builtin_memcpy(buf, &addr, size());
			return size();
		}

		virtual std::size_t printable(
			std::function<
				void (char const *, char const *)
			> &&consumer
		) const
		{
			using zivug::detail::ipv6_in6_addr_to_ascii;
			using ucpf::mina::detail::to_ascii_decimal_u;
			using ucpf::yesod::iterator::make_range_fill;

			constexpr std::size_t addr_buf_sz(
				INET6_ADDRSTRLEN + 20
			);
			char addr_buf[addr_buf_sz];
			__builtin_memset(addr_buf, 0, addr_buf_sz);

			auto sink(make_range_fill(
				addr_buf, addr_buf + addr_buf_sz
			));
			*sink++ = '[';
			ipv6_in6_addr_to_ascii(sink, addr.sin6_addr);
			if (addr.sin6_scope_id) {
				*sink++ = '%';
				to_ascii_decimal_u<uint32_t>(
					sink, addr.sin6_scope_id
				);
			}
			*sink++ = ']';
			*sink++ = ':';
			to_ascii_decimal_u<uint16_t>(sink, addr.sin6_port);
			consumer(addr_buf, addr_buf + sink.distance());
			return sink.distance();
		}

	private:
		friend address_family_inst<AF_INET6>;

		address(address_type const &other)
		: addr(other)
		{}

		address_type addr;
	};

	virtual void bind(
		descriptor const &d, char const *addr_first,
		char const *addr_last
	) const;

	virtual void connect(
		descriptor const &d, char const *addr_first,
		char const *addr_last
	) const;

	virtual void listen(descriptor const &d, int backlog) const;

	virtual descriptor accept(
		descriptor const &d, address_filter &flt
	) const;

protected:
	virtual descriptor create(
		int type, char const *proto_first, char const *proto_last
	) const;

private:
	static uint32_t scope_parse(
		char const *scope_first, char const *scope_last
	)
	{
		uint32_t scope_id(0);
		auto x_first(scope_first);
		if (mina::detail::from_ascii_numeric_u(
			scope_id, x_first, scope_last
		) && (x_first == scope_last))
			return scope_id;

		if (scope_first == scope_last)
			throw std::system_error(EINVAL, std::system_category());

		auto s_len(scope_last - scope_first + 1);
		char s_name[s_len];
		__builtin_memcpy(s_name, scope_first, s_len - 1);
		s_name[s_len - 1] = 0;

		scope_id = ::if_nametoindex(s_name);
		if (scope_id)
			return scope_id;
		else
			throw std::system_error(errno, std::system_category());
	}

	static void address_parse(
		address_type &addr, char const *addr_first, char const *addr_last
	)
	{
		using zivug::detail::ipv6_ascii_to_in6_addr;

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

		__builtin_memset(&addr, 0, sizeof(address_type));

		if ((ip_last == addr_last) || !ipv6_ascii_to_in6_addr(
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
	}
};

void address_family_inst<AF_INET6>::bind(
	descriptor const &d, char const *addr_first, char const *addr_last
) const
{
	address_type addr;
	address_parse(addr, addr_first, addr_last);

	if (0 > ::bind(
		d.native(), reinterpret_cast<::sockaddr *>(&addr), sizeof(addr)
	))
		throw std::system_error(errno, std::system_category());
}

void address_family_inst<AF_INET6>::connect(
	descriptor const &d, char const *addr_first, char const *addr_last
) const
{
	address_type addr;
	address_parse(addr, addr_first, addr_last);

	if (0 > ::connect(
		d.native(), reinterpret_cast<::sockaddr *>(&addr), sizeof(addr)
	)) {
		if (errno != EINPROGRESS)
			throw std::system_error(errno, std::system_category());
	}
}

void address_family_inst<AF_INET6>::listen(
	descriptor const &d, int backlog
) const
{
	auto rv(::listen(d.native(), backlog));
	if (rv < 0)
		throw std::system_error(errno, std::system_category());
}

descriptor address_family_inst<AF_INET6>::accept(
	descriptor const &d, address_filter &flt
) const
{
	return descriptor([&d, &flt]() -> int {
		::socklen_t a_len(sizeof(address_type));
		address addr;
		auto n_fd(::accept4(
			d.native(), reinterpret_cast<::sockaddr *>(&addr.addr),
			&a_len, SOCK_NONBLOCK
		));

		if ((n_fd >= 0) && !flt.accept(addr)) {
			::close(n_fd);
			n_fd = -1;
		}

		return n_fd;
	});
}

descriptor address_family_inst<AF_INET6>::create(
	int type, char const *proto_first, char const *proto_last
) const
{
	auto proto(inet_protocol_id(proto_first, proto_last));

	return descriptor([type, proto]() -> int {
		auto rv(::socket(AF_INET6, type | SOCK_NONBLOCK, proto));
		if (rv < 0)
			throw std::system_error(errno, std::system_category());

		return rv;
	});
}

}}}
