/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

/*=============================================================================
    Based on implementation of boost::mpl library:

    Copyright (c) 2002-2004 Aleksey Gurtovoy

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#if !defined(UCPF_YESOD_MPL_AS_SEQUENCE_DEC_16_2013_1650)
#define UCPF_YESOD_MPL_AS_SEQUENCE_DEC_16_2013_1650

#include <yesod/mpl/is_sequence.hpp>
#include <yesod/mpl/single_view.hpp>

namespace ucpf { namespace yesod { namespace mpl {

template <typename...>
struct as_sequence;

template <>
struct as_sequence<> {
	template <typename T0, typename... Tn>
	struct apply : as_sequence<T0> {};
};

template <typename Tag>
struct lambda<as_sequence<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef as_sequence<> result_;
	typedef as_sequence<> type;
};

template <typename T>
struct as_sequence<T> : if_<is_sequence<T>, T, single_view<T>> {};

}}}

#endif
