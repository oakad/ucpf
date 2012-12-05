/*
 * Copyright (C) 2012 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(_PARDES_INTERNAL_RESTRICTED_ARRAY_HPP)
#define _PARDES_INTERNAL_RESTRICTED_ARRAY_HPP

#include <boost/iterator/iterator_facade.hpp>
#include <memory>
#include <algorithm>
#include <initializer_list>
#include <stdexcept>

namespace pardes
{

template <typename value_type_, std::size_t capacity>
struct restricted_array {
	typedef value_type_      value_type;
	typedef value_type       &reference;
	typedef value_type const &const_reference;
	typedef value_type       *pointer;
	typedef value_type const *const_pointer;
	typedef std::size_t      size_type;
	typedef ptrdiff_t        difference_type;

	template <typename iter_type>
	struct iterator_base : public boost::iterator_facade <
		iterator_base<iter_type>, iter_type,
		boost::random_access_traversal_tag
	> {
	private:
		friend struct restricted_array;
		friend struct boost::iterator_core_access;
		struct enabler {};

	public:
		iterator_base() = default;

		iterator_base(iter_type *p) : base_ptr(p) {}

		template <typename iter_type1>
		iterator_base(
			iterator_base<iter_type1> const &other,
			typename std::enable_if<
				std::is_convertible<
					iter_type1 *, iter_type *
				>::value, enabler
			>::type = enabler()
		) : base_ptr(other.base_ptr)
		{}

	private:
		template <typename iter_type1>
		bool equal(iterator_base<iter_type1> const &other) const
		{
			return base_ptr == other.base_ptr;
		}

		void increment()
		{
			++base_ptr;
		}

		void decrement()
		{
			--base_ptr;
		}

		void advance(difference_type n)
		{
			base_ptr += n;
		}

		difference_type distance_to(iterator_base const &other) const
		{
			return other.base_ptr - base_ptr;
		}

		iter_type &dereference() const
		{
			return *base_ptr;
		}

		iter_type *base_ptr;
	};

	typedef iterator_base<value_type> iterator;
	typedef iterator_base<value_type const> const_iterator;
	typedef std::reverse_iterator<iterator> reverse_iterator;
	typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

	iterator begin()
	{
		return iterator(
			reinterpret_cast<value_type *>(&data[0])
		);
	}

	iterator end()
	{
		return iterator(
			reinterpret_cast<value_type *>(&data[valid_size])
		);
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

	reverse_iterator rbegin()
	{
		return reverse_iterator(end());
	}

	reverse_iterator rend()
	{
		return reverse_iterator(begin());
	}

	const_reverse_iterator crbegin() const
	{
		return const_reverse_iterator(cend());
	}

	const_reverse_iterator crend() const
	{
		return const_reverse_iterator(cbegin());
	}

	restricted_array() : valid_size(0) {}

	template <typename input_iter_t>
	restricted_array(input_iter_t first, input_iter_t last)
	: valid_size(0)
	{
		insert(cend(), first, last);
	}

	restricted_array(restricted_array const &other)
	: valid_size(other.valid_size)
	{
		std::uninitialized_copy(other.cbegin(), other.cend(), begin());
	}

	restricted_array(restricted_array &&other)
	: valid_size(other.valid_size)
	{
		std::move(
			other.begin(), other.end(),
			std::raw_storage_iterator<iterator, value_type>(begin())
		);
	}

	restricted_array(std::initializer_list<value_type> l)
	: valid_size(0)
	{
		insert(end(), l);
	}

	restricted_array &operator=(restricted_array const &other)
	{
		clear();
		valid_size = other.valid_size;
		std::copy(
			other.cbegin(), other.cend(),
			std::raw_storage_iterator<iterator, value_type>(begin())
		);
	}

	restricted_array &operator=(restricted_array &&other)
	{
		clear();
		swap(other);
		return *this;
	}

	restricted_array &operator=(std::initializer_list<value_type> l)
	{
		clear();
		insert(l);
		return *this;
	}

	~restricted_array()
	{
		clear();
	}

	bool empty() const
	{
		return !valid_size;
	}

	bool full() const
	{
		return valid_size == capacity;
	}

	size_type size() const
	{
		return valid_size;
	}

	size_type max_size() const
	{
		return capacity;
	}

	void swap(restricted_array &other)
	{
		auto common(std::min(valid_size, other.valid_size));
		restricted_array *src(this), *dst(&other);

		if (other.valid_size > valid_size) {
			src = &other;
			dst = *this;
		}

		std::swap_ranges(
			src->begin(), src->begin() + common, dst->begin()
		);

		std::move(
			src->begin() + common, src->end(),
			std::raw_storage_iterator<iterator, value_type>(
				begin() + common
			)
		);

		std::swap(valid_size, other.valid_size);
	}

	iterator insert(const_iterator pos, value_type const &v)
	{
		capacity_check(1);
		iterator rv(const_cast<value_type *>(pos.base_ptr));

		if (pos == cend())
			*std::raw_storage_iterator<iterator, value_type>(end())
			= v;
		else {
			*std::raw_storage_iterator<iterator, value_type>(end())
			= std::move(*(end() - 1));

			std::move_backward(rv, end() - 1, end());
			*rv = v;
		}
		++valid_size;
		return rv;
	}

	iterator insert(const_iterator pos, value_type &&v)
	{
		capacity_check(1);
		iterator rv(const_cast<value_type *>(pos.base_ptr));

		if (pos == end())
			*std::raw_storage_iterator<iterator, value_type>(end())
			= std::move(v);
		else {
			*std::raw_storage_iterator<iterator, value_type>(end())
			= std::move(*(end() - 1));

			std::move_backward(rv, end() - 1, end());
			*rv = std::move(v);
		}
		++valid_size;
		return rv;
	}

	template <typename input_iter_t>
	iterator insert(const_iterator pos, input_iter_t first,
			input_iter_t last)
	{
		size_type n(last - first);
		capacity_check(n);
		iterator rv(const_cast<value_type *>(pos.base_ptr));

		if (pos == cend())
			std::uninitialized_copy(first, last, end());
		else {
			size_type m(cend() - pos);
			auto k(std::min(m, n));

			std::uninitialized_copy(
				cend() - k, cend(), end() + n - k
			);

			if (n > m) {
				std::copy(
					first, last - n + k, rv
				);
				std::uninitialized_copy(
					last - n + k, last, end()
				);
			} else if (m >= n) {
				std::move_backward(
					rv, end() - k, end()
				);
				std::copy(first, last, rv);
			}
		}
		valid_size += n;
		return rv;
	}

	iterator insert(const_iterator pos, std::initializer_list<value_type> l)
	{
		return insert(pos, l.begin(), l.end());
	}

	template <typename... arg_type>
	iterator emplace(const_iterator pos, arg_type&&... args)
	{
		capacity_check(1);
		iterator rv(const_cast<value_type *>(pos.base_ptr));

		if (pos == cend())
			::new (rv.base_ptr) value_type(
				std::forward<arg_type>(args)...
			);
		else {
			*std::raw_storage_iterator<iterator, value_type>(end())
			= std::move(*(end() - 1));

			std::move_backward(rv, end() - 1, end());
			rv.base_ptr->~value_type();
			::new (rv.base_ptr) value_type(
				std::forward<arg_type>(args)...
			);
		}
		++valid_size;
		return rv;
	}

	iterator erase(const_iterator pos)
	{
		return erase(pos, pos + 1);
	}

	iterator erase(const_iterator first, const_iterator last)
	{
		iterator d(end() - (last - first));
		iterator l(const_cast<value_type *>(last.base_ptr));

		iterator rv(std::move_backward(l, end(), d));

		std::for_each(d, end(), [this](reference v) {v.~value_type();});

		valid_size = d - begin();
		return rv;
	}

	void clear()
	{
		for(size_type pos(0); pos < valid_size; ++pos)
			(*this)[pos].~value_type();

		valid_size = 0;
	}

	reference operator[](size_type pos)
	{
		return reinterpret_cast<value_type &>(data[pos]);
	}

	const_reference operator[](size_type pos) const
	{
		return reinterpret_cast<value_type const &>(data[pos]);
	}

	reference at(size_type pos)
	{
		range_check(pos);
		return (*this)[pos];
	}

	const_reference at(size_type pos) const
	{
		range_check(pos);
		return (*this)[pos];
	}

	reference front()
	{
		return (*this)[0];
	}

	const_reference front() const
	{
		return (*this)[0];
	}

	reference back()
	{
		return (*this)[valid_size > 0 ? valid_size - 1 : 0];
	}

	value_type const &back() const
	{
		return (*this)[valid_size > 0 ? valid_size - 1 : 0];
	}

private:
	void range_check(size_type pos) const
	{
		if (pos >= valid_size)
			throw std::out_of_range(
				"restricted_array::range_check"
			);
	}

	void capacity_check(size_type inc) const
	{
		if (valid_size + inc > capacity)
			throw std::length_error(
				"restricted_array::capacity_check"
			);
	}

	size_type valid_size;
	typename std::aligned_storage<
		sizeof(value_type), std::alignment_of<value_type>::value
	>::type data[capacity];
};

}

#endif


