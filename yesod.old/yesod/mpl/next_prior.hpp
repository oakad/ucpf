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

#if !defined(UCPF_YESOD_MPL_NEXT_PRIOR_DEC_11_2013_1250)
#define UCPF_YESOD_MPL_NEXT_PRIOR_DEC_11_2013_1250

namespace ucpf { namespace yesod { namespace mpl {

template <typename...>
struct next;

template <>
struct next<> {
	template <typename T0, typename... Tn>
	struct apply : next<T0> {};
};

template <typename T>
struct next<T> {
	typedef typename T::next type;
};

template <typename...>
struct prior;

template <>
struct prior<> {
	template <typename T0, typename... Tn>
	struct apply : prior<T0> {};
};

template <typename T>
struct prior<T> {
	typedef typename T::prior type;
};

}}}

#endif
