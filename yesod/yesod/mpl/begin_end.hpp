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

#if !defined(UCPF_YESOD_MPL_BEGIN_END_DEC_11_2013_1630)
#define UCPF_YESOD_MPL_BEGIN_END_DEC_11_2013_1630

#include <yesod/mpl/sequence_tag.hpp>
#include <yesod/mpl/detail/begin_end.hpp>

namespace ucpf { namespace yesod { namespace mpl {

template <typename Sequence>
struct begin {
	typedef typename sequence_tag<Sequence>::type tag_;

	typedef typename detail::begin_impl<
		tag_
	>::template apply<Sequence>::type type;
};

template <typename Sequence>
struct end {
	typedef typename sequence_tag<Sequence>::type tag_;

	typedef typename detail::end_impl<
		tag_
	>::template apply<Sequence>::type type;
};

}}}

#endif
