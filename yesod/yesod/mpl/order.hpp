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

#if !defined(UCPF_YESOD_MPL_ORDER_DEC_16_2013_1330)
#define UCPF_YESOD_MPL_ORDER_DEC_16_2013_1330

#include <yesod/mpl/lambda_fwd.hpp>
#include <yesod/mpl/detail/order.hpp>

namespace ucpf { namespace yesod { namespace mpl {

template <>
struct order<> {
	template <typename T0, typename T1, typename... Tn>
	struct apply : order<T0, T1> {};
};

template <typename Tag>
struct lambda<order<>, Tag, long_<-1>> {
        typedef false_type is_le;
        typedef order<> result_;
        typedef order<> type;
};

template <typename Sequence, typename Key>
struct order<Sequence, Key> : detail::order_impl<
	typename sequence_tag<Sequence>::type
>::template apply<Sequence, Key> {};

}}}

#endif
