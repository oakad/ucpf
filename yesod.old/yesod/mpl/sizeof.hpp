/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

/*=============================================================================
    Based on implementation of boost::mpl library:

    Copyright (c) 2000-2003 Aleksey Gurtovoy

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#if !defined(UCPF_YESOD_MPL_SIZEOF_DEC_12_2013_1840)
#define UCPF_YESOD_MPL_SIZEOF_DEC_12_2013_1840

#include <yesod/mpl/integral.hpp>

namespace ucpf { namespace yesod { namespace mpl {

template <std::size_t value>
using size_t = integral_constant<std::size_t, value>;

template <typename T>
struct sizeof_ : mpl::size_t<sizeof(T)> {};

}}}

#endif
