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

#if !defined(UCPF_YESOD_MPL_FRONT_BACK_DEC_12_2013_1700)
#define UCPF_YESOD_MPL_FRONT_BACK_DEC_12_2013_1700

#include <yesod/mpl/lambda_fwd.hpp>
#include <yesod/mpl/sequence_tag.hpp>

namespace ucpf { namespace yesod { namespace mpl {
namespace detail {

template <typename Tag>
struct front_impl {
	template <typename Sequence>
	struct apply {
		typedef typename begin<Sequence>::type iter_;
		typedef typename deref<iter_>::type type;
	};
};

template <>
struct front_impl<non_sequence_tag> {};

template <typename Tag>
struct back_impl {
	template <typename Sequence>
	struct apply {
		typedef typename end<Sequence>::type end_;
		typedef typename prior<end_>::type last_;
		typedef typename deref<last_>::type type;
	};
};

template <>
struct back_impl<non_sequence_tag> {};

}

template <>
struct front<> {
	template <typename T0, typename... Tn>
	struct apply : front<T0> {};
};

template <typename Tag>
struct lambda<front<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef front<> result_;
	typedef front<> type;
};

template <typename Sequence>
struct front<Sequence> : detail::front_impl<
	typename sequence_tag<Sequence>::type
>::template apply<Sequence> {};

template <>
struct back<> {
	template <typename T0, typename... Tn>
	struct apply : back<T0> {};
};

template <typename Tag>
struct lambda<back<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef back<> result_;
	typedef back<> type;
};

template <typename Sequence>
struct back<Sequence> : detail::back_impl<
	typename sequence_tag<Sequence>::type
>::template apply<Sequence> {};

}}}

#endif
