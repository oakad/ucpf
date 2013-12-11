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

#if !defined(UCPF_YESOD_MPL_ARITHMETIC_DEC_11_2013_1700)
#define UCPF_YESOD_MPL_ARITHMETIC_DEC_11_2013_1700

#include <yesod/mpl/if.hpp>
#include <yesod/mpl/lambda_fwd.hpp>
#include <yesod/mpl/numeric_cast.hpp>
#include <yesod/mpl/detail/largest_int.hpp>

namespace ucpf { namespace yesod { namespace mpl {
namespace detail {

template <typename T>
struct decrement_tag {
	typedef typename T::tag type;
};

template <typename T>
struct divides_tag {
	typedef typename T::tag type;
};

template <typename T>
struct increment_tag {
	typedef typename T::tag type;
};

template <typename T>
struct minus_tag {
	typedef typename T::tag type;
};

template <typename T>
struct modulus_tag {
	typedef typename T::tag type;
};

template <typename T>
struct negate_tag {
	typedef typename T::tag type;
};

template <typename T>
struct plus_tag {
	typedef typename T::tag type;
};

template <typename T>
struct times_tag {
	typedef typename T::tag type;
};

template <typename Tag>
struct decrement_impl;

template <>
struct decrement_impl<integral_c_tag> {
	template <typename N>
	struct apply : integral_constant<
		typename N::value_type, (N::value - 1)
	> {};
};

template <typename Tag0, typename Tag1>
struct divides_impl
: if_c<
	(Tag0::value > Tag1::value),
	detail::cast2nd_impl<divides_impl<Tag0, Tag0>, Tag0, Tag1>,
	detail::cast1st_impl<divides_impl<Tag1, Tag1>, Tag0, Tag1>
>::type {};

template <>
struct divides_impl<integral_c_tag, integral_c_tag> {
	template <typename N0, typename N1>
	struct apply : integral_constant<
		typename detail::largest_int<
			typename N0::value_type,
			typename N1::value_type
		>::type, (N0::value / N1::value)
	> {};
};

template <typename Tag>
struct increment_impl;

template <>
struct increment_impl<integral_c_tag> {
	template <typename N>
	struct apply : integral_constant<
		typename N::value_type, (N::value + 1)
	> {};
};

template <typename Tag0, typename Tag1>
struct minus_impl
: if_c<
	(Tag0::value > Tag1::value),
	detail::cast2nd_impl<minus_impl<Tag0, Tag0>, Tag0, Tag1>,
	detail::cast1st_impl<minus_impl<Tag1, Tag1>, Tag0, Tag1>
>::type {};

template <>
struct minus_impl<integral_c_tag, integral_c_tag> {
	template <typename N0, typename N1>
	struct apply : integral_constant<
		typename detail::largest_int<
			typename N0::value_type,
			typename N1::value_type
		>::type, (N0::value - N1::value)
	> {};
};

template <typename Tag0, typename Tag1>
struct modulus_impl
: if_c<
	(Tag0::value > Tag1::value),
	detail::cast2nd_impl<modulus_impl<Tag0, Tag0>, Tag0, Tag1>,
	detail::cast1st_impl<modulus_impl<Tag1, Tag1>, Tag0, Tag1>
>::type {};

template <>
struct modulus_impl<integral_c_tag, integral_c_tag> {
	template <typename N0, typename N1>
	struct apply : integral_constant<
		typename detail::largest_int<
			typename N0::value_type,
			typename N1::value_type
		>::type, (N0::value % N1::value)
	> {};
};

template <typename Tag>
struct negate_impl;

template <>
struct negate_impl<integral_c_tag> {
	template <typename N>
	struct apply : integral_constant<
		typename N::value_type, (-N::value)
	> {};
};

template <typename Tag0, typename Tag1>
struct plus_impl : if_c<
	(Tag0::value > Tag1::value),
	detail::cast2nd_impl<plus_impl<Tag0, Tag0>, Tag0, Tag1>,
	detail::cast1st_impl<plus_impl<Tag1, Tag1>, Tag0, Tag1>
>::type {};

template <>
struct plus_impl<integral_c_tag, integral_c_tag> {
	template <typename N0, typename N1>
	struct apply : integral_constant<
		typename detail::largest_int<
			typename N0::value_type,
			typename N1::value_type
		>::type, (N0::value + N1::value)
	> {};
};

template <typename Tag0, typename Tag1>
struct times_impl
: if_c<
	(Tag0::value > Tag1::value),
	detail::cast2nd_impl<times_impl<Tag0, Tag0>, Tag0, Tag1>,
	detail::cast1st_impl<times_impl<Tag1, Tag1>, Tag0, Tag1>
>::type {};

template <>
struct times_impl<integral_c_tag, integral_c_tag> {
	template <typename N0, typename N1>
	struct apply : integral_constant<
		typename detail::largest_int<
			typename N0::value_type,
			typename N1::value_type
		>::type, (N0::value * N1::value)
	> {};
};

}

template <typename...>
struct decrement;

template <>
struct decrement<> {
	template <typename T0, typename... Tn>
	struct apply : decrement<T0> {};
};

template <typename Tag>
struct lambda<decrement<>, Tag, int_<-1>> {
	typedef false_type is_le;
	typedef decrement<> result_;
	typedef decrement<> type;
};

template <typename T>
struct decrement<T> : detail::decrement_impl<
	typename detail::decrement_tag<T>::type
>::template apply<T>::type {};

template <typename...>
struct divides;

template <>
struct divides<> {
	template <typename... Tn>
	struct apply : divides<Tn...> {};
};

template <typename Tag>
struct lambda<divides<>, Tag, int_<-1>> {
	typedef false_type is_le;
	typedef divides<> result_;
	typedef divides<> type;
};

template <typename T0, typename T1>
struct divides<T0, T1> : detail::divides_impl<
	typename detail::divides_tag<T0>::type,
	typename detail::divides_tag<T1>::type
>::template apply<T0, T1>::type {};

template <typename T0, typename T1, typename... Tn>
struct divides<T0, T1, Tn...> : divides<divides<T0, T1>, Tn...>
{};

template <typename...>
struct increment;

template <>
struct increment<> {
	template <typename T0, typename... Tn>
	struct apply : increment<T0> {};
};

template <typename Tag>
struct lambda<increment<>, Tag, int_<-1>> {
	typedef false_type is_le;
	typedef increment<> result_;
	typedef increment<> type;
};

template <typename T>
struct increment<T> : detail::increment_impl<
	typename detail::increment_tag<T>::type
>::template apply<T>::type {};

template <typename...>
struct minus;

template <>
struct minus<> {
	template <typename... Tn>
	struct apply : minus<Tn...> {};
};

template <typename Tag>
struct lambda<minus<>, Tag, int_<-1>> {
	typedef false_type is_le;
	typedef minus<> result_;
	typedef minus<> type;
};

template <typename T0, typename T1>
struct minus<T0, T1> : detail::minus_impl<
	typename detail::minus_tag<T0>::type,
	typename detail::minus_tag<T1>::type
>::template apply<T0, T1>::type {};

template <typename T0, typename T1, typename... Tn>
struct minus<T0, T1, Tn...> : minus<minus<T0, T1>, Tn...>
{};

template <typename...>
struct modulus;

template <>
struct modulus<> {
	template <typename... Tn>
	struct apply : modulus<Tn...> {};
};

template <typename Tag>
struct lambda<modulus<>, Tag, int_<-1>> {
	typedef false_type is_le;
	typedef modulus<> result_;
	typedef modulus<> type;
};

template <typename T0, typename T1>
struct modulus<T0, T1> : detail::modulus_impl<
	typename detail::modulus_tag<T0>::type,
	typename detail::modulus_tag<T1>::type
>::template apply<T0, T1>::type {};

template <typename...>
struct negate;

template <>
struct negate<> {
	template <typename T0, typename... Tn>
	struct apply : negate<T0> {};
};

template <typename Tag>
struct lambda<negate<>, Tag, int_<-1>> {
	typedef false_type is_le;
	typedef negate<> result_;
	typedef negate<> type;
};

template <typename T>
struct negate<T> : detail::negate_impl<
	typename detail::negate_tag<T>::type
>::template apply<T>::type {};

template <typename...>
struct plus;

template <>
struct plus<> {
	template <typename... Tn>
	struct apply : plus<Tn...> {};
};

template <typename Tag>
struct lambda<plus<>, Tag, int_<-1>> {
	typedef false_type is_le;
	typedef plus<> result_;
	typedef plus<> type;
};

template <typename T0, typename T1>
struct plus<T0, T1> : detail::plus_impl<
	typename detail::plus_tag<T0>::type,
	typename detail::plus_tag<T1>::type
>::template apply<T0, T1>::type {};

template <typename T0, typename T1, typename... Tn>
struct plus<T0, T1, Tn...> : plus<plus<T0, T1>, Tn...>
{};

template <typename...>
struct times;

template <>
struct times<> {
	template <typename... Tn>
	struct apply : times<Tn...> {};
};

template <typename Tag>
struct lambda<times<>, Tag, int_<-1>> {
	typedef false_type is_le;
	typedef times<> result_;
	typedef times<> type;
};

template <typename T0, typename T1>
struct times<T0, T1> : detail::times_impl<
	typename detail::times_tag<T0>::type,
	typename detail::times_tag<T1>::type
>::template apply<T0, T1>::type {};

template <typename T0, typename T1, typename... Tn>
struct times<T0, T1, Tn...> : times<times<T0, T1>, Tn...>
{};

}}}

#endif
