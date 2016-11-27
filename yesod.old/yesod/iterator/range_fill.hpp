/*
 * Copyright (c) 2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_DE3C6E1EA3CF32424B1A562FE92567E9)
#define HPP_DE3C6E1EA3CF32424B1A562FE92567E9

#include <iterator>

namespace ucpf { namespace yesod { namespace iterator {

template <typename Iterator, bool Circular = false>
struct range_fill : std::iterator<
	std::output_iterator_tag, void, void, void, void
>
{
	typedef typename std::iterator_traits<Iterator>::value_type value_type;

	range_fill(Iterator first, Iterator last)
	: range(first, last), dist_count(0)
	{}

	range_fill &operator=(value_type const &v)
	{
		if (!range.is_last())
			*range.get_current() = v;

		return *this;
	}

	range_fill &operator=(value_type &&v)
	{
		if (!range.is_last())
			*(range.get_current()) = std::move(v);

		return *this;
	}

	range_fill &operator*()
	{
		return *this;
	}

	range_fill &operator++()
	{
		range.advance();
		++dist_count;
		return *this;
	}

	range_fill &operator++(int)
	{
		range.advance();
		++dist_count;
		return *this;
	}

	Iterator const &base()
	{
		return range.get_current();
	}

	std::size_t distance() const
	{
		return dist_count;
	}

private:
	struct truncating_range {
		truncating_range(Iterator first_, Iterator last_)
		: first(first_), last(last_)
		{}

		Iterator &get_current()
		{
			return first;
		}

		bool is_last() const
		{
			return first == last;
		}

		void advance()
		{
			if (first != last)
				++first;
		}

		Iterator first, last;
	};

	struct circular_range {
		circular_range(Iterator first_, Iterator last_)
		: pos(first_), first(first_), last(last_)
		{}

		Iterator &get_current()
		{
			return pos;
		}

		bool is_last() const
		{
			return false;
		}

		void advance()
		{
			if (pos != last)
				++pos;

			if (pos == last)
				pos = first;
		}		

		Iterator pos, first, last;
	};

	typename std::conditional<
		Circular, circular_range, truncating_range
	>::type range;
	std::size_t dist_count;
};

template <typename Iterator>
range_fill<Iterator> make_range_fill(Iterator first, Iterator last)
{
	return range_fill<Iterator>(first, last);
}

template <typename Container>
range_fill<typename Container::iterator> make_range_fill(Container &c)
{
	return range_fill<typename Container::iterator>(
		std::begin(c), std::end(c)
	);
}

}}}
#endif
