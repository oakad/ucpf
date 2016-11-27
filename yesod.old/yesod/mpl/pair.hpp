/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

/*=============================================================================
    Based on implementation of boost::mpl library:

    Copyright (c) 2001-2004 Aleksey Gurtovoy

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#if !defined(UCPF_YESOD_MPL_PAIR_DEC_11_2013_1230)
#define UCPF_YESOD_MPL_PAIR_DEC_11_2013_1230

#include <yesod/mpl/lambda_fwd.hpp>
#include <yesod/mpl/detail/type_wrapper.hpp>

namespace ucpf { namespace yesod { namespace mpl {
namespace detail {

template <typename T>
struct has_second {
	template <typename U>
	static true_type test(
		type_wrapper<U> const volatile *,
		type_wrapper<typename U::second> * = 0
	);

	static false_type test(...);

	typedef decltype(test(static_cast<type_wrapper<T> *>(nullptr))) type;

	static const bool value = type::value;
};

template <typename T, bool has_second_>
struct second_impl {
	typedef typename T::second type;
};

template <typename T>
struct second_impl<T, false> {
	typedef void_ type;
};

}

template <typename...>
struct pair;

template <>
struct pair<> {
	template <typename T0, typename T1, typename... Tn>
	struct apply : pair<T0, T1> {};
};

template <typename Tag>
struct lambda<pair<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef pair<> result_;
	typedef pair<> type;
};

template <typename T0>
struct pair<T0> {
	typedef pair type;
	typedef T0 first;
};

template <typename T0, typename T1>
struct pair<T0, T1> {
	typedef pair type;
	typedef T0 first;
	typedef T1 second;
};

template <typename...>
struct first;

template <>
struct first<> {
	template <typename T0, typename... Tn>
	struct apply : first<T0> {};
};

template <typename Tag>
struct lambda<first<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef first<> result_;
	typedef first<> type;
};

template <typename T>
struct first<T> {
	typedef typename T::first type;
};

template <typename...>
struct second;

template <>
struct second<> {
	template <typename T0, typename... Tn>
	struct apply : second<T0> {};
};

template <typename Tag>
struct lambda<second<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef second<> result_;
	typedef second<> type;
};

template <typename T>
struct second<T> {
	typedef typename detail::second_impl<
		T, detail::has_second<T>::value
	>::type type;
};

}}}

#endif
