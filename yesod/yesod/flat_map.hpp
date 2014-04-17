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

struct value_wrapper<typename T>

template <
	typename KeyType, typename ValueType, typename CompareF, typename Alloc
> struct flat_map_impl {
	typedef KeyType   key_type;
	typedef std::tuple<KeyType, ValueType> value_type;
	typedef CompareF  key_compare;

	flat_map_impl()
	: valid_pos(nullptr), keys(nullptr), values(nullptr), alloc_length(0)
	{}

private:
	typedef aligned_storage_t<value_type> value_storage_type;

	typedef allocator_array_helper<
		value_storage_type, Alloc
	> value_alloc;

	dynamic_bitset<Alloc> valid;
	value_storage_type *values;
	size_type alloc_length;
};

}

template <
	typename KeyType, typename ValueType, typename CompareF, typename Alloc
> struct flat_map {

private:
	detail::flat_map_impl<
		KeyType, ValueType, CompareF, Alloc
	> base_map;
};

template <
	typename KeyType, typename CompareF, typename Alloc
> struct flat_set {

private:
	detail::flat_map_impl<
		KeyType, mpl::void_, CompareF, Alloc
	> base_map;
};

}}
#endif
