/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

/*=============================================================================
    Based on implementation of boost::mpl library:

    Copyright (c) 2002-2008 Aleksey Gurtovoy

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#if !defined(UCPF_YESOD_MPL_DETAIL_PUSH_POP_DEC_12_2013_1420)
#define UCPF_YESOD_MPL_DETAIL_PUSH_POP_DEC_12_2013_1420

#include <yesod/mpl/detail/has_type.hpp>
#include <yesod/mpl/sequence_tag_fwd.hpp>

namespace ucpf { namespace yesod { namespace mpl { namespace detail {

struct has_push_back_arg {};
struct has_push_front_arg {};

template <typename Tag>
struct pop_back_impl {
	template <typename Sequence>
	struct apply;
};

template <>
struct pop_back_impl<non_sequence_tag> {};

template <typename Tag>
struct pop_front_impl {
	template <typename Sequence>
	struct apply;
};

template <>
struct pop_front_impl<non_sequence_tag> {};

template <typename Tag>
struct push_back_impl {
	template <typename Sequence, typename T>
	struct apply {};
};

template <typename Tag>
struct has_push_back_impl {
	template <typename Sequence>
	struct apply : has_type<push_back<Sequence, has_push_back_arg>> {};
};

template <typename Tag>
struct push_front_impl {
	template <typename Sequence, typename T>
	struct apply {};
};

template <typename Tag>
struct has_push_front_impl {
	template <typename Sequence>
	struct apply : has_type<push_front<Sequence, has_push_front_arg>> {};
};

template <>
struct has_push_back_impl<non_sequence_tag> {};

template <>
struct has_push_front_impl<non_sequence_tag> {};

}}}}

#endif
