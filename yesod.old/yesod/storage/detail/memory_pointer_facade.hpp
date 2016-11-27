/*
 * Copyright (c) 2016 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_320E3B8FEFCB5DF0FD6E60B5CB133C24)
#define HPP_320E3B8FEFCB5DF0FD6E60B5CB133C24

namespace ucpf { namespace yesod { namespace storage { namespace detail {

struct memory_pointer_facade {
	typedef std::uint8_t *address_type;
	typedef std::size_t size_type;
	typedef std::ptrdiff_t difference_type;

	static constexpr bool is_stateful = false;

	address_type access(
		address_type p, size_type unit_size
	)
	{
		return p;
	}

	address_type access(
		address_type p, size_type unit_size, difference_type offset
	)
	{
		return p + unit_size * offset;
	}
};

}}}}

#endif
