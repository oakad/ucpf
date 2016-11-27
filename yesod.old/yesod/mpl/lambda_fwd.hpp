/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

/*=============================================================================
    Based on implementation of boost::mpl library:

    Copyright (c) 2001-2004 Aleksey Gurtovoy

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#if !defined(UCPF_YESOD_MPL_LAMBDA_FWD_DEC_11_2013_1430)
#define UCPF_YESOD_MPL_LAMBDA_FWD_DEC_11_2013_1430

#include <yesod/mpl/void.hpp>
#include <yesod/mpl/detail/template_arity.hpp>

namespace ucpf { namespace yesod { namespace mpl {

template <typename...>
struct lambda;

template <typename T, typename Tag, typename Arity>
struct lambda<T, Tag, Arity>;

template <typename T, typename Tag>
struct lambda<T, Tag>
: lambda<T, Tag, typename detail::template_arity<T>::type> {};

template <typename T>
struct lambda<T> : lambda<T, void_> {};

}}}

#endif
