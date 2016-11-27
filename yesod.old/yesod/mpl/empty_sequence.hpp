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
    Copyright (c) 2004 Alexander Nasonov

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#if !defined(UCPF_YESOD_MPL_EMPTY_SEQUENCE_DEC_17_2013_1600)
#define UCPF_YESOD_MPL_EMPTY_SEQUENCE_DEC_17_2013_1600

#include <yesod/mpl/detail/size.hpp>
#include <yesod/mpl/iterator_tags.hpp>

namespace ucpf { namespace yesod { namespace mpl {

struct empty_sequence {
	struct tag;
	struct begin {
		typedef random_access_iterator_tag category;
	};
	typedef begin end;
};

namespace detail {

template <>
struct size_impl<empty_sequence::tag> {
	template <typename Sequence>
	struct apply : long_<0> {};
};

}

}}}

#endif
