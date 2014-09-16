/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_YESOD_DETAIL_PLACEMENT_ARRAY_JAN_15_2014_1150)
#define UCPF_YESOD_DETAIL_PLACEMENT_ARRAY_JAN_15_2014_1150

#include <array>
#include <tuple>

#include <yesod/bitset.hpp>
#include <yesod/allocator/array_helper.hpp>

namespace ucpf { namespace yesod { namespace detail {

template <
	typename ValueType, size_t N, typename ValueValidPred = void
> struct placement_array {
	typedef ValueType value_type;
	typedef typename std::aligned_storage<
		sizeof(value_type), std::alignment_of<value_type>::value
	>::type storage_type;

	typedef typename value_type &reference;
	typedef typename value_type const &const_reference;
	typedef typename value_type *pointer;
	typedef typename value_type const *const_pointer;
	typedef typename allocator_traits::void_pointer void_pointer;
	typedef typename size_t size_type;

	typedef pointer iterator;
	typedef const_pointer const_iterator;

	constexpr static bool is_pod_container
	= std::is_pod<value_type>::value;

	template <typename Alloc>
	placement_array(Alloc const &a)
	{
		typedef array_helper<value_type, Alloc> a_h;

		std::get<1>(items).reset();
		if (is_pod_container)
			ah::make_n(a, std::get<0>(items).data(), size());
	}

	template <typename Alloc>
	void destroy(Alloc const &a)
	{
		typedef array_helper<value_type, Alloc> a_h;

		if (is_pod_container)
			a_h::destroy(
				a, &std::get<0>(items)[0], size(), false
			);
		else {
			std::get<1>(items).for_each_set(
				[this, &a](size_t pos) -> void {
					a_h::destroy(
						a, &std::get<0>(items)[pos],
						1, false
					);
				}
			);
			std::get<1>(items).reset();
		}
	}

	constexpr size_type size() const
	{
		return N;
	}

	reference operator[](size_type pos)
	{
		return reinterpret_cast<reference>(std::get<0>(items[pos]));
	}

	const_reference operator[](size_type pos) const
	{
		return reinterpret_cast<const_reference>(
			std::get<0>(items[pos])
		);
	}

	pointer ptr_at(size_type pos)
	{
		return (
			std::get<1>(items).test(pos)
			&& value_valid_pred::test((*this)[pos])
		) ? &(*this)[pos] : nullptr;
	}

	const_pointer ptr_at(size_type pos) const
	{
		return (
			std::get<1>(items).test(pos)
			&& value_valid_pred::test((*this)[pos])
		) ? &(*this)[pos] : nullptr;
	}

	template <typename Alloc, typename... Args>
	reference emplace_at(Alloc const &a, size_type pos, Args&&... args)
	{
		typedef array_helper<value_type, Alloc> a_h;

		if (std::get<1>(items).test(pos))
			(*this)[pos] = std::move(
				value_type(std::forward<Args>(args)...)
			);
		else {
			a_h::make_n(
				a, &(*this)[pos], 1,
				std::forward<Args>(args)...
			);
			std::get<1>(items).set(pos);
		}

		return (*this)[pos];
	}

	template <typename Alloc>
	void erase_at(Alloc const &a, size_type pos)
	{
		typedef array_helper<value_type, Alloc> a_h;

		if (std::get<1>(items).test(pos)) {
			a_h::destroy(a, &(*this)[pos], 1, false);
			std::get<1>(items).reset(pos);
		}
	}

	const_iterator find_vacant(size_type first) const
	{
		for (size_type pos(first); pos < size(); ++pos) {
			if (!(
				std::get<1>(items).test(pos)
				|| value_valid_pred::test((*this)[pos])
			))
				return const_iterator(&(*this)[pos];
		}
		return cend();
	}

	template <typename Pred>
	bool for_each(size_type first, Pred &&pred)
	{
		for (
			size_type pos(std::get<1>(items).find_first_set(first));
			pos < size();
			pos = std::get<1>(items).find_first_set(pos + 1)
		) {
			if (value_valid_pred::test((*this)[pos])
				pred(pos, (*this)[pos]);
		}
	}

	template <typename Pred>
	bool for_each(size_type first, Pred &&pred) const
	{
		for (
			size_type pos(std::get<1>(items).find_first_set(first));
			pos < size();
			pos = std::get<1>(items).find_first_set(pos + 1)
		) {
			if (value_valid_pred::test((*this)[pos])
				pred(pos, (*this)[pos]);
		}
	}

	iterator begin()
	{
		return iterator(&(*this)[0]);
	}

	const_iterator begin() const
	{
		return const_iterator(&(*this)[0]);
	}

	const_iterator cbegin() const
	{
		return const_iterator(&(*this)[0]);
	}

	iterator end()
	{
		return iterator(&(*this)[N]);
	}

	const_iterator end() const
	{
		return const_iterator(&(*this)[N]);
	}

	const_iterator cend() const
	{
		return const_iterator(&(*this)[N]);
	}

private:
	struct null_pred {
		constexpr static bool test(value_type const &v)
		{
			return true;
		}
	};

	typedef typename std::conditional<
		std::is_same<ValueValidPred, void>::value,
		null_pred, ValueValidPred
	> value_valid_pred;

	struct null_bitset {
		constexpr void set(size_t pos)
		{
		}

		constexpr void reset(size_t pos)
		{
		}

		constexpr bool test(size_t pos) const
		{
			return true;
		}

		constexpr size_t find_first_set(size_t first) const
		{
			return first;
		}
	};

	std::tuple<
		std::array<storage_type, N>,
		typename std::conditional<
			is_pod_container, null_bitset, bitset<N>
		>::type
	> items;
};

}}}
#endif
