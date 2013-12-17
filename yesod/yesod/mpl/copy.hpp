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

#if !defined(UCPF_YESOD_MPL_COPY_DEC_16_2013_1400)
#define UCPF_YESOD_MPL_COPY_DEC_16_2013_1400

#include <yesod/mpl/fold.hpp>
#include <yesod/mpl/clear.hpp>
#include <yesod/mpl/inserter.hpp>

namespace ucpf { namespace yesod { namespace mpl {

namespace detail {

template <typename Sequence, typename Inserter>
struct copy_impl : fold<
	Sequence, typename Inserter::state, typename Inserter::operation
> {};

template <typename Sequence, typename Inserter>
struct reverse_copy_impl : reverse_fold<
	Sequence, typename Inserter::state, typename Inserter::operation
> {};

template <typename Operation, typename Predicate>
struct copy_if_op {
	template <typename Sequence, typename T>
	struct apply : eval_if<
		typename mpl::apply<Predicate, T>::type,
		mpl::apply<Operation, Sequence, T>,
		identity<Sequence>
	> {};
};

template <typename Sequence, typename Predicate, typename Inserter>
struct copy_if_impl : fold<
	Sequence, typename Inserter::state,
	protect<copy_if_op<typename Inserter::operation, Predicate>>
> {};

template <typename Sequence, typename Predicate, typename Inserter>
struct reverse_copy_if_impl : reverse_fold<
	Sequence, typename Inserter::state,
	protect<copy_if_op<typename Inserter::operation, Predicate>>
> {};

}

template <typename...>
struct copy;

template <>
struct copy<> {
	template <typename T0, typename T1, typename... Tn>
	struct apply : copy<T0, T1> {};
};

template <typename Tag>
struct lambda<copy<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef copy<> result_;
	typedef copy<> type;
};

template <typename P0>
struct copy<P0> : if_<
	has_push_back<typename clear<P0>::type>,
	detail::copy_impl<P0, back_inserter<typename clear<P0>::type>>,
	detail::reverse_copy_impl<P0, front_inserter<typename clear<P0>::type>>
>::type {};

template <typename P0, typename P1>
struct copy<P0, P1> : detail::copy_impl<P0, P1> {};

template <typename...>
struct reverse_copy;

template <>
struct reverse_copy<> {
	template <typename T0, typename T1, typename... Tn>
	struct apply : reverse_copy<T0, T1> {};
};

template <typename Tag>
struct lambda<reverse_copy<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef reverse_copy<> result_;
	typedef reverse_copy<> type;
};

template <typename P0>
struct reverse_copy<P0> : if_<
	has_push_back<P0>,
	detail::reverse_copy_impl<P0, back_inserter<typename clear<P0>::type>>,
	detail::copy_impl<P0, front_inserter<typename clear<P0>::type>>
>::type {};

template <typename P0, typename P1>
struct reverse_copy<P0, P1> : detail::reverse_copy_impl<P0, P1> {};

template <typename...>
struct copy_if;

template <>
struct copy_if<> {
	template <typename T0, typename T1, typename T2, typename... Tn>
	struct apply : copy_if<T0, T1, T2> {};
};

template <typename Tag>
struct lambda<copy_if<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef copy_if<> result_; 
	typedef copy_if<> type;
};

template <typename P0, typename P1>
struct copy_if<P0, P1> : if_<
	has_push_back<typename clear<P0>::type>,
	detail::copy_if_impl<P0, P1, back_inserter<typename clear<P0>::type>>,
	detail::reverse_copy_if_impl<
		P0, P1, front_inserter<typename clear<P0>::type>
	>
>::type {};

template <typename P0, typename P1, typename P2>
struct copy_if<P0, P1, P2> : detail::copy_if_impl<P0, P1, P2> {};

template <typename...>
struct reverse_copy_if;

template <>
struct reverse_copy_if<> {
	template<typename T0, typename T1, typename T2, typename... Tn>
	struct apply : reverse_copy_if<T0, T1, T2> {};
};

template <typename Tag>
struct lambda<reverse_copy_if<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef reverse_copy_if<> result_;
	typedef reverse_copy_if<> type;
};

template <typename P0, typename P1, typename P2>
struct reverse_copy_if<P0, P1, P2>
: detail::reverse_copy_if_impl<P0, P1, P2> {};

template <typename P0, typename P1>
struct reverse_copy_if<P0, P1>
: if_<
	has_push_back<P0>,
	detail::reverse_copy_if_impl<
		P0, P1, back_inserter<typename clear<P0>::type>
	>,
	detail::copy_if_impl<P0, P1, front_inserter<typename clear<P0>::type>>
>::type { };


}}}

#endif
