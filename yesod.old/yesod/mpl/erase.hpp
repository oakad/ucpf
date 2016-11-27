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

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#if !defined(UCPF_YESOD_MPL_ERASE_DEC_11_2013_1520)
#define UCPF_YESOD_MPL_ERASE_DEC_11_2013_1520

#include <yesod/mpl/fold.hpp>
#include <yesod/mpl/clear.hpp>

namespace ucpf { namespace yesod { namespace mpl {
namespace detail {

template <typename Tag>
struct erase_impl {
	template <
		typename Sequence, typename First,
		typename Last = typename next<First>::type
	> struct apply {
		typedef iterator_range<
			typename begin<Sequence>::type, First
		> first_half_;

		typedef iterator_range<
			Last, typename end<Sequence>::type
		> second_half_;

		typedef typename reverse_fold<
			second_half_,
			typename clear<Sequence>::type,
			push_front<arg<-1>, arg<-1>>
		>::type half_sequence_;

		typedef typename reverse_fold<
			first_half_, half_sequence_,
			push_front<arg<-1>, arg<-1>>
		>::type type;
	};
};

}

template <>
struct erase<> {
	template <typename T0, typename T1, typename T2, typename... Tn>
	struct apply : erase<T0, T1, T2> {};
};

template <typename Tag>
struct lambda<erase<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef erase<> result_;
	typedef erase<> type;
};

template <typename Sequence, typename First>
struct erase<Sequence, First> : detail::erase_impl<
	typename sequence_tag<Sequence>::type
>::template apply<Sequence, First> {};

template <typename Sequence, typename First, typename Last>
struct erase<Sequence, First, Last> : detail::erase_impl<
	typename sequence_tag<Sequence>::type
>::template apply<Sequence, First, Last> {};

}}}

#endif
