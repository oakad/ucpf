/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

/*=============================================================================
    Based on implementation of boost::mpl library:

    Copyright (c) 2002-2004 Aleksey Gurtovoy

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#if !defined(UCPF_YESOD_MPL_DETAIL_ITER_APPLY_DEC_16_2013_1315)
#define UCPF_YESOD_MPL_DETAIL_ITER_APPLY_DEC_16_2013_1315

#include <yesod/mpl/apply.hpp>
#include <yesod/mpl/deref.hpp>

namespace ucpf { namespace yesod { namespace mpl { namespace detail {

template <typename F, typename... Iterator>
struct iter_apply : apply<F, typename deref<Iterator>::type...> {};

}}}}

#endif
