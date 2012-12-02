/*
 * Copyright (C) 2012 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(_PARDES_ORDERED_ARRAY_HPP)
#define _PARDES_ORDERED_ARRAY_HPP

#include <cstddef>
#include <memory>
#include <algorithm>
#include <iterator>
#include <initializer_list>
#include <stdexcept>
#include <stdio.h>

namespace pardes
{

template <
	typename value_type_, std::size_t capacity,
	typename value_compare_ = std::less<value_type_>
> struct ordered_array {
	typedef value_type_      value_type;
	typedef value_type       &reference;
	typedef value_type const &const_reference;
	typedef value_type       *pointer;
	typedef value_type const *const_pointer;
	typedef std::size_t      size_type;
	typedef ptrdiff_t        difference_type;
	typedef value_compare_   value_compare;
	typedef __gnu_cxx::__normal_iterator<
		pointer, ordered_array
	> mutable_iterator;
	typedef __gnu_cxx::__normal_iterator<
		const_pointer, ordered_array
	> const_iterator;
	typedef const_iterator iterator;
	typedef std::reverse_iterator<iterator> reverse_iterator;
	typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

	ordered_array() : valid_size(0)
	{}

	ordered_array(value_compare const &comp_) : comp(comp_), valid_size(0)
	{}

	template <typename input_iter_t>
	ordered_array(input_iter_t first, input_iter_t last) : valid_size(0)
	{
		insert(first, last);
	}

	template <typename input_iter_t>
	ordered_array(input_iter_t first, input_iter_t last,
		      value_compare const &comp_)
	: comp(comp_), valid_size(0)
	{
		insert(first, last);
	}

	ordered_array(ordered_array const &other)
	: comp(other.comp), valid_size(other.valid_size)
	{
		std::uninitialized_copy(other.cbegin(), other.cend(),
					mutable_begin());
	}

	ordered_array(ordered_array &&other)
	: comp(other.comp), valid_size(other.valid_size)
	{
		std::move(
			other.mutable_begin(), other.mutable_end(),
			std::raw_storage_iterator<
				mutable_iterator, value_type
			>(mutable_begin())
		);
	}

	ordered_array(std::initializer_list<value_type> l,
		      value_compare const &comp_ = value_compare())
	: comp(comp_), valid_size(0)
	{
		insert(l);
	}

	ordered_array &operator=(ordered_array const &other)
	{
		clear();
		comp = other.comp;
		valid_size = other.valid_size;
		std::copy(
			other.cbegin(), other.cend(),
			std::raw_storage_iterator<
				mutable_iterator, value_type
			>(mutable_begin())
		);
	}

	ordered_array &operator=(ordered_array &&other)
	{
		clear();
		swap(other);
		return *this;
	}

	ordered_array &operator=(std::initializer_list<value_type> l)
	{
		clear();
		insert(l);
		return *this;
	}

	~ordered_array()
	{
		clear();
	}

	iterator begin() const
	{
		return cbegin();
	}

	iterator end() const
	{
		return cend();
	}

	reverse_iterator rbegin() const
	{
		return crbegin();
	}

	reverse_iterator rend() const
	{
		return crend();
	}

	const_iterator cbegin() const
	{
		return const_iterator(
			reinterpret_cast<value_type const *>(&data[0])
		);
	}

	const_iterator cend() const
	{
		return const_iterator(
			reinterpret_cast<value_type const *>(&data[valid_size])
		);
	}

	const_reverse_iterator crbegin() const
	{
		return const_reverse_iterator(cend());
	}

	const_reverse_iterator crend() const
	{
		return const_reverse_iterator(cbegin());
	}

	value_compare value_comp() const
	{
		return comp;
	}

	bool empty() const
	{
		return !valid_size;
	}

	size_type size() const
	{
		return valid_size;
	}

	size_type max_size() const
	{
		return capacity;
	}

	void swap(ordered_array &other)
	{
		auto common(std::min(valid_size, other.valid_size));
		ordered_array *src(this), *dst(&other);

		if (other.valid_size > valid_size) {
			src = &other;
			dst = *this;
		}

		std::swap_ranges(
			src->mutable_begin(), src->mutable_begin() + common,
			dst->mutable_begin()
		);

		std::move(
			src->mutable_begin() + common, src->mutable_end(),
			std::raw_storage_iterator<
				mutable_iterator, value_type
			>(dst->mutable_begin() + common)
		);

		std::swap(comp, other.comp);
		std::swap(valid_size, other.valid_size);
	}

	const_iterator insert(value_type const &v)
	{
		capacity_check(1);

		auto pos(std::upper_bound(
			 mutable_begin(), mutable_end(), v, comp
		));

		if (pos == mutable_end()) {
			std::uninitialized_copy_n(&v, 1, mutable_end());
			++valid_size;
		} else {
			*std::raw_storage_iterator<
				mutable_iterator, value_type
			>(mutable_end()) = std::move(*(mutable_end() - 1));
			std::move_backward(
				pos, mutable_end() - 1, mutable_end()
			);
			++valid_size;
			*pos = v;
		}
		return pos;
	}

	const_iterator insert(value_type &&v)
	{
		capacity_check(1);

		auto pos(std::upper_bound(
			 mutable_begin(), mutable_end(), v, comp
		));

		if (pos == mutable_end()) {
			*std::raw_storage_iterator<
				mutable_iterator, value_type
			>(mutable_end()) = std::move(v);
			++valid_size;
		} else {
			*std::raw_storage_iterator<
				mutable_iterator, value_type
			>(mutable_end()) = std::move(*(mutable_end() - 1));
			std::move_backward(
				pos, mutable_end() - 1, mutable_end()
			);
			++valid_size;
			*pos = std::move(v);
		}
		return pos;
	}

	template <typename input_iter_t>
	void insert(input_iter_t first, input_iter_t last)
	{
		std::for_each(
			first, last, [this](value_type const &v) -> void {
				this->insert(v);
			}
		);
	}

	void insert(std::initializer_list<value_type> l)
	{
		for (auto &v : l)
			insert(v);
	}

	const_iterator erase(const_iterator pos)
	{
		return erase(pos, pos + 1);
	}

	size_type erase(value_type const &v)
	{
		auto r(equal_range(v));
		size_type rv(r.second - r.first);

		if (rv)
			erase(r.first, r.second);

		return rv;
	}

	const_iterator erase(const_iterator first, const_iterator last)
	{
		auto new_size(valid_size - (last - first));
		mutable_iterator f(const_cast<pointer>(first.base()));
		mutable_iterator l(const_cast<pointer>(last.base()));

		const_iterator rv(std::move_backward(
			l, mutable_end(), mutable_iterator(&(*this)[new_size])
		));

		for (auto pos(valid_size - 1); pos >= new_size; --pos)
			(*this)[pos].~value_type();

		valid_size = new_size;
		return rv;
	}

	void clear()
	{
		for(size_type pos(0); pos < valid_size; ++pos)
			(*this)[pos].~value_type();

		valid_size = 0;
	}

	value_type const &operator[](size_type pos) const
	{
		return reinterpret_cast<value_type const &>(data[pos]);
	}

	value_type const &at(size_type pos) const
	{
		range_check(pos);
		return (*this)[pos];
	}

	value_type const &front() const
	{
		return *cbegin();
	}

	value_type const &back() const
	{
		return *(cend() - 1);
	}

	size_type count(value_type const &v)
	{
		auto range(equal_range(v));
		return range.second - range.first;
	}

	const_iterator find(value_type const &v) const
	{
		auto pos(std::lower_bound(cbegin(), cend(), v, comp));

		if (pos != cend() && *pos == v)
			return pos;
		else
			return cend();
	}

	const_iterator lower_bound(value_type const &v) const
	{
		return std::lower_bound(cbegin(), cend(), v, comp);
	}

	const_iterator upper_bound(value_type const &v) const
	{
		return std::upper_bound(cbegin(), cend(), v, comp);
	}

	std::pair<const_iterator, const_iterator>
	equal_range(value_type const &v) const
	{
		return std::equal_range(cbegin(), cend(), v, comp);
	}

private:
	mutable_iterator mutable_begin()
	{
		return mutable_iterator(
			reinterpret_cast<value_type *>(&data[0])
		);
	}

	mutable_iterator mutable_end()
	{
		return mutable_iterator(
			reinterpret_cast<value_type *>(&data[valid_size])
		);
	}

	value_type &operator[](size_type pos)
	{
		return reinterpret_cast<value_type &>(data[pos]);
	}

	void range_check(size_type pos) const
	{
		if (pos >= valid_size)
			throw std::out_of_range("ordered_array::range_check");
	}

	void capacity_check(size_type inc) const
	{
		if (valid_size + inc > capacity)
			throw std::length_error(
				"ordered_array::capacity_check"
			);
	}

	value_compare comp;
	size_type valid_size;
	typename std::aligned_storage<
		sizeof(value_type), std::alignment_of<value_type>::value
	>::type data[capacity];
};

}

#endif
