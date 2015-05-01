/*
 * Copyright (c) 2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_A2DAE0CD18CD54DD19386DCEA3F6B44F)
#define HPP_A2DAE0CD18CD54DD19386DCEA3F6B44F

#include <cstdio>

namespace ucpf { namespace yesod { namespace allocator {

struct klipah_default_policy {

};

template <typename T, typename Policy = klipah_default_policy>
struct klipah;

template <>
struct klipah<void, klipah_default_policy> {
	typedef std::size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef void *pointer;
	typedef void const *const_pointer;
	typedef void value_type;

	template<typename U>
	struct rebind {
		typedef debug<U> other;
	};

	typedef std::true_type propagate_on_container_copy_assignment;
	typedef std::true_type propagate_on_container_move_assignment;
	typedef std::true_type propagate_on_container_swap;


};

template <typename Policy>
struct klipah<void, Policy> {
	typedef std::size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef void *pointer;
	typedef void const *const_pointer;
	typedef void value_type;

	template<typename U>
	struct rebind {
		typedef debug<U> other;
	};

	typedef std::true_type propagate_on_container_copy_assignment;
	typedef std::true_type propagate_on_container_move_assignment;
	typedef std::true_type propagate_on_container_swap;

};

template <typename T, typename Policy>
struct klipah {
	typedef std::size_t size_type;
	typedef ptrdiff_t  difference_type;
	typedef T *pointer;
	typedef T const *const_pointer;
	typedef T &reference;
	typedef T const &const_reference;
	typedef T value_type;

	template <typename U>
	struct rebind {
		typedef debug<U> other;
	};

	typedef std::true_type propagate_on_container_copy_assignment;
	typedef std::true_type propagate_on_container_move_assignment;
	typedef std::true_type propagate_on_container_swap;

};

}}}
#endif
