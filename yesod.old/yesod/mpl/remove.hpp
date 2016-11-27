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

#if !defined(UCPF_YESOD_MPL_REMOVE_DEC_17_2013_1750)
#define UCPF_YESOD_MPL_REMOVE_DEC_17_2013_1750

#include <yesod/mpl/clear.hpp>
#include <yesod/mpl/same_as.hpp>
#include <yesod/mpl/transform.hpp>

namespace ucpf { namespace yesod { namespace mpl {
namespace detail {

template <typename Pred, typename InsertOp>
struct remove_if_helper {
	template <typename Sequence, typename U>
	struct apply {
		typedef typename eval_if<
			typename mpl::apply<Pred, U>::type,
			identity<Sequence>,
			mpl::apply<InsertOp, Sequence, U>
		>::type type;
	};
};

template <typename Sequence, typename Predicate, typename Inserter>
struct remove_if_impl : fold<
	Sequence, typename Inserter::state, protect<
		remove_if_helper<
			typename lambda<Predicate>::type,
			typename Inserter::operation
		>
	>
> {};

template <typename Sequence, typename Predicate, typename Inserter>
struct reverse_remove_if_impl : reverse_fold<
	Sequence, typename Inserter::state, protect<
		remove_if_helper<
			typename lambda<Predicate>::type,
			typename Inserter::operation
		>
	>
> {};

template <typename Sequence, typename T, typename Inserter>
struct remove_impl : remove_if_impl<Sequence, same_as<T>, Inserter> {};

template <typename Sequence, typename T, typename Inserter>
struct reverse_remove_impl
: reverse_remove_if_impl<Sequence, same_as<T>, Inserter> {};

}

template <typename...>
struct remove_if;

template <>
struct remove_if<> {
	template <
		typename T0, typename T1, typename T2, typename... Tn
	> struct apply : remove_if<T0, T1, T2> {};
};

template <typename Tag>
struct lambda<remove_if<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef remove_if<> result_;
	typedef remove_if<> type;
};

template <typename P0, typename P1>
struct remove_if<P0, P1> : if_<
	has_push_back<typename clear<P0>::type>,
	detail::remove_if_impl<
		P0, P1, back_inserter<typename clear<P0>::type>
	>,
	detail::reverse_remove_if_impl<
		P0, P1, front_inserter<typename clear<P0>::type>
	>
>::type {};

template <typename P0, typename P1, typename P2>
struct remove_if<P0, P1, P2> : detail::remove_if_impl<P0, P1, P2> {};

template <typename...>
struct reverse_remove_if;

template <>
struct reverse_remove_if<> {
	template <
		typename T0, typename T1, typename T2, typename... Tn
	> struct apply : reverse_remove_if<T0, T1, T2> {};
};

template <typename Tag>
struct lambda<reverse_remove_if<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef reverse_remove_if<> result_;
	typedef reverse_remove_if<> type;
};

template <typename P0, typename P1>
struct reverse_remove_if<P0, P1> : if_<
	has_push_back<P0>,
	detail::reverse_remove_if_impl<
		P0, P1, back_inserter<typename clear<P0>::type>
	>,
	detail::remove_if_impl<
		P0, P1, front_inserter<typename clear<P0>::type>
	>
>::type {};

template <typename P0, typename P1, typename P2>
struct reverse_remove_if<P0, P1, P2>
: detail::reverse_remove_if_impl<P0, P1, P2> {};

template <typename...>
struct remove;

template <>
struct remove<> {
	template <
		typename T0, typename T1, typename T2, typename... Tn
	> struct apply : remove<T0, T1, T2> {};
};

template <typename Tag>
struct lambda<remove<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef remove<> result_;
	typedef remove<> type;
};

template <typename P0, typename P1>
struct remove<P0, P1> : if_<
	has_push_back<typename clear<P0>::type>,
	detail::remove_impl<
		P0, P1, back_inserter<typename clear<P0>::type>
	>,
	detail::reverse_remove_impl<
		P0, P1, front_inserter<typename clear<P0>::type>
	>
>::type {};

template <typename P0, typename P1, typename P2>
struct remove<P0, P1, P2> : detail::remove_impl<P0, P1, P2> {};

template <typename...>
struct reverse_remove;

template <>
struct reverse_remove<> {
	template <
		typename T0, typename T1, typename T2, typename... Tn
	> struct apply : reverse_remove<T0, T1, T2> {};
};

template <typename Tag>
struct lambda<reverse_remove<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef reverse_remove<> result_;
	typedef reverse_remove<> type;
};

template <typename P0, typename P1>
struct reverse_remove<P0, P1> : if_<
	has_push_back<P0> ,
	detail::reverse_remove_impl<
		P0, P1, back_inserter<typename clear<P0>::type>
	>,
	detail::remove_impl<
		P0, P1, front_inserter<typename clear<P0>::type>
	>
>::type {};

template <typename P0, typename P1, typename P2>
struct reverse_remove<P0, P1, P2>
: detail::reverse_remove_impl<P0, P1, P2> {};

}}}

#endif
