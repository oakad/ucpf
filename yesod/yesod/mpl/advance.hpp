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

#if !defined(UCPF_YESOD_MPL_ADVANCE_DEC_11_2013_1610)
#define UCPF_YESOD_MPL_ADVANCE_DEC_11_2013_1610

#include <yesod/mpl/tag.hpp>
#include <yesod/mpl/detail/advance.hpp>

namespace ucpf { namespace yesod { namespace mpl {

template <typename Iterator, typename N>
struct advance : detail::advance_impl<
	typename tag<Iterator>::type
>::template apply<Iterator, N> {};

template <typename Iterator, long N>
struct advance_c : detail::advance_impl<
	typename tag<Iterator>::type
>::template apply<Iterator, long_<N>> {};

}}}

#endif
