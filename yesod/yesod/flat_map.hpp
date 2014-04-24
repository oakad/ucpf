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
	typename KeyType, typename ValueType, typename KeyOfValue,
	typename CompareF, typename Alloc
> struct flat_map_impl {
	typedef KeyType   key_type;
	typedef ValueType value_type;
	typedef CompareF  key_compare;
	typedef aligned_storage_t<value_type> value_storage_type;

	template <typename ValueType1>
	struct iterator_base : iterator::facade<
		iterator_base<ValueType1>, ValueType1,
		std::random_access_iterator_tag
	> {
	private:
		value_storage_type *ptr;
	};

	typedef iterator_base<value_type> iterator;
	typedef iterator_base<value_type const> const_iterator;

	flat_map_impl(Alloc const &a = Alloc())
	: bit_index(a), data(nullptr), lower(nullptr), upper(nullptr),
	  alloc_size(nullptr)
	{}

	const_iterator find(key_type const &key) const
	{
	}

	iterator find(key_type const &key)
	{
		const_iterator iter(find(key));
		return
	}

private:
	typedef allocator_array_helper<
		value_storage_type, Alloc
	> value_alloc;

	dynamic_bitset<Alloc> bit_index;
	value_storage_type *data;
	value_storage_type *lower;
	value_storage_type *upper;
	size_type alloc_size;
};

}

template <
	typename KeyType, typename T, typename CompareF, typename Alloc
> struct flat_map {
	typedef KeyType key_type;
	typedef T       mapped_type;
	typedef std::pair<key_type const, mapped_type> value_type;
private:
	struct select_first {
		static key_type const &apply(value_type const &p)
		{
			return std::get<0>(p);
		}
	};

	detail::flat_map_impl<
		key_type, value_type, select_first, CompareF, Alloc
	> base_map;
};

template <
	typename KeyType, typename CompareF, typename Alloc
> struct flat_set {

private:
	struct identity {
		static key_type const &apply(key_type const &k)
		{
			return k;
		}
	};

	detail::flat_map_impl<
		key_type, key_type, identity, CompareF, Alloc
	> base_map;
};

}}
#endif
