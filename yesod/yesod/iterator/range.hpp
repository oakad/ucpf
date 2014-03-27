/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_YESOD_ITERATOR_RANGE_MAR_27_2014_1420)
#define UCPF_YESOD_ITERATOR_RANGE_MAR_27_2014_1420

#include <yesod/iterator/facade.hpp>

namespace ucpf { namespace yesod { namespace iterator {

template <typename Iterator>
struct range {
	typedef range type;
	typedef size_t size_type;
	typedef typename std::iterator_traits<Iterator>::value_type value_type;
	typedef typename std::iterator_traits<Iterator>::reference reference;
	typedef typename std::iterator_traits<
		Iterator
	>::difference_type difference_type;

	template <typename ValueType>
	struct iterator_base : facade<
		iterator_base<ValueType>, ValueType,
		std::iterator_traits<Iterator>::iterator_category
	> {
	private:
		friend struct iterator_core_access;

		iterator_base() = default;

		template <typename ValueType1>
		iterator_base(
			iterator_base<ValueType1> const &other,
			typename std::enable_if<
				std::is_convertible<
					ValueType1 *, ValueType *
				>::value, nullptr_t
			>::type = nullptr
		) : pos(other)
		{}

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

		void advance(difference_type n)
		{
			pos += n;
		}

		ValueType &dereference() const
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

	range(Iterator first_, Iterator last_)
	: first(first_), last(last_)
	{}

	iterator begin()
	{
		return iterator(first);
	}

	iterator end()
	{
		return iterator(last);
	}

	const_iterator begin() const
	{
		return const_iterator(first);
	}

	const_iterator end() const
	{
		return const_iterator(last);
	}

private:
	Iterator first;
	Iterator last;
};

template <typename Iterator>
range<Iterator> make_range(Iterator first, Iterator last)
{
	return range<Iterator>(first, last);
}

template <typename Iterator>
range<Iterator> make_range(Iterator first, size_t n)
{
	return range<Iterator>(first, first + n);
}

template <typename Range>
range<decltype(std::begin(Range)> make_range(Range const &r)
{
	return range<Iterator>(std::begin(r), std::end(r));
}

}}}
#endif
