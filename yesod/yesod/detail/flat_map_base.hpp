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
#include <yesod/iterator/facade.hpp>

namespace ucpf { namespace yesod { namespace detail {

template <
	typename KeyOfValue, typename CompareF, typename Alloc,
	typename AllocPolicy
> struct flat_map_impl {
	typedef typename KeyOfValue::result_type key_type;
	typedef typename KeyOfValue::value_type value_type;
	typedef allocator::array_helper<value_type, Alloc> value_alloc;
	typedef typename value_alloc::allocator_type allocator_type;
	typedef typename value_alloc::size_type size_type;
	typedef typename value_alloc::difference_type difference_type;

	template <typename ValueType>
	struct iterator_base : iterator::facade<
		iterator_base<ValueType>, ValueType,
		std::random_access_iterator_tag
	> {
		typedef iterator::facade<
			iterator_base<ValueType>, ValueType,
			std::random_access_iterator_tag
		> base_type;

		iterator_base()
		: map(nullptr), pos(decltype(bit_index)::npos)
		{}

		template <
			typename ValueType1,
			typename = typename std::enable_if<
				std::is_convertible<
					ValueType1 *, ValueType *
				>::value
			>::type
		>
		iterator_base(iterator_base<ValueType1> const &other)
		: map(other.map), pos(other.pos)
		{}

	private:
		friend struct flat_map_impl;
		friend struct yesod::iterator::core_access;

		typedef typename std::conditional<
			std::is_const<ValueType>::value,
			flat_map_impl const,
			flat_map_impl
		>::type map_type;

		iterator_base(map_type *map_, size_type pos_)
		: map(map_), pos(pos_)
		{}

		template <typename ValueType1>
		bool equal(iterator_base<ValueType1> const &other) const
		{
			return (map == other.map) && (
				(pos == other.pos)
				|| (at_end() == other.at_end())
			);
		}

		ValueType &dereference() const
		{
			return reinterpret_cast<ValueType &>(map->data[pos]);
		}

		void increment(typename base_type::difference_type n)
		{
			do {
				pos = map->bit_index.template find_above<true>(
					pos, map->end_pos
				);
				--n;
			} while (n && map->bit_index.valid(pos));
		}

		void increment()
		{
			increment(1);
		}

		void decrement(typename base_type::difference_type n)
		{
			do {
				pos = map->bit_index.template find_below<true>(
					pos
				);
				--n;
			} while (n && pos);
		}

		void decrement()
		{
			decrement(1);
		}

		void advance(typename base_type::difference_type n)
		{
			if (n > 0)
				increment(n);
			else if (n < 0)
				decrement(-n);
		}

		bool at_end() const
		{
			return !map || (pos >= map->end_pos);
		}

		map_type *map;
		size_type pos;
	};

	typedef iterator_base<value_type> iterator;
	typedef iterator_base<value_type const> const_iterator;

	flat_map_impl(CompareF const &comp, Alloc const &a)
	: bit_index(a), data(nullptr), aux(0, comp), begin_pos(0), end_pos(0)
	{}

	~flat_map_impl()
	{
		if (data) {
			clear_data();
			value_alloc::free_s(
				bit_index.get_allocator(), data,
				std::get<0>(aux)
			);
		}
	}

	iterator end()
	{
		return iterator(this, end_pos);
	}

	const_iterator cend() const
	{
		return const_iterator(this, end_pos);
	}

	size_type size() const
	{
		return bit_index.template count<true>(begin_pos, end_pos);
	}

	void reserve(size_type cnt)
	{
		adjust_reserve(cnt);
	}

	const_iterator lower_bound(key_type const &key) const;

	iterator lower_bound(key_type const &key)
	{
		auto iter(const_cast<
			flat_map_impl const *
		>(this)->lower_bound(key));
		return iterator(this, iter.pos);
	}

	const_iterator upper_bound(key_type const &key) const;

	iterator upper_bound(key_type const &key)
	{
		auto iter(const_cast<
			flat_map_impl const *
		>(this)->upper_bound(key));
		return iterator(this, iter.pos);
	}

	iterator find(key_type const &key)
	{
		iterator iter(this, lower_bound(key).pos);
		if (iter != end()) {
			if (!key_compare(key, key_ref(data + iter.pos)))
				return iter;
		}
		return end();
	}

	const_iterator find(key_type const &key) const
	{
		const_iterator iter(this, lower_bound(key).pos);
		if (iter != end()) {
			if (!key_compare(key, key_ref(data[iter.pos])))
				return iter;
		}
		return end();
	}

	template <typename... Args>
	std::pair<iterator, bool> emplace_unique(
		const_iterator hint, Args&&... args
	);

	iterator erase(const_iterator first, const_iterator last);

private:
	static key_type const &key_ref(
		typename value_alloc::storage_type const *p
	)
	{
		return KeyOfValue::apply(
			*reinterpret_cast<value_type const *>(p)
		);
	}

	typename value_alloc::storage_type const *ptr_at(size_type pos) const
	{
		if ((pos >= begin_pos) && (pos < end_pos))
			return data + pos;
		else
			return nullptr;
	}

	void clear_data()
	{
		typename value_alloc::allocator_type alloc(
			bit_index.get_allocator()
		);

		for (
			auto pos(bit_index.template find_below<true>(end_pos));
			bit_index.valid(pos) && (pos >= begin_pos);
			pos = bit_index.template find_below<true>(pos)
		)
			value_alloc::allocator_traits::destroy(
				alloc,
				reinterpret_cast<value_type *>(&data[pos])
			);

		bit_index.reset();
	}

	size_type alloc_size() const
	{
		return std::get<0>(aux);
	}

	bool key_compare(key_type const &l, key_type const &r) const
	{
		return std::get<1>(aux)(l, r);
	}

	difference_type adjust_reserve(size_type cnt);

	dynamic_bitset<Alloc, AllocPolicy> bit_index;
	typename value_alloc::storage_type *data;
	std::tuple<size_type, CompareF> aux;
	size_type begin_pos;
	size_type end_pos;
};

}}}
#endif
