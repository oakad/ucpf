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

#if !defined(UCPF_YESOD_MPL_ITERATOR_TAGS_DEC_12_2013_1230)
#define UCPF_YESOD_MPL_ITERATOR_TAGS_DEC_12_2013_1230

#include <yesod/mpl/integral.hpp>

namespace ucpf { namespace yesod { namespace mpl {

struct forward_iterator_tag : int_<0> {
	typedef forward_iterator_tag type;
};

struct bidirectional_iterator_tag : int_<1> {
	typedef bidirectional_iterator_tag type;
};

struct random_access_iterator_tag : int_<2> {
	typedef random_access_iterator_tag type;
};

}}}

#endif
