/*=============================================================================
    Copyright (c) 2001-2013 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

/*
 * Cosmetic adaptations as part of present software bundle:
 * 
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(BOOST_SPIRIT_X3_TST_MARCH_09_2007_0905AM)
#define BOOST_SPIRIT_X3_TST_MARCH_09_2007_0905AM

namespace ubb { namespace ext { namespace detail {

// This file contains low level TST routines, not for
// public consumption.

template <typename Char, typename T>
struct tst_node {
	tst_node(Char id)
	: id(id), data(nullptr), lt(nullptr), eq(nullptr), gt(nullptr)
	{}

	template <typename Alloc>
	static void destruct_node(tst_node *p, Alloc *alloc)
	{
		if (p) {
			if (p->data)
				alloc->delete_data(p->data);
			
			destruct_node(p->lt, alloc);
			destruct_node(p->eq, alloc);
			destruct_node(p->gt, alloc);
			alloc->delete_node(p);
		}
	}

	template <typename Alloc>
	static tst_node *clone_node(tst_node *p, Alloc *alloc)
	{
		if (p) {
			tst_node *clone = alloc->new_node(p->id);

			if (p->data)
				clone->data = alloc->new_data(*p->data);

			clone->lt = clone_node(p->lt, alloc);
			clone->eq = clone_node(p->eq, alloc);
			clone->gt = clone_node(p->gt, alloc);
			return clone;
		}
		return nullptr;
	}

	template <typename Iterator, typename Filter>
	static T *find(
		tst_node *start, Iterator &first, Iterator last, Filter filter
	)
	{
		if (first == last)
			return nullptr;

		auto i(first), latest(first);
		auto p(start);
		T *found(nullptr);

		while (p && i != last) {
			auto c(filter(*i)); // filter only the input

			if (c == p->id) {
				if (p->data) {
					found = p->data;
					latest = i;
				}
				p = p->eq;
				++i;
			} else if (c < p->id)
				p = p->lt;
			else
				p = p->gt;
		}

		if (found)
			first = ++latest; // one past the last matching char

		return found;
	}

	template <typename Iterator, typename Alloc>
        static T *add(
		tst_node *&start, Iterator first, Iterator last,
		T &&val, Alloc *alloc
	)
	{
		if (first == last)
			return nullptr;

		auto pp(&start);
		for(;;) {
			auto c(*first);

			if (*pp == nullptr)
				*pp = alloc->new_node(c);

			auto p = *pp;

			if (c == p->id) {
				if (++first == last) {
					if (p->data == nullptr)
						p->data = alloc->new_data(val);

					return p->data;
				}
				pp = &p->eq;
			} else if (c < p->id)
				pp = &p->lt;
			else
				pp = &p->gt;
		}
	}

	template <typename Iterator, typename Alloc>
	static void remove(
		tst_node *&p, Iterator first, Iterator last, Alloc* alloc
	) {
		if (p == nullptr || first == last)
			return;

		auto c(*first);

		if (c == p->id) {
			if (++first == last) {
				if (p->data) {
					alloc->delete_data(p->data);
					p->data = 0;
				}
			}
			remove(p->eq, first, last, alloc);
		} else if (c < p->id)
			remove(p->lt, first, last, alloc);
		else
			remove(p->gt, first, last, alloc);

		if (
			p->data == nullptr
			&& p->lt == nullptr
			&& p->eq == nullptr
			&& p->gt == nullptr
		) {
			alloc->delete_node(p);
			p = nullptr;
		}
	}

	template <typename F>
	static void for_each(tst_node *p, std::basic_string<Char> prefix, F f)
	{
		if (p) {
			for_each(p->lt, prefix, f);

			auto s(prefix + p->id);

			for_each(p->eq, s, f);

			if (p->data)
				f(s, *p->data);

			for_each(p->gt, prefix, f);
		}
	}

		Char id;        // the node's identity character
		T* data;        // optional data
		tst_node* lt;   // left pointer
		tst_node* eq;   // middle pointer
		tst_node* gt;   // right pointer
	};
}}}

#endif
