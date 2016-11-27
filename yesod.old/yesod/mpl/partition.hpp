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

#if !defined(UCPF_YESOD_MPL_PARTITION_DEC_17_2013_1740)
#define UCPF_YESOD_MPL_PARTITION_DEC_17_2013_1740

#include <yesod/mpl/fold.hpp>
#include <yesod/mpl/clear.hpp>
#include <yesod/mpl/inserter.hpp>

namespace ucpf { namespace yesod { namespace mpl {
namespace detail {

template <typename Pred, typename In1Op, typename In2Op>
struct partition_op {
	template <typename State, typename T>
	struct apply {
		typedef typename State::first first_;
		typedef typename State::second second_;
		typedef typename mpl::apply<Pred, T>::type pred_;

		typedef typename eval_if<
			pred_,
			mpl::apply<In1Op, first_, T>,
			mpl::apply<In2Op, second_, T>
		>::type result_;

		typedef typename if_<
			pred_, pair<result_, second_>, pair<first_, result_>
		>::type type;
	};
};

}

template <typename T0, typename T1, typename T2, typename Tag>
struct lambda<detail::partition_op<T0, T1, T2>, Tag, long_<3>> {
	typedef false_type is_le;
	typedef detail::partition_op<T0, T1, T2> result_;
	typedef result_ type;
};

namespace detail {
template <
	typename Sequence, typename Pred, typename In1, typename In2
> struct stable_partition_impl : fold<
	Sequence, pair<typename In1::state, typename In2::state>,
	protect<partition_op<
		Pred, typename In1::operation, typename In2::operation
	>>
> {};

template <
	typename Sequence, typename Pred, typename In1, typename In2
> struct reverse_stable_partition_impl : reverse_fold<
	Sequence, pair<typename In1::state, typename In2::state>,
	protect<partition_op<
		Pred, typename In1::operation, typename In2::operation
	>>
> {};

}

template <typename...>
struct stable_partition;

template <>
struct stable_partition<> {
	template <
		typename T0, typename T1 , typename T2 , typename T3,
		typename... Tn
	> struct apply : stable_partition<T0, T1, T2, T3> {};
};

template <typename Tag>
struct lambda<stable_partition<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef stable_partition<> result_;
	typedef stable_partition<> type;
};

template <typename P0, typename P1>
struct stable_partition<P0, P1> : if_<
	has_push_back<typename clear<P0>::type>,
	detail::stable_partition_impl<
		P0, P1, back_inserter<typename clear<P0>::type>,
		back_inserter<typename clear<P0>::type>
	>,
	detail::reverse_stable_partition_impl<
		P0, P1, front_inserter<typename clear<P0>::type>,
		front_inserter<typename clear<P0>::type>
	>
>::type {};

template <typename P0, typename P1, typename P2>
struct stable_partition<P0, P1, P2> : if_<
	has_push_back<typename clear<P0>::type>,
	detail::stable_partition_impl<
		P0, P1, P2, back_inserter<typename clear<P0>::type>
	>,
	detail::reverse_stable_partition_impl<
		P0, P1, P2, front_inserter<typename clear<P0>::type>
	>
>::type {};

template <typename P0, typename P1, typename P2, typename P3>
struct stable_partition<P0, P1, P2, P3>
: detail::stable_partition_impl<P0, P1, P2, P3> {};

template <typename ...>
struct reverse_stable_partition;

template <>
struct reverse_stable_partition<> {
	template <
		typename T0, typename T1 , typename T2 , typename T3,
		typename... Tn
	> struct apply : reverse_stable_partition<T0, T1, T2, T3> {};
};

template <typename Tag>
struct lambda<reverse_stable_partition<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef reverse_stable_partition<> result_;
	typedef reverse_stable_partition<> type;
};

template <typename P0, typename P1>
struct reverse_stable_partition<P0, P1> : if_<
	has_push_back<P0>,
	detail::reverse_stable_partition_impl<
		P0, P1, back_inserter<typename clear<P0>::type>,
		back_inserter<typename clear<P0>::type>
	>,
	detail::stable_partition_impl<
		P0, P1, front_inserter<typename clear<P0>::type>,
		front_inserter<typename clear<P0>::type>
	>
>::type {};

template <typename P0, typename P1, typename P2>
struct reverse_stable_partition<P0, P1, P2> : if_<
	has_push_back<P0>,
	detail::reverse_stable_partition_impl<
		P0, P1, P2, back_inserter<typename clear<P0>::type>
	>,
	detail::stable_partition_impl<
		P0, P1, P2, front_inserter<typename clear<P0>::type>
	>
>::type {};

template <typename P0, typename P1, typename P2, typename P3>
struct reverse_stable_partition<P0, P1, P2, P3>
: detail::reverse_stable_partition_impl<P0, P1, P2, P3> {};

template <typename...>
struct partition;

template <>
struct partition<> {
	template <
		typename T0, typename T1 , typename T2 , typename T3,
		typename... Tn
	> struct apply : partition<T0, T1, T2, T3> {};
};

template <typename Tag>
struct lambda<partition<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef partition<> result_;
	typedef partition<> type;
};

template <typename P0, typename P1>
struct partition<P0, P1> : if_<
	has_push_back<typename clear<P0>::type>,
	detail::stable_partition_impl<
		P0, P1, back_inserter<typename clear<P0>::type>,
		back_inserter<typename clear<P0>::type>
	>,
	detail::reverse_stable_partition_impl<
		P0, P1, front_inserter<typename clear<P0>::type>,
		front_inserter<typename clear<P0>::type>
	>
>::type {};

template <typename P0, typename P1, typename P2>
struct partition<P0, P1, P2> : if_<
	has_push_back<typename clear<P0>::type>,
	detail::stable_partition_impl<
		P0, P1, P2, back_inserter<typename clear<P0>::type>
	>,
	detail::reverse_stable_partition_impl<
		P0, P1, P2, front_inserter<typename clear<P0>::type>
	>
>::type {};

template <typename P0, typename P1, typename P2, typename P3>
struct partition<P0, P1, P2, P3>
: detail::stable_partition_impl<P0, P1, P2, P3> {};

template <typename ...>
struct reverse_partition;

template <>
struct reverse_partition<> {
	template <
		typename T0, typename T1 , typename T2 , typename T3,
		typename... Tn
	> struct apply : reverse_partition<T0, T1, T2, T3> {};
};

template <typename Tag>
struct lambda<reverse_partition<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef reverse_partition<> result_;
	typedef reverse_partition<> type;
};

template <typename P0, typename P1>
struct reverse_partition<P0, P1> : if_<
	has_push_back<P0>,
	detail::reverse_stable_partition_impl<
		P0, P1, back_inserter<typename clear<P0>::type>,
		back_inserter<typename clear<P0>::type>
	>,
	detail::stable_partition_impl<
		P0, P1, front_inserter<typename clear<P0>::type>,
		front_inserter<typename clear<P0>::type>
	>
>::type {};

template <typename P0, typename P1, typename P2>
struct reverse_partition<P0, P1, P2> : if_<
	has_push_back<P0>,
	detail::reverse_stable_partition_impl<
		P0, P1, P2, back_inserter<typename clear<P0>::type>
	>,
	detail::stable_partition_impl<
		P0, P1, P2, front_inserter<typename clear<P0>::type>
	>
>::type {};

template <typename P0, typename P1, typename P2, typename P3>
struct reverse_partition<P0, P1, P2, P3>
: detail::reverse_stable_partition_impl<P0, P1, P2, P3> {};

}}}

#endif
