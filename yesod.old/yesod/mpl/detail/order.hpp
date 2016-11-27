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

#if !defined(UCPF_YESOD_MPL_DETAIL_ORDER_DEC_11_2013_1730)
#define UCPF_YESOD_MPL_DETAIL_ORDER_DEC_11_2013_1730

#include <yesod/mpl/detail/has_key.hpp>

namespace ucpf { namespace yesod { namespace mpl { namespace detail {

template <typename T>
static T const &ptr_to_ref(T *);

template <typename Sequence, typename Key>
struct x_order_impl : long_<
	sizeof(
		Sequence::order_by_key_(
			ptr_to_ref(static_cast<Sequence *>(nullptr)),
			static_cast<type_wrapper<Key> *>(nullptr)
		)
	)
> {};

template <typename Tag>
struct order_impl {
	template <typename Sequence, typename Key>
	struct apply : if_<
		typename has_key_impl<Tag>::template apply<Sequence, Key>,
		x_order_impl<Sequence, Key>, void_
	>::type {};
};

template <>
struct order_impl<non_sequence_tag> {};

}}}}

#endif
