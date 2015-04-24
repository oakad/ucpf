/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_143727AD722EA03961E8636E66406C03)
#define HPP_143727AD722EA03961E8636E66406C03

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

	typedef Iterator iterator;
	typedef Iterator const_iterator;

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
