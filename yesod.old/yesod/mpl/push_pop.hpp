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

#if !defined(UCPF_YESOD_MPL_PUSH_POP_DEC_12_2013_1420)
#define UCPF_YESOD_MPL_PUSH_POP_DEC_12_2013_1420

#include <yesod/mpl/lambda_fwd.hpp>
#include <yesod/mpl/sequence_tag.hpp>
#include <yesod/mpl/detail/push_pop.hpp>

namespace ucpf { namespace yesod { namespace mpl {

template <>
struct push_back<> {
	template <typename T0, typename T1, typename... Tn>
	struct apply : push_back<T0, T1> {};
};

template <typename Tag>
struct lambda<push_back<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef push_back<> result_;
	typedef push_back<> type;
};

template <typename Sequence, typename T>
struct push_back<Sequence, T> : detail::push_back_impl<
	typename sequence_tag<Sequence>::type
>::template apply<Sequence, T> {};

template <>
struct push_front<> {
	template <typename T0, typename T1, typename... Tn>
	struct apply : push_front<T0, T1> {};
};

template <typename Tag>
struct lambda<push_front<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef push_front<> result_;
	typedef push_front<> type;
};

template <typename Sequence, typename T>
struct push_front<Sequence, T> : detail::push_front_impl<
	typename sequence_tag<Sequence>::type
>::template apply<Sequence, T> {};

template <typename...>
struct has_push_back;

template <>
struct has_push_back<> {
	template <typename T0, typename... Tn>
	struct apply : has_push_back<T0> {};
};

template <typename Tag>
struct lambda<has_push_back<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef has_push_back<> result_;
	typedef has_push_back<> type;
};

template <typename Sequence>
struct has_push_back<Sequence> : detail::has_push_back_impl<
	typename sequence_tag<Sequence>::type
>::template apply<Sequence> {};

template <typename...>
struct has_push_front;

template <>
struct has_push_front<> {
	template <typename T0, typename... Tn>
	struct apply : has_push_front<T0> {};
};

template <typename Tag>
struct lambda<has_push_front<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef has_push_front<> result_;
	typedef has_push_front<> type;
};

template <typename Sequence>
struct has_push_front<Sequence> : detail::has_push_front_impl<
	typename sequence_tag<Sequence>::type
>::template apply<Sequence> {};

template <>
struct pop_back<> {
	template <typename T0, typename... Tn>
	struct apply : pop_back<T0> {};
};

template <typename Tag>
struct lambda<pop_back<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef pop_back<> result_;
	typedef pop_back<> type;
};

template <typename Sequence>
struct pop_back<Sequence> : detail::pop_back_impl<
	typename sequence_tag<Sequence>::type
>::template apply<Sequence> {};

template <>
struct pop_front<> {
	template <typename T0, typename... Tn>
	struct apply : pop_front<T0> {};
};

template <typename Tag>
struct lambda<pop_front<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef pop_front<> result_;
	typedef pop_front<> type;
};

template <typename Sequence>
struct pop_front<Sequence> : detail::pop_front_impl<
	typename sequence_tag<Sequence>::type
>::template apply<Sequence> {};

}}}

#endif
