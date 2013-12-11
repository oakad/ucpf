/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

/*=============================================================================
    Based on implementation of boost::mpl library:

    Copyright (c) 2004 Aleksey Gurtovoy

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#if !defined(UCPF_YESOD_MPL_TAG_DEC_11_2013_1710)
#define UCPF_YESOD_MPL_TAG_DEC_11_2013_1710

#include <yesod/mpl/void.hpp>
#include <yesod/mpl/detail/has_tag.hpp>

namespace ucpf { namespace yesod { namespace mpl {

namespace detail {

template <typename T>
struct tag_impl {
	typedef typename T::tag type;
};

}

template <typename T, typename Default = void_>
struct tag : std::conditional<
	detail::has_tag<T>::value, detail::tag_impl<T>, Default
>::type {};

}}}

#endif
