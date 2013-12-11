/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

/*=============================================================================
    Based on implementation of boost::mpl library:

    Copyright (c) 2003-2004 Aleksey Gurtovoy

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#if !defined(UCPF_YESOD_MPL_NUMERIC_CAST_DEC_11_2013_1650)
#define UCPF_YESOD_MPL_NUMERIC_CAST_DEC_11_2013_1650

#include <yesod/mpl/apply_wrap.hpp>

namespace ucpf { namespace yesod { namespace mpl {

template <typename SourceTag, typename TargetTag>
struct numeric_cast {
	template <typename N>
	struct apply;
};

namespace detail {

template <typename F, typename Tag0, typename Tag1>
struct cast1st_impl {
	template <typename N0, typename N1>
	struct apply : apply_wrap<
		F, typename apply_wrap<numeric_cast<Tag0, Tag1>, N0>::type, N1
	> {};
};

template <typename F, typename Tag0, typename Tag1>
struct cast2nd_impl {
	template <typename N0, typename N1>
	struct apply : apply_wrap<
		F, N0, typename apply_wrap<numeric_cast<Tag1, Tag0>, N1>::type
	> {};
};

}

}}}

#endif
