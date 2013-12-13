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

    Copyright (c) 2006-2009 Larry Evans

    Permission to copy, use, modify, sell and distribute this software is
    granted provided this copyright notice appears in all copies. This software
    is provided "as is" without express or implied warranty, and with no claim
    as to its suitability for any purpose.
==============================================================================*/

#if !defined(UCPF_YESOD_MPL_QUOTE_DEC_12_2013_1220)
#define UCPF_YESOD_MPL_QUOTE_DEC_12_2013_1220

#include <yesod/mpl/void.hpp>
#include <yesod/mpl/detail/has_type.hpp>

namespace ucpf { namespace yesod { namespace mpl {
namespace detail {

template <typename T, bool has_type_>
struct quote_impl {
	typedef typename T::type type;
};

template <typename T>
struct quote_impl<T, false> {
	typedef T type;
};

}

template <template <typename...> class F, typename Tag = void_>
struct quote {
	template <typename... Un>
	struct apply : detail::quote_impl<
		F<Un...>, detail::has_type<F<Un...>>::value
	> {};
};

}}}

#endif
