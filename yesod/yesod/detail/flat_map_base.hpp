/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_YESOD_DETAIL_FLAT_MAP_BASE_20140430T1620)
#define UCPF_YESOD_DETAIL_FLAT_MAP_BASE_20140430T1620

#include <yesod/dynamic_bitset.hpp>

namespace ucpf { namespace yesod { namespace detail {

template <
	typename KeyType, typename ValueType, typename KeyOfValue,
	typename CompareF, typename Alloc, typename AllocPolicy
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
		friend struct flat_map_impl;
		friend struct yesod::iterator::core_access;

		typedef typename std::conditional<
			std::is_const<ValueType1>::value,
			flat_map_impl const,
			flat_map_impl
		>::type map_type;

		iterator_base(map_type *map_, size_type pos_)
		: map(map_), pos(pos_)
		{}

		map_type *map;
		size_type pos;
	};

	typedef iterator_base<value_type> iterator;
	typedef iterator_base<value_type const> const_iterator;

	flat_map_impl(Alloc const &a = Alloc())
	: bit_index(a), data(nullptr), begin_pos(0), end_pos(0), alloc_size(0)
	{}

	const_iterator cend() const
	{
		return const_iterator(this, end_pos);
	}

	size_type size() const
	{
		return bit_index.count(begin_pos, end_pos);
	}

	void reserve(size_type cnt);

	const_iterator lower_bound(key_type const &key) const;

	template <typename... Args>
	std::pair<iterator, bool> emplace_unique(Args&&... args);

private:
	typedef allocator_array_helper<
		value_storage_type, Alloc
	> value_alloc;

	static key_type const &key_ref(value_storage_type const *p)
	{
		return KeyOfValue()(*reinterpret_cast<value_type const *>(p));
	}

	value_storage_type const *ptr_at(size_type pos) const
	{
		if (((data + pos) >= begin_ptr) && ((data + pos) < end_ptr))
			return data + pos;
		else
			return nullptr;
	}

	dynamic_bitset<Alloc, AllocPolicy> bit_index;
	value_storage_type *data;
	size_type begin_pos;
	size_type end_pos;
	size_type alloc_size;
};

}}}
#endif
