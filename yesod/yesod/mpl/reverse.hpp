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

#if !defined(UCPF_YESOD_MPL_REVERSE_DEC_17_2013_1750)
#define UCPF_YESOD_MPL_REVERSE_DEC_17_2013_1750

#include <yesod/mpl/copy.hpp>

namespace ucpf { namespace yesod { namespace mpl {

template <typename...>
struct reverse;

template <>
struct reverse<> {
	template <typename T0, typename T1, typename... Tn>
	struct apply : reverse<T0> {};
};

template <typename Tag>
struct lambda<reverse<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef reverse<> result_;
	typedef reverse<> type; 
};

template <typename Sequence>
struct reverse<Sequence> : reverse_copy<Sequence> {};

template <typename Sequence, typename Inserter>
struct reverse<Sequence, Inserter> : reverse_copy<Sequence, Inserter> {};

}}}

#endif
