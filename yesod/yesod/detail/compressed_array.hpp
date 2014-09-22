/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_YESOD_DETAIL_COMPRESSED_ARRAY_20140922T1900)
#define UCPF_YESOD_DETAIL_COMPRESSED_ARRAY_20140922T1900

#include <yesod/detail/placement_array.hpp>

namespace ucpf { namespace yesod { namespace detail {

template <
	typename ValueType, std::size_t ApparentOrder, std::size_t Order,
	typename ValueValidPred
> struct compressed_array {
	static_assert(ApparentOrder > Order, "ApparentOrder > Order");

	constexpr static std::size_t index_order = Order;
	typedef placement_array<
		ValueType, 1 << index_order, ValueValidPred
	> array_type;
	typedef yesod::bitset<index_order * size()> index_type;
	typedef typename index_type::word_type word_type;
	constexpr static index_type::word_type index_mask
	= (word_type(1) << index_order) - 1;

	typedef typename array_type::size_type size_type;
	typedef typename array_type::pointer pointer;
	typedef typename array_type::const_pointer const_pointer;

	constexpr size_type size() const
	{
		return size_type(1) << ApparentOrder;
	}

	pointer ptr_at(size_type pos)
	{
		auto id(index.word_at(index_order * pos) & index_mask);
		return (id < items.size()) ? items.ptr_at(id) : nullptr;
	}

	const_pointer ptr_at(size_type pos) const
	{
		auto id(index.word_at(index_order * pos) & index_mask);
		return (id < items.size()) ? items.ptr_at(id) : nullptr;
	}

	template <typename Alloc, typename... Args>
	pointer emplace_at(
		Alloc const &a, size_type pos, Args&&... args
	)
	{
		auto id(index.word_at(index_order * pos));

		if ((id & index_mask) == index_mask) {
			auto x_pos(items.find_vacant(0));
			if (x_pos == items.size())
				return nullptr;

			auto rv(items.emplace_at(
				a, x_pos, std::forward<Args>(args)...
			));
			index.reset(index_order * pos, id & x_pos);
			return rv;
		} else
			return items.emplace_at(
				a, id & index_mask, std::forward<Args>(args)...
			);
	}

	template <typename Alloc>
	bool erase_at(Alloc const &a, size_type pos)
	{
		auto id(index.word_at(index_order * pos));

		if ((id & index_mask) == index_mask)
			return false;

		index.set(index_order * pos, id | index_mask);
		return items.erase_at(a, id & index_mask);
	}

	size_type find_vacant(size_type first) const
	{
		for (; first < size(); ++first) {
			if ((
				index.word_at(index_order * first)
				& index_mask
			) == index_mask)
				return first;
		}
		return size();
	}

	index_type index;
	array_type items;
};

template <
	typename ValueType, std::size_t ApparentOrder, typename ValueValidPred
> struct compressed_array<
	ValueType, ApparentOrder, ApparentOrder, ValueValidPred
> {
	constexpr static std::size_t index_order = ApparentOrder;
	typedef placement_array<
		ValueType, 1 << index_order, ValueValidPred
	> array_type;

	typedef typename array_type::size_type size_type;
	typedef typename array_type::pointer pointer;
	typedef typename array_type::const_pointer const_pointer;

	constexpr std::size_t size() const
	{
		return items.size();
	}

	pointer ptr_at(size_type pos)
	{
		return items.ptr_at(pos);
	}

	const_pointer ptr_at(size_type pos) const
	{
		return items.ptr_at(pos);
	}

	template <typename Alloc, typename... Args>
	pointer emplace_at(
		Alloc const &a, size_type pos, Args&&... args
	)
	{
		return items.emplace_at(a, pos, std::forward<Args>(args)...);
	}

	template <typename Alloc>
	bool erase_at(Alloc const &a, size_type pos)
	{
		return items.erase_at(a, pos);
	}

	size_type find_vacant(size_type first) const
	{
		return items.find_vacant(first);
	}

	array_type items;
};


}}}
#endif
