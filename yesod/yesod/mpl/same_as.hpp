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

#if !defined(UCPF_YESOD_MPL_SAME_AS_DEC_16_2013_1315)
#define UCPF_YESOD_MPL_SAME_AS_DEC_16_2013_1315

#include <yesod/mpl/logical.hpp>

namespace ucpf { namespace yesod { namespace mpl {

template <typename T>
struct same_as {
	template <typename U>
	struct apply : std::is_same<T, U> {};
};

template <typename T>
struct not_same_as {
	template <typename U>
	struct apply : not_<std::is_same<T, U>> {};
};

}}}

#endif
