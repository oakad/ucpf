/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

/*=============================================================================
    Based on extensions to boost::mpl library:

    Copyright (c) 2006-2010 Larry Evans

    Permission to copy, use, modify, sell and distribute this software is
    granted provided this copyright notice appears in all copies. This software
    is provided "as is" without express or implied warranty, and with no claim
    as to its suitability for any purpose.
==============================================================================*/

#if !defined(UCPF_YESOD_MPL_APPLY_DEC_11_2013_1610)
#define UCPF_YESOD_MPL_APPLY_DEC_11_2013_1610

#include <yesod/mpl/lambda_fwd.hpp>
#include <yesod/mpl/apply_wrap.hpp>

namespace ucpf { namespace yesod { namespace mpl {

template <typename...>
struct apply;

template <typename F>
struct apply<F> : apply_wrap<typename lambda<F>::type> {};

template <typename F, typename... Tn>
struct apply<F, Tn...> : apply_wrap<typename lambda<F>::type, Tn...> {};

}}}

#endif
