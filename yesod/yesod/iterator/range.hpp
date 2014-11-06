/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_YESOD_ITERATOR_RANGE_20140327T1420)
#define UCPF_YESOD_ITERATOR_RANGE_20140327T1420

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

	size_type size() const
	{
		return std::distance(first, last);
	}

	reference operator[](difference_type pos) const
	{
		auto iter(first);
		std::advance(iter, pos);
		return *iter;
	}

private:
	Iterator first, last;
};

template <typename Iterator>
auto make_range(Iterator first, Iterator last)
{
	return range<Iterator>(first, last);
}

template <typename Iterator>
auto make_range(Iterator first, size_t n)
{
	auto last(first);
	std::advance(last, n);
	return range<Iterator>(first, last);
}

template <typename Range>
auto make_range(Range &r)
{
	return range<decltype(std::begin(r))>(std::begin(r), std::end(r));
}

template <typename CharType>
auto str(CharType const *s)
{
	return range<CharType const *>(
		s, s + std::char_traits<CharType>::length(s)
	);
}

}}}
#endif
