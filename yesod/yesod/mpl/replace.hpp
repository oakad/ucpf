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
    Copyright (c) 2000-2002 John R. Bandela
    Copyright (c) 2003-2004 David Abrahams

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#if !defined(UCPF_YESOD_MPL_REPLACE_DEC_17_2013_1750)
#define UCPF_YESOD_MPL_REPLACE_DEC_17_2013_1750

#include <yesod/mpl/clear.hpp>
#include <yesod/mpl/same_as.hpp>
#include <yesod/mpl/transform.hpp>

namespace ucpf { namespace yesod { namespace mpl {
namespace detail {

template <typename Predicate, typename T>
struct replace_if_op {
	template <typename U>
	struct apply : if_<
		typename mpl::apply<Predicate, U>::type, T, U
	> {};
};

template <typename Sequence, typename Predicate, typename T, typename Inserter>
struct replace_if_impl : transform_impl<
	Sequence, protect<replace_if_op<Predicate, T>>, Inserter
> {};

template <typename Sequence, typename Predicate, typename T, typename Inserter>
struct reverse_replace_if_impl : reverse_transform_impl<
	Sequence, protect<replace_if_op<Predicate, T>>, Inserter
> {};

template <
	typename Sequence, typename OldType, typename NewType, typename Inserter
> struct replace_impl
: replace_if_impl<Sequence, same_as<OldType>, NewType, Inserter> {};

template <
	typename Sequence, typename OldType, typename NewType, typename Inserter
> struct reverse_replace_impl
: reverse_replace_if_impl<Sequence, same_as<OldType>, NewType, Inserter> {};

}

template <typename...>
struct replace_if;

template <>
struct replace_if<> {
	template <
		typename T0, typename T1, typename T2, typename T3,
		typename... Tn
	> struct apply : replace_if<T0, T1, T2, T3> {};
};

template <typename Tag>
struct lambda<replace_if<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef replace_if<> result_;
	typedef replace_if<> type;
};

template <typename P0, typename P1, typename P2>
struct replace_if<P0, P1, P2> : if_<
	has_push_back<typename clear<P0>::type>,
	detail::replace_if_impl<
		P0, P1, P2, back_inserter<typename clear<P0>::type>
	>,
	detail::reverse_replace_if_impl<
		P0, P1, P2, front_inserter<typename clear<P0>::type>
	>
>::type {};

template <typename P0, typename P1, typename P2, typename P3>
struct replace_if<P0, P1, P2, P3> : detail::replace_if_impl<P0, P1, P2, P3> {};

template <typename...>
struct reverse_replace_if;

template <>
struct reverse_replace_if<> {
	template <
		typename T0, typename T1, typename T2, typename T3,
		typename... Tn
	> struct apply : reverse_replace_if<T0, T1, T2, T3> {};
};

template <typename Tag>
struct lambda<reverse_replace_if<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef reverse_replace_if<> result_;
	typedef reverse_replace_if<> type;
};

template <typename P0, typename P1, typename P2>
struct reverse_replace_if<P0, P1, P2> : if_<
	has_push_back<P0>,
	detail::reverse_replace_if_impl<
		P0, P1, P2, back_inserter<typename clear<P0>::type>
	>,
	detail::replace_if_impl<
		P0, P1, P2, front_inserter<typename clear<P0>::type>
	>
>::type {};

template <typename P0, typename P1, typename P2, typename P3>
struct reverse_replace_if<P0, P1, P2, P3>
: detail::reverse_replace_if_impl<P0, P1, P2, P3> {};

template <typename...>
struct replace;

template <>
struct replace<> {
	template <
		typename T0, typename T1, typename T2, typename T3,
		typename... Tn
	> struct apply : replace<T0, T1, T2, T3> {};
};

template <typename Tag>
struct lambda<replace<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef replace<> result_;
	typedef replace<> type;
};

template <typename P0, typename P1, typename P2>
struct replace<P0, P1, P2> : if_<
	has_push_back<typename clear<P0>::type>,
	detail::replace_impl<
		P0, P1, P2, back_inserter<typename clear<P0>::type>
	>,
	detail::reverse_replace_impl<
		P0, P1, P2, front_inserter<typename clear<P0>::type>
	>
>::type {};

template <typename P0, typename P1, typename P2, typename P3>
struct replace<P0, P1, P2, P3> : detail::replace_impl<P0, P1, P2, P3> {};

template <typename...>
struct reverse_replace;

template <>
struct reverse_replace<> {
	template <
		typename T0, typename T1, typename T2, typename T3,
		typename... Tn
	> struct apply : reverse_replace<T0, T1, T2, T3> {};
};

template <typename Tag>
struct lambda<reverse_replace<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef reverse_replace<> result_;
	typedef reverse_replace<> type;
};

template <typename P0, typename P1, typename P2>
struct reverse_replace<P0, P1, P2> : if_<
	has_push_back<P0> ,
	detail::reverse_replace_impl<
		P0, P1, P2, back_inserter<typename clear<P0>::type>
	>,
	detail::replace_impl<
		P0, P1, P2, front_inserter<typename clear<P0>::type>
	>
>::type {};

template <typename P0, typename P1, typename P2, typename P3>
struct reverse_replace<P0, P1, P2, P3>
: detail::reverse_replace_impl<P0, P1, P2, P3> {};

}}}

#endif
