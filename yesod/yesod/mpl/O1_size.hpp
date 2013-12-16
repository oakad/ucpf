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

#if !defined(UCPF_YESOD_MPL_O1_SIZE_DEC_16_2013_1450)
#define UCPF_YESOD_MPL_O1_SIZE_DEC_16_2013_1450

#include <yesod/mpl/lambda_fwd.hpp>
#include <yesod/mpl/detail/has_size.hpp>

namespace ucpf { namespace yesod { namespace mpl {
namespace detail {

template <typename Tag>
struct O1_size_impl {
	template <typename Sequence>
	struct O1_size_seq_impl : Sequence::size {};

	template <typename Sequence>
	struct apply : if_<
		has_size<Sequence>,
		O1_size_seq_impl<Sequence>,
		long_<-1>
	>::type {};
};

}

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

}}}

#endif
