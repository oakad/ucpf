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

#if !defined(UCPF_YESOD_MPL_IDENTITY_DEC_11_2013_1710)
#define UCPF_YESOD_MPL_IDENTITY_DEC_11_2013_1710

#include <yesod/mpl/lambda_fwd.hpp>

namespace ucpf { namespace yesod { namespace mpl {

template <typename...>
struct identity;

template <>
struct identity<> {
	template <typename T0, typename... Tn>
	struct apply : identity<T0> {};
};

template <typename Tag>
struct lambda<identity<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef identity<> result_;
	typedef identity<> type;
};

template <typename T>
struct identity<T> {
	typedef T type;
};

template <typename...>
struct make_identity;

template <>
struct make_identity<> {
	template <typename T0, typename... Tn>
	struct apply : make_identity<T0> {};
};

template <typename Tag>
struct lambda<make_identity<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef make_identity<> result_;
	typedef make_identity<> type;
};

template <typename T>
struct make_identity<T> {
	typedef identity<T> type;
};

template <typename...>
struct protect;

template <>
struct protect<> {
	template <typename T0, typename... Tn>
	struct apply : protect<T0> {};
};

template <typename T>
struct protect<T> : T {
	typedef protect type;
};

}}}

#endif
