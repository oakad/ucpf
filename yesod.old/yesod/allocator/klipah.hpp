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

struct klipah_policy_a1 {
	constexpr static std::size_t min_size_order = 3;
	constexpr static std::size_t small_size_class_cnt = 64;
	constexpr static std::size_t max_small_size_order = 18;
	constexpr static std::size_t small_size_incr_order_delta = 6;
	constexpr static std::size_t medium_size_delta
	= max_small_size_order - small_size_incr_order_delta;
	constexpr static std::size_t max_medium_size_order = 23;
	constexpr static std::size_t large_size_delta
	= Policy::max_medium_size_order - Policy::max_small_size_order;
	constexpr static std::size_t large_size_bin_count
	= std::size_t(1) << max_order;
	constexpr static std::size_t medium_map_node_leaf_count = 101;
};

typedef klipah_policy_a1 klipah_default_policy;

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
