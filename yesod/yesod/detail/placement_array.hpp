/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_YESOD_DETAIL_PLACEMENT_ARRAY_20140115T1150)
#define UCPF_YESOD_DETAIL_PLACEMENT_ARRAY_20140115T1150

#include <yesod/bitset.hpp>
#include <yesod/allocator/array_helper.hpp>

namespace ucpf { namespace yesod { namespace detail {

template <
	typename ValueType, std::size_t N, typename ValueValidPred = void
> struct placement_array {
	typedef ValueType value_type;
	typedef typename std::aligned_storage<
		sizeof(value_type), std::alignment_of<value_type>::value
	>::type storage_type;

	typedef value_type &reference;
	typedef value_type const &const_reference;
	typedef value_type *pointer;
	typedef value_type const *const_pointer;
	typedef std::size_t size_type;

	constexpr static bool is_pod_container
	= std::is_pod<value_type>::value;

	template <typename Alloc>
	void init(Alloc const &a)
	{
		items.reset();
		init_type::init(a, *this);
	}

	template <typename Alloc>
	void destroy(Alloc const &a)
	{
		typedef allocator::array_helper<value_type, Alloc> a_h;

		if (is_pod_container)
			a_h::destroy(a, items.data, size(), false);
		else {
			items.for_each_set(
				[this, &a](size_t pos) -> void {
					a_h::destroy(
						a, &items.data[pos], 1, false
					);
				}
			);
			items.reset();
		}
	}

	constexpr size_type size() const
	{
		return N;
	}

	reference operator[](size_type pos)
	{
		return reinterpret_cast<reference>(items.data[pos]);
	}

	const_reference operator[](size_type pos) const
	{
		return reinterpret_cast<const_reference>(items.data[pos]);
	}

	pointer ptr_at(size_type pos)
	{
		return (
			items.test(pos) && value_valid_pred::test((*this)[pos])
		) ? &(*this)[pos] : nullptr;
	}

	const_pointer ptr_at(size_type pos) const
	{
		return (
			items.test(pos) && value_valid_pred::test((*this)[pos])
		) ? &(*this)[pos] : nullptr;
	}

	template <typename Alloc, typename... Args>
	pointer emplace_at(
		Alloc const &a, size_type pos, Args&&... args
	)
	{
		typedef allocator::array_helper<value_type, Alloc> a_h;

		if (items.test(pos))
			(*this)[pos] = std::move(
				value_type(std::forward<Args>(args)...)
			);
		else {
			a_h::make_n(
				a, items.data + pos, 1,
				std::forward<Args>(args)...
			);
			items.set(pos);
		}

		return &(*this)[pos];
	}

	template <typename Alloc>
	bool erase_at(Alloc const &a, size_type pos)
	{
		typedef allocator::array_helper<value_type, Alloc> a_h;

		if (items.test(pos)) {
			a_h::destroy(a, &(*this)[pos], 1, false);
			items.reset(pos);
			return true;
		} else
			return false;
	}

	bool set_valid(size_type pos)
	{
		auto rv(
			items.test(pos)
			&& value_valid_pred::test((*this)[pos])
		);
		items.set(pos);
		return rv;
	}

	void reset_valid(size_type pos)
	{
		items.reset(pos);
	}

	size_type find_vacant(size_type first) const
	{
		for (size_type pos(first); pos < size(); ++pos) {
			if (!(
				items.test(pos)
				&& value_valid_pred::test((*this)[pos])
			))
				return pos;
		}
		return size();
	}

	size_type find_occupied(size_type first) const
	{
		for (size_type pos(first); pos < size(); ++pos) {
			if (
				items.test(pos)
				&& value_valid_pred::test((*this)[pos])
			)
				return pos;
		}
		return size();
	}

	template <typename Pred>
	bool for_each(size_type first, Pred &&pred)
	{
		for (
			size_type pos(items.find_first_set(first));
			pos < size();
			pos = items.find_first_set(pos + 1)
		) {
			if (value_valid_pred::test((*this)[pos])) {
				if (pred(pos, (*this)[pos]))
					return true;
			}
		}

		return false;
	}

	template <typename Pred>
	bool for_each(size_type first, Pred &&pred) const
	{
		for (
			size_type pos(items.find_first_set(first));
			pos < size();
			pos = items.find_first_set(pos + 1)
		) {
			if (value_valid_pred::test((*this)[pos])) {
				if (pred(pos, (*this)[pos]))
					return true;
			}
		}

		return false;
	}

private:
	struct trivial_pred {
		constexpr static bool test(value_type const &v)
		{
			return true;
		}
	};

	typedef typename std::conditional<
		std::is_same<ValueValidPred, void>::value,
		trivial_pred, ValueValidPred
	>::type value_valid_pred;

	struct dummy_bitset {
		constexpr void set(size_type pos)
		{
		}

		constexpr void reset(size_type pos)
		{
		}

		constexpr void reset()
		{
		}

		constexpr bool test(size_type pos) const
		{
			return true;
		}

		constexpr size_t find_first_set(size_type first) const
		{
			return first;
		}

		template <typename Pred>
		void for_each_set(Pred &&pred) const
		{
			for (size_type pos(0); pos < N; ++pos)
				pred(pos);
		}
	};

	struct default_init {
		template <typename Alloc>
		static void init(Alloc const &a, placement_array &self)
		{
			typedef allocator::array_helper<value_type, Alloc> a_h;
			a_h::make_n(a, self.items.data, self.size());
		}
	};

	struct dummy_init {
		template <typename Alloc>
		static void init(Alloc const &a, placement_array &self)
		{}
	};

	typedef typename std::conditional<
		is_pod_container, default_init, dummy_init
	>::type init_type;

	struct item_type : std::conditional<
		is_pod_container, dummy_bitset, bitset<N>
	>::type {
		storage_type data[N];
	} items;
};

}}}
#endif
