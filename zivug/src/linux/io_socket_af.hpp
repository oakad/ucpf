/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_DBA7740ACA4573C0D286564BC18115E2)
#define HPP_DBA7740ACA4573C0D286564BC18115E2

#include <system_error>
#include <zivug/linux/io_descriptor.hpp>

namespace ucpf { namespace zivug { namespace io {
namespace detail {

struct family_base {
	virtual descriptor create(
		int type, char const *proto_first, char const *proto_last
	) const = 0;

	virtual void bind(
		descriptor const &d, char const *addr_first,
		char const *addr_last
	) const = 0;
};

template <int AddrFamily>
struct family : family_base {
	virtual descriptor create(
		int type, char const *proto_first, char const *proto_last
	) const
	{
		throw std::system_error(
			EAFNOSUPPORT, std::system_category()
		);
	}

	virtual void bind(
		descriptor const &d, char const *addr_first,
		char const *addr_last
	) const
	{
		throw std::system_error(
			EADDRNOTAVAIL, std::system_category()
		);
	}
};

}

namespace address_family {

detail::family_base const *from_string(char const *first, char const *last);

}

namespace socket_type {

int from_string(char const *first, char const *last);

}

}}}
#endif
