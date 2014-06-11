/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_YESOD_ITERATOR_SUB_RANGE_20140611T2230)
#define UCPF_YESOD_ITERATOR_SUB_RANGE_20140611T2230

#include <yesod/iterator/facade.hpp>

namespace ucpf { namespace yesod { namespace iterator {

template <typename Iterator>
struct sub_range {
	typedef sub_range type;
	typedef size_t size_type;
	typedef typename std::iterator_traits<Iterator>::value_type value_type;
	typedef typename std::iterator_traits<Iterator>::reference reference;
	typedef typename std::iterator_traits<
		Iterator
	>::difference_type difference_type;

	template <typename ValueType>
	struct iterator_base : facade<
		iterator_base<ValueType>, ValueType,
		typename std::iterator_traits<Iterator>::iterator_category,
		reference, difference_type
	> {
		friend struct core_access;
		friend struct range;

		iterator_base() = default;

		iterator_base(Iterator pos_)
		: pos(pos_)
		{}

		template <typename ValueType1>
		iterator_base(
			iterator_base<ValueType1> const &other,
			typename std::enable_if<
				std::is_convertible<
					ValueType1 *, ValueType *
				>::value, std::nullptr_t
			>::type = nullptr
		) : pos(other.pos)
		{}
	private:
		template <typename ValueType1>
		bool equal(iterator_base<ValueType1> const &other) const
		{
			return pos == other.pos;
		}

		void increment()
		{
			++pos;
		}

		void decrement()
		{
			--pos;
		}

 		void advance(typename iterator_base::difference_type n)
		{
			std::advance(pos, n);
		}

		template <typename ValueType1>
		typename iterator_base::difference_type distance_to(
			iterator_base<ValueType1> const &other
		) const
		{
			return std::distance(pos, other.pos);
		}

		typename iterator_base::reference dereference() const
		{
			return *pos;
		}

		Iterator pos;
	};

	typedef iterator_base<value_type> iterator;
	typedef typename std::conditional<
		std::is_const<value_type>::value,
		iterator_base<value_type>,
		iterator_base<typename std::add_const<value_type>::type>
	>::type const_iterator;

	sub_range(Iterator first_, Iterator last_)
	: first(first_), last(last_), lower(first_), upper(first_), 
	{}

	iterator begin()
	{
		return iterator(lower);
	}

	iterator end()
	{
		return iterator(upper);
	}

	const_iterator begin() const
	{
		return const_iterator(lower);
	}

	const_iterator end() const
	{
		return const_iterator(upper);
	}

	size_type size() const
	{
		return std::distance(lower, upper);
	}

	size_type capacity() const
	{
		return std::distance(first, last);
	}

	reference operator[](difference_type pos) const
	{
		auto iter(lower);
		std::advance(iter, pos);
		return *iter;
	}

	void push_back(value_type const &v)
	{
		if (upper != last) {
			auto pos(upper);
			++upper;
			*pos = v;
		}
	}

	void push_back(value_type &&v)
	{
		if (upper != last) {
			auto pos(upper);
			++upper;
			*pos = std::move(v);
		}
	}

	void pop_back()
	{
		if (upper != lower)
			--upper;
	}

private:
	Iterator first, last, lower, upper;
};

template <typename Iterator>
auto make_sub_range(Iterator first, Iterator last) -> range<Iterator>
{
	return sub_range<Iterator>(first, last);
}

template <typename Iterator>
auto make_sub_range(Iterator first, size_t n) -> range<Iterator>
{
	auto last(first);
	std::advance(last, n);
	return sub_range<Iterator>(first, last);
}

template <typename Range>
auto make_sub_range(Range const &r) -> range<
	decltype(std::begin(std::declval<Range>()))
>
{
	return sub_range<
		decltype(std::begin(std::declval<Range>()))
	>(std::begin(r), std::end(r));
}

}}}
#endif
