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
    Copyright (c) 2003-2004 David Abrahams

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#if !defined(UCPF_YESOD_MPL_INSERTER_DEC_16_2013_1420)
#define UCPF_YESOD_MPL_INSERTER_DEC_16_2013_1420

#include <yesod/mpl/push_pop.hpp>

namespace ucpf { namespace yesod { namespace mpl {

template <typename Sequence, typename Operation>
struct inserter {
	typedef Sequence state;
	typedef Operation operation;
};

template <typename Sequence>
struct back_inserter : inserter<Sequence, push_back<>> {};

template <typename Sequence>
struct front_inserter : inserter<Sequence, push_front<>> {};

}}}

#endif
