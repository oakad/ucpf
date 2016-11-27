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

#if !defined(UCPF_YESOD_MPL_IS_EVEN_DEC_17_2013_1820)
#define UCPF_YESOD_MPL_IS_EVEN_DEC_17_2013_1820

#include <yesod/mpl/integral.hpp>
#include <yesod/mpl/lambda_fwd.hpp>

namespace ucpf { namespace yesod { namespace mpl {

template <typename...>
struct is_even;

template <>
struct is_even<> {
	template <typename T0, typename... Tn>
	struct apply : is_even<T0> {};
};

template <typename Tag>
struct lambda<is_even<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef is_even<> result_;
	typedef is_even<> type;
};

template <typename N>
struct is_even<N> : bool_<((N::value % 2) == 0)> {};

}}}

#endif
