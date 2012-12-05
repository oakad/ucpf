/*
 * Copyright (C) 2012 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(_PARDES_ORDERED_ARRAY_HPP)
#define _PARDES_ORDERED_ARRAY_HPP

#include <pardes/internal/restricted_array.hpp>

namespace pardes
{

template <
	typename value_type_, std::size_t capacity,
	typename value_compare_ = std::less<value_type_>
> struct ordered_array {
	typedef restricted_array<value_type_, capacity> base_type;

	typedef typename base_type::value_type        value_type;
	typedef typename base_type::reference         reference;
	typedef typename base_type::const_iterator    const_reference;
	typedef typename base_type::pointer           pointer;
	typedef typename base_type::const_pointer     const_pointer;
	typedef typename base_type::size_type         size_type;
	typedef typename base_type::difference_type   difference_type;
	typedef value_compare_                        value_compare;
	typedef typename base_type::const_iterator    iterator;
	typedef typename base_type::const_iterator    const_iterator;
	typedef std::reverse_iterator<iterator>       reverse_iterator;
	typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

	ordered_array() = default;

	ordered_array(value_compare const &comp_) : comp(comp_)
	{}

	template <typename input_iter_t>
	ordered_array(input_iter_t first, input_iter_t last)
	{
		insert(first, last);
	}

	template <typename input_iter_t>
	ordered_array(input_iter_t first, input_iter_t last,
		      value_compare const &comp_)
	: comp(comp_)
	{
		insert(first, last);
	}

	ordered_array(ordered_array const &other) = default;

	ordered_array(ordered_array &&other) = default;

	ordered_array(std::initializer_list<value_type> l,
		      value_compare const &comp_ = value_compare())
	: comp(comp_)
	{
		insert(l);
	}

	ordered_array &operator=(ordered_array const &other) = default;

	ordered_array &operator=(ordered_array &&other)
	{
		std::swap(comp, other.comp);
		data.swap(other.data);
		return *this;
	}

	ordered_array &operator=(std::initializer_list<value_type> l)
	{
		data.clear();
		insert(l);
		return *this;
	}

	~ordered_array() = default;

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
		return data.cbegin();
	}

	const_iterator cend() const
	{
		return data.cend();
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
		return data.empty();
	}

	size_type size() const
	{
		return data.size();
	}

	size_type max_size() const
	{
		return capacity;
	}

	void swap(ordered_array &other)
	{
		std::swap(comp, other.comp);
		data.swap(other.data);
	}

	const_iterator insert(value_type const &v)
	{
		auto pos(std::upper_bound(data.cbegin(), data.cend(), v, comp));

		return const_iterator(data.insert(pos, v));
	}

	const_iterator insert(value_type &&v)
	{
		auto pos(std::upper_bound(data.cbegin(), data.cend(), v, comp));

		return const_iterator(data.insert(pos, std::move(v)));
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
		return const_iterator(data.erase(first, last));
	}

	void clear()
	{
		data.clear();
	}

	value_type const &operator[](size_type pos) const
	{
		return data[pos];
	}

	value_type const &at(size_type pos) const
	{
		return data.at[pos];
	}

	value_type const &front() const
	{
		return data.front();
	}

	value_type const &back() const
	{
		return data.back();
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
	value_compare comp;
	base_type data;
};

}

#endif
