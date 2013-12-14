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

#if !defined(UCPF_YESOD_MPL_APPLY_WRAP_DEC_11_2013_1610)
#define UCPF_YESOD_MPL_APPLY_WRAP_DEC_11_2013_1610

#include <yesod/mpl/detail/has_apply.hpp>

namespace ucpf { namespace yesod { namespace mpl {
namespace detail {

template <
	typename F,
	typename has_apply_ = typename detail::has_apply<F>::type
> struct apply_wrap0 : F::template apply<> {};

template <typename F>
struct apply_wrap0<F, true_type> : F::apply {};

}

template <typename F, typename... Tn>
struct apply_wrap {
	typedef typename F::template apply<Tn...>::type type;
};

template <typename F>
struct apply_wrap<F> : detail::apply_wrap0<F> {};

}}}

#endif
