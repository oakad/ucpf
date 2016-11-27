/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

/*=============================================================================
    Based on implementation of boost::mpl library:

    Copyright (c) 2003-2004 David Abrahams
    Copyright (c) 2004      Aleksey Gurtovoy

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#if !defined(UCPF_YESOD_MPL_PAIR_VIEW_DEC_17_2013_1730)
#define UCPF_YESOD_MPL_PAIR_VIEW_DEC_17_2013_1730

#include <yesod/mpl/pair.hpp>
#include <yesod/mpl/min_max.hpp>
#include <yesod/mpl/distance.hpp>
#include <yesod/mpl/iterator_category.hpp>

namespace ucpf { namespace yesod { namespace mpl {
namespace detail {

struct pair_iterator_tag;

template <typename Iter0, typename Iter1, typename Category>
struct pair_iterator {
    typedef pair_iterator_tag tag;
    typedef Category category;
    typedef Iter0 first;
    typedef Iter1 second;
};

template <>
struct advance_impl<pair_iterator_tag> {
	template <typename Iter, typename D>
	struct apply {
		typedef typename advance<typename Iter::first, D>::type i0_;
		typedef typename advance<typename Iter::second, D>::type i1_;
		typedef pair_iterator<i0_, i1_, typename Iter::category> type;
	};
};

template <>
struct distance_impl<pair_iterator_tag> {
	template <typename Iter0, typename Iter1>
	struct apply {
		typedef typename distance<
			typename first<Iter0>::type,
			typename first<Iter1>::type
		>::type type;
	};
};

}

template <typename Iter0, typename Iter1, typename C>
struct deref<detail::pair_iterator<Iter0, Iter1, C>> {
	typedef pair<
		typename deref<Iter0>::type, typename deref<Iter1>::type
	> type;
};

template <typename Iter0, typename Iter1, typename C>
struct next<detail::pair_iterator<Iter0, Iter1, C>> {
	typedef typename next<Iter0>::type i0_;
	typedef typename next<Iter1>::type i1_;
	typedef detail::pair_iterator<i0_, i1_, C> type;
};

template <typename Iter0, typename Iter1, typename C>
struct prior<detail::pair_iterator<Iter0, Iter1, C>> {
	typedef typename prior<Iter0>::type i0_;
	typedef typename prior<Iter1>::type i1_;
	typedef detail::pair_iterator<i0_, i1_, C> type;
};

template <typename...>
struct pair_view;

template <>
struct pair_view<> {
	template <typename T0, typename T1, typename... Tn>
	struct apply : pair_view<T0, T1> {};
};

template <typename Tag>
struct lambda<pair_view<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef pair_view<> result_;
	typedef pair_view<> type;
};

template <typename Sequence0, typename Sequence1>
struct pair_view<Sequence0, Sequence1> {
	typedef nested_begin_end_tag tag;

	typedef typename begin<Sequence0>::type iter0_;
	typedef typename begin<Sequence1>::type iter1_;

	typedef typename min<
		typename iterator_category<iter0_>::type,
		typename iterator_category<iter1_>::type
	>::type category_;

	typedef detail::pair_iterator<iter0_, iter1_, category_> begin;

	typedef detail::pair_iterator<
		typename end<Sequence0>::type,
		typename end<Sequence1>::type,
		category_
	> end;
};

}}}

#endif
