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
	typedef uintptr_t word_type;
	constexpr static std::size_t index_order = Order;
	constexpr static std::size_t bit_count = index_order << ApparentOrder;
	constexpr static std::size_t word_bits = sizeof(word_type) * 8;
	constexpr static std::size_t word_count
	= bit_count / word_bits + (bit_count % word_bits ? 1 : 0);
	constexpr static word_type index_mask
	= (word_type(1) << index_order) - 1;
	constexpr static word_type upper_index_mask
	= index_mask << (word_bits - index_order);

	static_assert(Order >= 2, "Order >= 2");
	static_assert(ApparentOrder > Order, "ApparentOrder > Order");
	static_assert(word_bits >= Order, "word_bits >= Order");

	typedef placement_array<
		ValueType, (1 << index_order) - 1, ValueValidPred
	> array_type;

	typedef typename array_type::size_type size_type;
	typedef typename array_type::pointer pointer;
	typedef typename array_type::const_pointer const_pointer;

	template <typename Alloc>
	void init(Alloc const &a)
	{
		for (size_type c(0); c < word_count; ++c)
			index[c] = ~word_type(0);

		items.init(a);
	}

	template <typename Alloc, std::size_t OtherOrder>
	void init_move(
		Alloc const &a, compressed_array<
			ValueType, ApparentOrder, OtherOrder,
			ValueValidPred
		> &other
	)
	{
		init(a);
		for(
			size_type pos(other.find_occupied(0));
			pos < other.size(); pos = other.find_occupied(++pos)
		)
			emplace(
				a, pos,
				std::move_if_noexcept(*other.ptr_at(pos))
			);
	}

	template <typename Alloc>
	void destroy(Alloc const &a)
	{
		items.destroy(a);
	}

	constexpr size_type size() const
	{
		return size_type(1) << ApparentOrder;
	}

	constexpr static size_type storage_size()
	{
		return array_type::storage_size();
	}

	pointer ptr_at(size_type pos)
	{
		auto id(get_index(pos));
		return (id != index_mask) ? items.ptr_at(id) : nullptr;
	}

	const_pointer ptr_at(size_type pos) const
	{
		auto id(get_index(pos));
		return (id != index_mask) ? items.ptr_at(id) : nullptr;
	}

	pointer unsafe_ptr_at(size_type pos)
	{
		return items.unsafe_ptr_at(get_index(pos));
	}

	const_pointer unsafe_ptr_at(size_type pos) const
	{
		return items.unsafe_ptr_at(get_index(pos));
	}

	std::pair<pointer, bool> reserve_at(size_type pos)
	{
		auto id(get_index(pos));
		if (id == index_mask) {
			auto x_pos(items.find_vacant(0));
			if (x_pos == items.size())
				return std::make_pair(nullptr, false);

			items.set_valid(x_pos);
			set_index(pos, x_pos);
			return std::make_pair(
				items.unsafe_ptr_at(x_pos), false
			);
		} else
			return std::make_pair(
				items.unsafe_ptr_at(id), items.set_valid(id)
			);
	}

	void release_at(size_type pos)
	{
		auto id(get_index(pos));
		if (id != index_mask) {
			items.reset_valid(id);
			set_index(pos, index_mask);
		}
	}

	template <typename Alloc, typename... Args>
	pointer emplace(
		Alloc const &a, size_type pos, Args&&... args
	)
	{
		auto id(get_index(pos));

		if (id == index_mask) {
			auto x_pos(items.find_vacant(0));
			if (x_pos == items.size())
				return nullptr;

			auto rv(items.emplace(
				a, x_pos, std::forward<Args>(args)...
			));
			if (rv)
				set_index(pos, x_pos);

			return rv;
		} else {
			auto rv(items.emplace(
				a, id, std::forward<Args>(args)...
			));
			if (!rv)
				set_index(pos, index_mask);

			return rv;
		}
	}

	template <typename Alloc>
	bool erase_at(Alloc const &a, size_type pos)
	{
		auto id(get_index(pos));

		if (id == index_mask)
			return false;

		set_index(pos, index_mask);
		return items.erase_at(a, id);
	}

	size_type find_vacant(size_type first) const
	{
		auto x_first(find_occupied(first));
		for (; x_first < size(); x_first = find_occupied(first)){
			if (x_first > first)
				return first;

			first = x_first + 1;
		}

		return x_first > first ? first : size();
	}

	size_type find_occupied(size_type first) const
	{
		auto first_bit(first * index_order);
		auto w_pos(first_bit / word_bits);
		auto w_off(first_bit % word_bits);

		if (w_pos >= word_count)
			return size();

		auto w(~index[w_pos] & ~((word_type(1) << w_off) - 1));
		while(true) {
			if (w) {
				first_bit = yesod::ffs(w) - 1
					    + w_pos * word_bits;

				return first_bit / index_order;
			}

			if (w_pos == (word_count - 1))
				return size();

			w = ~index[++w_pos];
		}
	}

	size_type count() const
	{
		return items.count();
	}

	word_type get_index(size_type pos) const
	{
		auto b_pos(pos * index_order);
		auto e_pos(b_pos + index_order - 1);

		auto w1((
			index[b_pos / word_bits] >> (b_pos % word_bits)
		) & index_mask);
		auto w2((
			index[e_pos / word_bits]
			<< (word_bits - e_pos % word_bits - 1)
		) & upper_index_mask);

		return w1 | (w2 >> (word_bits - index_order));
	}

	void set_index(size_type pos, word_type value)
	{
		auto b_pos(pos * index_order);
		auto e_pos(b_pos + index_order - 1);

		auto w1(value << (b_pos % word_bits));
		auto m1(index_mask << (b_pos % word_bits));
		index[b_pos / word_bits] &= ~m1;
		index[b_pos / word_bits] |= w1;

		auto w2(value << (word_bits - index_order));
		w2 >>= word_bits - e_pos % word_bits - 1;
		auto m2(upper_index_mask >> (
			word_bits - e_pos % word_bits - 1
		));
		index[e_pos / word_bits] &= ~m2;
		index[e_pos / word_bits] |= w2;
	}

	word_type index[word_count];
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

	template <typename Alloc>
	void init(Alloc const &a)
	{
		items.init(a);
	}

	template <typename Alloc>
	void init_at(Alloc const &a, size_type pos)
	{
		items.init_at(a, pos);
	}

	template <typename Alloc, std::size_t OtherOrder>
	void init_move(
		Alloc const &a, compressed_array<
			ValueType, ApparentOrder, OtherOrder,
			ValueValidPred
		> &other
	)
	{
		init(a);
		for(
			size_type pos(other.find_occupied(0));
			pos < other.size(); pos = other.find_occupied(++pos)
		)
			emplace(
				a, pos,
				std::move_if_noexcept(*other.ptr_at(pos))
			);
	}

	template <typename Alloc>
	void destroy(Alloc const &a)
	{
		items.destroy(a);
	}

	constexpr std::size_t size() const
	{
		return items.size();
	}

	constexpr static size_type storage_size()
	{
		return array_type::storage_size();
	}

	pointer ptr_at(size_type pos)
	{
		return items.ptr_at(pos);
	}

	const_pointer ptr_at(size_type pos) const
	{
		return items.ptr_at(pos);
	}

	pointer unsafe_ptr_at(size_type pos)
	{
		return items.unsafe_ptr_at(pos);
	}

	const_pointer unsafe_ptr_at(size_type pos) const
	{
		return items.unsafe_ptr_at(pos);
	}

	std::pair<pointer, bool> reserve_at(size_type pos)
	{
		return std::make_pair(
			items.unsafe_ptr_at(pos), items.set_valid(pos)
		);
	}

	void release_at(size_type pos)
	{
		items.reset_valid(pos);
	}

	template <typename Alloc, typename... Args>
	pointer emplace(
		Alloc const &a, size_type pos, Args&&... args
	)
	{
		return items.emplace(a, pos, std::forward<Args>(args)...);
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

	size_type find_occupied(size_type first) const
	{
		return items.find_occupied(first);
	}

	size_type count() const
	{
		return items.count();
	}

	array_type items;
};

}}}
#endif
