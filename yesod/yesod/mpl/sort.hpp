/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

/*=============================================================================
    Based on implementation of boost::mpl library:

    Copyright (c) 2002-2003 Eric Friedman
    Copyright (c) 2004      Aleksey Gurtovoy

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#if !defined(UCPF_YESOD_MPL_SORT_DEC_17_2013_1800)
#define UCPF_YESOD_MPL_SORT_DEC_17_2013_1800

#include <yesod/mpl/size.hpp>
#include <yesod/mpl/copy.hpp>
#include <yesod/mpl/vector.hpp>
#include <yesod/mpl/partition.hpp>
#include <yesod/mpl/joint_view.hpp>
#include <yesod/mpl/single_view.hpp>

namespace ucpf { namespace yesod { namespace mpl {
namespace detail {

template <typename Seq, typename Pred>
struct quick_sort;

template <typename Pred, typename Pivot>
struct quick_sort_pred {
	template <typename T>
	struct apply {
		typedef typename mpl::apply<Pred, T, Pivot>::type type;
	};
};

template <typename Seq, typename Pred>
struct quick_sort_impl {
	typedef typename begin<Seq>::type pivot;
	typedef typename partition<
		iterator_range<
			typename next<pivot>::type,
			typename end<Seq>::type
		>, protect<quick_sort_pred<Pred, typename deref<pivot>::type>>,
		back_inserter<vector<>>, back_inserter<vector<>>
	>::type partitioned;

	typedef typename quick_sort<
		typename partitioned::first, Pred
	>::type part1;

	typedef typename quick_sort<
		typename partitioned::second, Pred
	>::type part2;

	typedef joint_view<
		joint_view<part1, single_view<typename deref<pivot>::type>>,
		part2
	> type;
};

template <typename Seq, typename Pred>
struct quick_sort : eval_if<
	empty<Seq>, identity<Seq>, quick_sort_impl<Seq,Pred>
> {};

template <typename Sequence, typename Pred, typename In>
struct sort_impl {
	typedef typename quick_sort<Sequence, Pred>::type result_;

	typedef typename copy<result_, In>::type type;
};

template <typename Sequence, typename Pred, typename In>
struct reverse_sort_impl {
	typedef typename quick_sort<Sequence, Pred>::type result_;

	typedef typename reverse_copy<result_, In>::type type;
};

}

template <typename...>
struct sort;

template <>
struct sort<> {
	template<typename T0, typename T1, typename T2, typename... Tn>
	struct apply : sort<T0, T1, T2> {};
};

template <typename Tag>
struct lambda<sort<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef sort<> result_;
	typedef sort<> type;
};

template <typename P0>
struct sort<P0> : sort<P0, less<>> {};

template <typename P0, typename P1>
struct sort<P0, P1> : if_<
	has_push_back<typename clear<P0>::type>,
	detail::sort_impl<P0, P1, back_inserter<typename clear<P0>::type>>,
	detail::reverse_sort_impl<
		P0, P1, front_inserter<typename clear<P0>::type>
	>
>::type {};

template <typename P0, typename P1, typename P2>
struct sort<P0, P1, P2> : detail::sort_impl<P0, P1, P2> {};

template <typename...>
struct reverse_sort;

template <>
struct reverse_sort<> {
	template<typename T0, typename T1, typename T2, typename... Tn>
	struct apply : reverse_sort<T0, T1, T2> {};
};

template <typename Tag>
struct lambda<reverse_sort<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef reverse_sort<> result_;
	typedef reverse_sort<> type;
};

template <typename P0>
struct reverse_sort<P0> : reverse_sort<P0, less<>> {};

template <typename P0, typename P1>
struct reverse_sort<P0, P1> : if_<
	has_push_back<P0> ,
	detail::reverse_sort_impl<
		P0, P1, back_inserter<typename clear<P0>::type>
	>,
	detail::sort_impl<P0, P1, front_inserter<typename clear<P0>::type>>
>::type {};

template <typename P0, typename P1, typename P2>
struct reverse_sort<P0, P1, P2> : detail::reverse_sort_impl<P0, P1, P2> {};

}}}

#endif
