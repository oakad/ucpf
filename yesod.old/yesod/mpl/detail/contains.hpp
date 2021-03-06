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

#if !defined(UCPF_YESOD_MPL_DETAIL_CONTAINS_DEC_16_2013_1300)
#define UCPF_YESOD_MPL_DETAIL_CONTAINS_DEC_16_2013_1300

#include <yesod/mpl/find.hpp>

namespace ucpf { namespace yesod { namespace mpl { namespace detail {

template <typename Tag>
struct contains_impl {
	template <typename Sequence, typename T>
	struct apply : not_<std::is_same<
		typename find<Sequence, T>::type,
		typename end<Sequence>::type
	>> {};
};

template <>
struct contains_impl<non_sequence_tag> {};

}}}}

#endif
