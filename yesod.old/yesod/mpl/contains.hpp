/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

/*=============================================================================
    Based on implementation of boost::mpl library:

    Copyright (c) 2002 Eric Friedman
    Copyright (c) 2004 Aleksey Gurtovoy

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#if !defined(UCPF_YESOD_MPL_CONTAINS_DEC_16_2013_1300)
#define UCPF_YESOD_MPL_CONTAINS_DEC_16_2013_1300

#include <yesod/mpl/detail/contains.hpp>

namespace ucpf { namespace yesod { namespace mpl {

template <>
struct contains<> {
	template <typename T0, typename T1, typename... Tn>
	struct apply : contains<T0, T1> {};
};

template <typename Tag>
struct lambda<contains<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef contains<> result_;
	typedef contains<> type;
};

template <typename Sequence, typename T>
struct contains<Sequence, T> : detail::contains_impl<
	typename sequence_tag<Sequence>::type
>::template apply<Sequence, T> {};

}}}

#endif
