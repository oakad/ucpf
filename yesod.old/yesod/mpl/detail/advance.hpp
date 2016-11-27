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

/*=============================================================================
    Based on extensions to boost::mpl library:

    Copyright (c) 2006-2010 Larry Evans

    Permission to copy, use, modify, sell and distribute this software is
    granted provided this copyright notice appears in all copies. This software
    is provided "as is" without express or implied warranty, and with no claim
    as to its suitability for any purpose.
==============================================================================*/

#if !defined(UCPF_YESOD_MPL_DETAIL_ADVANCE_DEC_11_2013_1610)
#define UCPF_YESOD_MPL_DETAIL_ADVANCE_DEC_11_2013_1610

#include <yesod/mpl/next_prior.hpp>
#include <yesod/mpl/apply_wrap.hpp>
#include <yesod/mpl/comparison.hpp>
#include <yesod/mpl/arithmetic.hpp>

namespace ucpf { namespace yesod { namespace mpl { namespace detail {

template <long N> 
struct advance_backward {
	constexpr static long unrolling_limit = 1;

	template <typename Iterator>
	struct apply {
		typedef typename apply_wrap<
			advance_backward<unrolling_limit>, Iterator
		>::type chunk_result_;

		typedef typename apply_wrap<
			advance_backward<(
				(N - unrolling_limit) < 0
				? 0 : N - unrolling_limit
			)> , chunk_result_
		>::type type;
	};
};

template <>
struct advance_backward<0> {
	template <typename Iterator>
	struct apply {
		typedef Iterator iter0;
		typedef iter0 type;
	};
};

template <>
struct advance_backward<1> {
	template <typename Iterator>
	struct apply {
		typedef Iterator iter0;
		typedef typename prior<iter0>::type iter1;
		typedef iter1 type;
	};
};

template <long N> 
struct advance_forward {
	constexpr static long unrolling_limit = 1;

	template <typename Iterator>
	struct apply {
		typedef typename apply_wrap<
			advance_forward<unrolling_limit>, Iterator
		>::type chunk_result_;

		typedef typename apply_wrap<
			advance_forward<(
				(N - unrolling_limit) < 0
				? 0 : N - unrolling_limit
			)>, chunk_result_
		>::type type;
	};
};

template <>
struct advance_forward<0> {
	template <typename Iterator>
	struct apply {
		typedef Iterator iter0;
		typedef iter0 type;
	};
};

template <>
struct advance_forward<1> {
	template <typename Iterator>
	struct apply {
		typedef Iterator iter0;
		typedef typename next<iter0>::type iter1;
		typedef iter1 type;
	};
};

template <typename Tag>
struct advance_impl {
	template <typename Iterator, typename N>
	struct apply {
		typedef typename less<N, long_<0>>::type backward_;

		typedef typename if_<backward_, negate<N>, N>::type offset_;

		typedef typename if_<
			backward_,
			advance_backward<offset_::value>,
			advance_forward<offset_::value>
			>::type f_;

		typedef typename apply_wrap<f_, Iterator>::type type;
        };
};

}}}}

#endif
