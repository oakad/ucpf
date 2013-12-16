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

#if !defined(UCPF_YESOD_MPL_AT_DEC_16_2013_1300)
#define UCPF_YESOD_MPL_AT_DEC_16_2013_1300

#include <yesod/mpl/detail/at.hpp>
#include <yesod/mpl/sequence_fwd.hpp>

namespace ucpf { namespace yesod { namespace mpl {

template <>
struct at<> {
	template <typename T0, typename T1, typename... Tn>
	struct apply : at<T0, T1> {};
};

template <typename Sequence, typename N>
struct at<Sequence, N> : detail::at_impl<
	typename sequence_tag<Sequence>::type
>::template apply<Sequence, N> {};

template<typename Sequence, long N>
struct at_c : detail::at_impl<
	typename sequence_tag<Sequence>::type
>::template apply<Sequence, long_<N>> {};

}}}

#endif
