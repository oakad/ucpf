/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

/*=============================================================================
    Based on implementation of boost::mpl library:

    Copyright (c) 2000-2004 Aleksey Gurtovoy
    Copyright (c) 2003-2004 David Abrahams

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#if !defined(UCPF_YESOD_MPL_TRANSFORM_DEC_17_2013_1750)
#define UCPF_YESOD_MPL_TRANSFORM_DEC_17_2013_1750

#include <yesod/mpl/fold.hpp>
#include <yesod/mpl/clear.hpp>
#include <yesod/mpl/inserter.hpp>
#include <yesod/mpl/pair_view.hpp>
#include <yesod/mpl/is_sequence.hpp>

namespace ucpf { namespace yesod { namespace mpl {
namespace detail {

template <typename...>
struct transform_impl;

template <typename Seq0, typename Op, typename In>
struct transform_impl<Seq0, Op, In>
: fold<Seq0, typename In::state, bind<
	typename lambda<typename In::operation>::type, arg<0>,
	bind<typename lambda<Op>::type, arg<1>>
>> {};

template <typename Seq0, typename Seq1, typename Op, typename In>
struct transform_impl<Seq0, Seq1, Op, In>
: fold<pair_view<Seq0, Seq1>, typename In::state, bind<
	typename lambda<typename In::operation>::type, arg<0>,
	bind<
		typename lambda<Op>::type, bind<first<>, arg<1>>,
		bind<second<>, arg<1>>
	>
>> {};

template <typename...>
struct reverse_transform_impl;

template <typename Seq0, typename Op, typename In>
struct reverse_transform_impl<Seq0, Op, In>
: reverse_fold<Seq0, typename In::state, bind<
	typename lambda<typename In::operation>::type, arg<0>,
	bind<typename lambda<Op>::type, arg<1>>
>> {};

template <typename Seq0, typename Seq1, typename Op, typename In>
struct reverse_transform_impl<Seq0, Seq1, Op, In>
: reverse_fold<pair_view<Seq0, Seq1>, typename In::state, bind<
	typename lambda<typename In::operation>::type, arg<0>,
	bind<
		typename lambda<Op>::type, bind<first<>, arg<1>>,
		bind<second<>, arg<1>>
	>
>> {};

}

template <typename...>
struct transform1;

template <>
struct transform1<> {
	template <typename T0, typename T1, typename T2, typename... Tn>
	struct apply : transform1<T0, T1, T2> {};
};

template <typename Tag>
struct lambda<transform1<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef transform1<> result_;
	typedef transform1<> type;
};

template <typename P0, typename P1>
struct transform1<P0, P1> : if_<
	has_push_back<typename clear<P0>::type>,
	detail::transform_impl<
		P0, P1, back_inserter<typename clear<P0>::type>
	>,
	detail::reverse_transform_impl<
		P0, P1, front_inserter<typename clear<P0>::type>
	>
>::type {};

template <typename P0, typename P1, typename P2>
struct transform1<P0, P1, P2> : detail::transform_impl<P0, P1, P2> {};

template <typename...>
struct reverse_transform1;

template <>
struct reverse_transform1<> {
	template<typename T0, typename T1 , typename T2, typename... Tn>
	struct apply : reverse_transform1<T0, T1, T2> {};
};

template <typename Tag>
struct lambda<reverse_transform1<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef reverse_transform1<> result_;
	typedef reverse_transform1<> type;
};

template <typename P0, typename P1>
struct reverse_transform1<P0, P1> : if_<
	has_push_back<P0>,
	detail::reverse_transform_impl<
		P0, P1, back_inserter<typename clear<P0>::type>
	>,
	detail::transform_impl<
		P0, P1, front_inserter<typename clear<P0>::type>
	>
>::type {};

template <typename P0, typename P1, typename P2>
struct reverse_transform1<P0, P1, P2>
: detail::reverse_transform_impl<P0, P1, P2> {};

template <typename...>
struct transform2;

template <>
struct transform2<> {
	template <
		typename T0, typename T1, typename T2, typename T3,
		typename... Tn
	> struct apply : transform2<T0, T1, T2, T3> {};
};

template <typename Tag>
struct lambda <transform2<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef transform2<> result_;
	typedef transform2<> type;
};

template <typename P0, typename P1, typename P2>
struct transform2<P0, P1, P2> : if_<
	has_push_back<typename clear<P0>::type>,
	detail::transform_impl<
		P0, P1, P2, back_inserter<typename clear<P0>::type>
	>,
	detail::reverse_transform_impl<
		P0, P1, P2, front_inserter<typename clear<P0>::type>
	>
>::type {};
  
template <typename P0, typename P1, typename P2, typename P3>
struct transform2<P0, P1, P2, P3> : detail::transform_impl<P0, P1, P2, P3> {};

template <typename...>
struct reverse_transform2;

template <>
struct reverse_transform2<> {
	template <
		typename T0, typename T1, typename T2, typename T3,
		typename... Tn
	> struct apply : reverse_transform2<T0, T1, T2, T3> {};
};

template <typename Tag>
struct lambda<reverse_transform2<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef reverse_transform2<> result_;
	typedef reverse_transform2<> type;
};

template <typename P0, typename P1, typename P2>
struct reverse_transform2<P0, P1, P2> : if_<
	has_push_back<P0>,
	detail::reverse_transform_impl<
		P0, P1, P2, back_inserter<typename clear<P0>::type>
	>,
	detail::transform_impl<
		P0, P1, P2, front_inserter<typename clear<P0>::type>
	>
>::type {};

template <typename P0, typename P1, typename P2, typename P3>
struct reverse_transform2<P0, P1, P2, P3>
: detail::reverse_transform_impl<P0, P1, P2, P3> {};

template <typename...>
struct transform;

template <>
struct transform<> {
	template <
		typename T0, typename T1 , typename T2 , typename T3,
		typename... Tn
	> struct apply : transform<T0, T1, T2, T3> { };
};

template <typename Tag>
struct lambda<transform<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef transform<> result_;
	typedef transform<> type;
};

template <typename Seq0, typename Seq1OrOperation>
struct transform<Seq0, Seq1OrOperation> {
	typedef typename transform1<Seq0, Seq1OrOperation>::type type;
};

template <
	typename Seq0, typename Seq1OrOperation, typename OperationOrInserter
> struct transform<Seq0, Seq1OrOperation, OperationOrInserter> {
	typedef typename eval_if<
		or_<
			is_lambda_expression<Seq1OrOperation>,
			not_<is_sequence<Seq1OrOperation>>
		>,
		transform1<Seq0, Seq1OrOperation, OperationOrInserter>,
		transform2<Seq0, Seq1OrOperation, OperationOrInserter>
	>::type type;
};

template <
	typename Seq0, typename Seq1OrOperation, typename OperationOrInserter,
	typename Inserter
> struct transform<Seq0, Seq1OrOperation, OperationOrInserter, Inserter> {
	typedef typename eval_if<
		or_<
			is_lambda_expression<Seq1OrOperation>,
			not_<is_sequence<Seq1OrOperation>>
		>,
		transform1<Seq0, Seq1OrOperation, OperationOrInserter>,
		transform2<Seq0, Seq1OrOperation, OperationOrInserter, Inserter>
	>::type type;
};

template <typename...>
struct reverse_transform;

template <>
struct reverse_transform<> {
	template <
		typename T0, typename T1, typename T2, typename T3,
		typename... Tn
	> struct apply : reverse_transform<T0, T1, T2, T3> {};
};

template <typename Tag>
struct lambda<reverse_transform<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef reverse_transform<> result_;
	typedef reverse_transform<> type;
};

template <typename Seq0, typename Seq1OrOperation>
struct reverse_transform<Seq0, Seq1OrOperation> {
	typedef typename reverse_transform1<Seq0, Seq1OrOperation>::type type;
};

template <typename Seq0, typename Seq1OrOperation, typename OperationOrInserter>
struct reverse_transform<Seq0, Seq1OrOperation, OperationOrInserter> {
	typedef typename eval_if<
		or_<
			is_lambda_expression<Seq1OrOperation>,
			not_<is_sequence<Seq1OrOperation>>
		>,
		reverse_transform1<Seq0, Seq1OrOperation, OperationOrInserter>,
		reverse_transform2<Seq0, Seq1OrOperation, OperationOrInserter>
	>::type type;
};

template <
	typename Seq0, typename Seq1OrOperation, typename OperationOrInserter,
	typename Inserter
> struct reverse_transform<
	Seq0, Seq1OrOperation, OperationOrInserter, Inserter
> {
	typedef typename eval_if<
		or_<
			is_lambda_expression<Seq1OrOperation>,
			not_<is_sequence<Seq1OrOperation>>
		>,
		reverse_transform1<Seq0, Seq1OrOperation, OperationOrInserter>,
		reverse_transform2<
			Seq0, Seq1OrOperation, OperationOrInserter, Inserter
		>
	>::type type;
};

}}}

#endif
