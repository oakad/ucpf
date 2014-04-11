/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_YESOD_FLAT_MAP_20140411T1915)
#define UCPF_YESOD_FLAT_MAP_20140411T1915

#include <yesod/detail/allocator_utils.hpp>

namespace ucpf { namespace yesod {
namespace detail {

template <
	typename KeyType, typename ValueType, typename CompareF, typename Alloc
> struct flat_map_impl {
	typedef KeyType   key_type;
	typedef ValueType value_type;
	typedef CompareF  key_compare;

	typedef typename std::allocator_traits<
		Alloc
	>::template rebind_alloc<key_type> key_allocator_type;

	typedef typename std::allocator_traits<
		Alloc
	>::template rebind_traits<key_type> key_allocator_traits;

	typedef typename key_allocator_type::reference       key_reference;
	typedef typename key_allocator_type::const_reference const_key_reference;
	typedef typename key_allocator_traits::pointer       key_pointer;
	typedef typename key_allocator_traits::const_pointer const_key_pointer;

	typedef typename std::allocator_traits<
		Alloc
	>::template rebind_alloc<value_type> allocator_type;

	typedef typename std::allocator_traits<
		Alloc
	>::template rebind_traits<value_type> allocator_traits;

	typedef typename allocator_type::reference       reference;
	typedef typename allocator_type::const_reference const_reference;
	typedef typename allocator_traits::size_type     size_type;
	typedef typename allocator_traits::pointer       pointer;
	typedef typename allocator_traits::const_pointer const_pointer;

	flat_map_impl()
	: valid_pos(nullptr), keys(nullptr), values(nullptr), alloc_length(0)
	{}

private:
	typedef unsigned long bitset_type;

	typedef typename std::allocator_traits<
		Alloc
	>::template rebind_alloc<bitmap_type> bitset_allocator_type;

	typedef typename std::allocator_traits<
		Alloc
	>::template rebind_traits<bitmap_type> bitset_allocator_traits;

	bitset_type *valid_pos;
	detail::aligned_storage_t<
		typename std::conditional<
			std::is_same<ValueType, void>::value,
			std::tuple<key_type>,
			std::tuple<key_type, value_type>
		>::type
	> *items;
	size_type alloc_length;
};

}

}}
#endif
