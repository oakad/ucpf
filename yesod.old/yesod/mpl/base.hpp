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

#if !defined(UCPF_YESOD_MPL_BASE_DEC_11_2013_1710)
#define UCPF_YESOD_MPL_BASE_DEC_11_2013_1710

#include <yesod/mpl/lambda_fwd.hpp>

namespace ucpf { namespace yesod { namespace mpl {

template <typename...>
struct base;

template <>
struct base<> {
	template <typename T0, typename... Tn>
	struct apply : base<T0> {};
};

template <typename Tag>
struct lambda<base<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef base<> result_;
	typedef base<> type;
};

template <typename T>
struct base<T> {
	typedef typename T::base type;
};

}}}

#endif
