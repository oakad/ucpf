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
    Copyright (c) 2003      Jaap Suter

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#if !defined(UCPF_YESOD_MPL_BITWISE_DEC_17_2013_1530)
#define UCPF_YESOD_MPL_BITWISE_DEC_17_2013_1530

#include <yesod/mpl/integral.hpp>
#include <yesod/mpl/lambda_fwd.hpp>
#include <yesod/mpl/numeric_cast.hpp>
#include <yesod/mpl/detail/largest_int.hpp>

namespace ucpf { namespace yesod { namespace mpl {
namespace detail {

template <typename T>
struct bitand_tag {
	typedef typename T::tag type;
};

template <typename T>
struct bitor_tag {
	typedef typename T::tag type;
};

template <typename T>
struct bitxor_tag {
	typedef typename T::tag type;
};

template <typename T>
struct shift_left_tag {
	typedef typename T::tag type;
};

template <typename T>
struct shift_right_tag {
	typedef typename T::tag type;
};

template <typename Tag0, typename Tag1>
struct bitand_impl
: std::conditional<
	(Tag0::value > Tag1::value),
	detail::cast2nd_impl<bitand_impl<Tag0, Tag0>, Tag0, Tag1>,
	detail::cast1st_impl<bitand_impl<Tag1, Tag1>, Tag0, Tag1>
>::type {};

template <>
struct bitand_impl<integral_c_tag, integral_c_tag> {
	template <typename N0, typename N1>
	struct apply : integral_constant<
		typename detail::largest_int<
			typename N0::value_type,
			typename N1::value_type
		>::type, (N0::value & N1::value)
	> {};
};

template <typename Tag0, typename Tag1>
struct bitor_impl
: std::conditional<
	(Tag0::value > Tag1::value),
	detail::cast2nd_impl<bitor_impl<Tag0, Tag0>, Tag0, Tag1>,
	detail::cast1st_impl<bitor_impl<Tag1, Tag1>, Tag0, Tag1>
>::type {};

template <>
struct bitor_impl<integral_c_tag, integral_c_tag> {
	template <typename N0, typename N1>
	struct apply : integral_constant<
		typename detail::largest_int<
			typename N0::value_type,
			typename N1::value_type
		>::type, (N0::value | N1::value)
	> {};
};

template <typename Tag0, typename Tag1>
struct bitxor_impl : std::conditional<
	(Tag0::value > Tag1::value),
	detail::cast2nd_impl<bitxor_impl<Tag0, Tag0>, Tag0, Tag1>,
	detail::cast1st_impl<bitxor_impl<Tag1, Tag1>, Tag0, Tag1>
>::type {};

template <>
struct bitxor_impl<integral_c_tag, integral_c_tag> {
	template <typename N0, typename N1>
	struct apply : integral_constant<
		typename detail::largest_int<
			typename N0::value_type,
			typename N1::value_type
		>::type, (N0::value ^ N1::value)
	> {};
};

template <typename Tag0, typename Tag1>
struct shift_left_impl
: std::conditional<
	(Tag0::value > Tag1::value),
	detail::cast2nd_impl<shift_left_impl<Tag0, Tag0>, Tag0, Tag1>,
	detail::cast1st_impl<shift_left_impl<Tag1, Tag1>, Tag0, Tag1>
>::type {};

template<>
struct shift_left_impl<integral_c_tag, integral_c_tag> {
	template <typename N, typename S>
	struct apply : integral_constant<
		typename N::value_type, (N::value << S::value)
	> {};
};

template <typename Tag0, typename Tag1>
struct shift_right_impl
: std::conditional<
	(Tag0::value > Tag1::value),
	detail::cast2nd_impl<shift_right_impl<Tag0, Tag0>, Tag0, Tag1>,
	detail::cast1st_impl<shift_right_impl<Tag1, Tag1>, Tag0, Tag1>
>::type {};

template<>
struct shift_right_impl<integral_c_tag, integral_c_tag> {
	template <typename N, typename S>
	struct apply : integral_constant<
		typename N::value_type, (N::value >> S::value)
	> {};
};

}

template <typename...>
struct bitand_;

template <>
struct bitand_<> {
	template <typename... Tn>
	struct apply : bitand_<Tn...> {};
};

template <typename Tag>
struct lambda<bitand_<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef bitand_<> result_;
	typedef bitand_<> type;
};

template <typename T0, typename T1>
struct bitand_<T0, T1> : detail::bitand_impl<
	typename detail::bitand_tag<T0>::type,
	typename detail::bitand_tag<T1>::type
>::template apply<T0, T1>::type {};

template <typename T0, typename T1, typename... Tn>
struct bitand_<T0, T1, Tn...> : bitand_<bitand_<T0, T1>, Tn...>
{};

template <typename...>
struct bitor_;

template <>
struct bitor_<> {
	template <typename... Tn>
	struct apply : bitor_<Tn...> {};
};

template <typename Tag>
struct lambda<bitor_<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef bitor_<> result_;
	typedef bitor_<> type;
};

template <typename T0, typename T1>
struct bitor_<T0, T1> : detail::bitor_impl<
	typename detail::bitor_tag<T0>::type,
	typename detail::bitor_tag<T1>::type
>::template apply<T0, T1>::type {};

template <typename T0, typename T1, typename... Tn>
struct bitor_<T0, T1, Tn...> : bitor_<bitor_<T0, T1>, Tn...>
{};

template <typename...>
struct bitxor_;

template <>
struct bitxor_<> {
	template <typename... Tn>
	struct apply : bitxor_<Tn...> {};
};

template <typename Tag>
struct lambda<bitxor_<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef bitxor_<> result_;
	typedef bitxor_<> type;
};

template <typename T0, typename T1>
struct bitxor_<T0, T1> : detail::bitxor_impl<
	typename detail::bitxor_tag<T0>::type,
	typename detail::bitxor_tag<T1>::type
>::template apply<T0, T1>::type {};

template <typename T0, typename T1, typename... Tn>
struct bitxor_<T0, T1, Tn...> : bitxor_<bitxor_<T0, T1>, Tn...>
{};

template <typename...>
struct shift_left;

template <>
struct shift_left<> {
	template <typename... Tn>
	struct apply : shift_left<Tn...> {};
};

template <typename Tag>
struct lambda<shift_left<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef shift_left<> result_;
	typedef shift_left<> type;
};

template <typename T0, typename T1>
struct shift_left<T0, T1> : detail::shift_left_impl<
	typename detail::shift_left_tag<T0>::type,
	typename detail::shift_left_tag<T1>::type
>::template apply<T0, T1>::type {};

template <typename...>
struct shift_right;

template <>
struct shift_right<> {
	template <typename... Tn>
	struct apply : shift_right<Tn...> {};
};

template <typename Tag>
struct lambda<shift_right<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef shift_right<> result_;
	typedef shift_right<> type;
};

template <typename T0, typename T1>
struct shift_right<T0, T1> : detail::shift_right_impl<
	typename detail::shift_right_tag<T0>::type,
	typename detail::shift_right_tag<T1>::type
>::template apply<T0, T1>::type {};

}}}

#endif
