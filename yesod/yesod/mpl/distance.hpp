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

#if !defined(UCPF_YESOD_MPL_DISTANCE_DEC_11_2013_1800)
#define UCPF_YESOD_MPL_DISTANCE_DEC_11_2013_1800

#include <yesod/mpl/iter_fold.hpp>
#include <yesod/mpl/sequence_fwd.hpp>
#include <yesod/mpl/iterator_range.hpp>

namespace ucpf { namespace yesod { namespace mpl {
namespace detail {

template <typename Tag>
struct distance_impl {
	template <typename First, typename Last>
	struct apply : iter_fold<
		iterator_range<First, Last>, long_<0>, next<>
	>::type {};
};

}

template <>
struct distance<> {
	template <typename T0, typename T1, typename... Tn>
	struct apply : distance<T0, T1> {};
};

template <typename Tag>
struct lambda<distance<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef distance<> result_;
	typedef distance<> type;
};

template <typename First, typename Last>
struct distance<First, Last> : detail::distance_impl<
	typename tag<First>::type
>::template apply<First, Last> {};

}}}

#endif
