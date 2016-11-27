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

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#if !defined(UCPF_YESOD_MPL_UNIQUE_DEC_17_2013_1820)
#define UCPF_YESOD_MPL_UNIQUE_DEC_17_2013_1820

#include <yesod/mpl/fold.hpp>
#include <yesod/mpl/pair.hpp>
#include <yesod/mpl/clear.hpp>
#include <yesod/mpl/inserter.hpp>

namespace ucpf { namespace yesod { namespace mpl {
namespace detail {

template <typename Predicate, typename Operation>
struct unique_op {
	template <typename Pair, typename T>
	struct apply {
		typedef typename first<Pair>::type seq_;
		typedef typename second<Pair>::type prior_;

		typedef typename eval_if<
			and_<
				has_second<Pair>,
				mpl::apply<Predicate, prior_, T>
			>, identity<seq_>, mpl::apply<Operation, seq_, T>
		>::type new_seq_;

		typedef pair<new_seq_, T> type;
	};
};

template <typename Sequence, typename Predicate, typename Inserter>
struct unique_impl : first<
	typename fold<Sequence, pair<typename Inserter::state>,
	protect<unique_op<Predicate, typename Inserter::operation>>>::type
> {};

template <typename Sequence, typename Predicate, typename Inserter>
struct reverse_unique_impl : first<
	typename reverse_fold<
		Sequence, pair<typename Inserter::state>,
		protect<unique_op<Predicate, typename Inserter::operation>>
	>::type
> {};

}

template <typename...>
struct unique;

template <>
struct unique<> {
	template <typename T0, typename T1, typename T2, typename... Tn>
	struct apply : unique<T0, T1, T2> { };
};

template <typename Tag>
struct lambda<unique<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef unique<> result_;
	typedef unique<> type;
};

template <typename P0 , typename P1>
struct unique<P0, P1> : if_<
	has_push_back<typename clear<P0>::type>,
	detail::unique_impl<P0, P1, back_inserter<typename clear<P0>::type>>,
	detail::reverse_unique_impl<
		P0, P1, front_inserter<typename clear<P0>::type>
	>
>::type {};

template <typename P0, typename P1, typename P2>
struct unique<P0, P1, P2> : detail::unique_impl<P0, P1, P2> {};

template <typename...>
struct reverse_unique;

template <>
struct reverse_unique<> {
	template <typename T0, typename T1, typename T2, typename... Tn>
	struct apply : reverse_unique<T0, T1, T2> { };
};

template <typename P0, typename P1>
struct reverse_unique<P0, P1> : if_<
	has_push_back<P0>,
	detail::reverse_unique_impl<
		P0, P1, back_inserter<typename clear<P0>::type>
	> ,
	detail::unique_impl<P0, P1, front_inserter<typename clear<P0>::type>>
>::type {};

template <typename P0, typename P1, typename P2>
struct reverse_unique<P0, P1, P2> : detail::reverse_unique_impl<P0, P1, P2> {};
 
}}}

#endif
