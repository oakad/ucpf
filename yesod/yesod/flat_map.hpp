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
	: bit_index(a), data(nullptr), begin_ptr(nullptr), end_ptr(nullptr),
	  alloc_size(nullptr)
	{}

	const_iterator cend() const
	{
		return const_iterator(this, end_pos);
	}

	const_iterator lower_bound(key_type const &key) const
	{
		if (end_pos <= begin_pos)
			return cend();

		auto c_begin(begin_pos), c_end(end_pos - 1);

		do {
			auto c_pos((c_begin + c_end) >> 1);

			if (!bit_index.test(c_pos)) {
				auto x_pos(bit_index.find_set_above(pos));
				auto p(ptr_at(x_pos));

				if (p) {
					if (key_compare(key_ref(p), key)) {
						c_begin = x_pos;
						continue;
					}

					c_end = x_pos;
					x_pos = bit_index.find_set_below(pos);
					auto q(ptr_at(x_pos));
					if (!q || key_compare(key_ref(q), key))
						return const_iterator(
							this, c_end
						);

					c_end = x_pos;
				} else {
					x_pos = bit_index.find_set_below(pos);
					p = ptr_at(x_pos);
					if (!p || key_compare(key_ref(p), key))
						return cend();

					c_end = x_pos;
				}
			} else {
				auto p(ptr_at(x_pos));
				if (key_compare(key_ref(p), key))
					c_begin = x_pos;
				else
					c_end = x_pos;
			}
		} while (c_begin != c_end);

		auto p(ptr_at(c_begin));
		return p ? const_iterator(this, c_begin) : cend();
	}

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
