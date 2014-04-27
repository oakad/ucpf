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
	: bit_index(a), data(nullptr), begin_ptr(nullptr), end_ptr(nullptr),
	  alloc_size(nullptr)
	{}

	const_iterator find(key_type const &key) const
	{
		auto c_pos((begin_pos + end_pos) >> 1);
		auto cr(range_for_pos(c_pos));
		if (cr.first == cr.second) {
			auto p(ptr_at(cr.first));
			if (!p)
				return const_iterator(data + end_pos);

			
		} else {
			auto pb(ptr_at(cr.first)), pe(ptr_at(cr.end));
		}
	}

	iterator find(key_type const &key)
	{
		const_iterator iter(find(key));
		return iterator(iter.ptr);
	}

private:
	typedef allocator_array_helper<
		value_storage_type, Alloc
	> value_alloc;

	value_storage_type const *ptr_at(size_type pos) const
	{
		if (((data + pos) >= begin_ptr) && ((data + pos) < end_ptr))
			return data + pos;
		else
			return nullptr;
	}

	std::pair<size_type, size_type> range_for_pos(size_type pos) const
	{
		if (bit_index.test(pos))
			return std::make_pair(pos, pos);
		else
			return std::make_pair(
				bit_index.find_set_below(pos),
				bit_index.find_set_above(pos)
			);
	}

	dynamic_bitset<Alloc> bit_index;
	value_storage_type *data;
	size_type begin_pos;
	size_type end_pos;
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
