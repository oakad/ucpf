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

#if !defined(UCPF_YESOD_MPL_DETAIL_SIZE_DEC_11_2013_1750)
#define UCPF_YESOD_MPL_DETAIL_SIZE_DEC_11_2013_1750

#include <yesod/mpl/distance.hpp>
#include <yesod/mpl/begin_end.hpp>
#include <yesod/mpl/detail/has_size.hpp>

namespace ucpf { namespace yesod { namespace mpl { namespace detail {

template <typename Tag>
struct size_impl {
	template <typename Sequence>
	struct apply : distance<
		typename begin<Sequence>::type,
		typename end<Sequence>::type
	> {};
};

template <>
struct size_impl<non_sequence_tag> {};

template <typename Tag>
struct O1_size_impl {
	template <typename Sequence>
	struct O1_size_seq_impl : Sequence::size {};

	template <typename Sequence>
	struct apply : std::conditional<
		has_size<Sequence>::value,
		O1_size_seq_impl<Sequence>,
		long_<-1>
	>::type {};
};

template <>
struct O1_size_impl<non_sequence_tag> {};

template <typename Tag>
struct empty_impl {
	template <typename Sequence>
	struct apply : std::is_same<
		typename begin<Sequence>::type,
		typename end<Sequence>::type
	> {};
};

template <>
struct empty_impl<non_sequence_tag> {};

}}}}

#endif
