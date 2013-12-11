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

#if !defined(UCPF_YESOD_MPL_SIZE_DEC_11_2013_1740)
#define UCPF_YESOD_MPL_SIZE_DEC_11_2013_1740

#include <yesod/mpl/lambda_fwd.hpp>
#include <yesod/mpl/detail/size.hpp>

namespace ucpf { namespace yesod { namespace mpl {

template <>
struct size<> {
	template <typename T0, typename... Tn>
	struct apply : size<T0> {};
};

template <typename Tag>
struct lambda<size<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef size<> result_;
	typedef size<> type;
};

template <typename Sequence>
struct size<Sequence> : detail::size_impl<
	typename sequence_tag<Sequence>::type
>::template apply<Sequence>::type {};

template <>
struct O1_size<> {
	template <typename T0, typename... Tn>
	struct apply : O1_size<T0> {};
};

template <typename Tag>
struct lambda<O1_size<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef O1_size<> result_;
	typedef O1_size<> type;
};

template <typename Sequence>
struct O1_size<Sequence> : detail::O1_size_impl<
	typename sequence_tag<Sequence>::type
>::template apply<Sequence> {};

template <>
struct empty<> {
	template <typename T0, typename... Tn>
	struct apply : empty<T0> {};
};

template <typename Tag>
struct lambda<empty<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef empty<> result_;
	typedef empty<> type;
};

template <typename Sequence>
struct empty : detail::empty_impl<
	typename sequence_tag<Sequence>::type
>::template apply<Sequence>::type {};

}}}

#endif
