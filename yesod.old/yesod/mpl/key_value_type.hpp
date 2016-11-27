/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

/*=============================================================================
    Based on implementation of boost::mpl library:

    Copyright (c) 2003-2004 Aleksey Gurtovoy
    Copyright (c) 2003-2004 David Abrahams

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#if !defined(UCPF_YESOD_MPL_KEY_VALUE_TYPE_DEC_17_2013_1820)
#define UCPF_YESOD_MPL_KEY_VALUE_TYPE_DEC_17_2013_1820

#include <yesod/mpl/lambda_fwd.hpp>
#include <yesod/mpl/sequence_fwd.hpp>

namespace ucpf { namespace yesod { namespace mpl {

template <>
struct key_type<> {
	template <typename T0, typename T1, typename... Tn>
	struct apply : key_type<T0, T1> {};
};

template <typename Tag>
struct lambda<key_type<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef key_type<> result_;
	typedef key_type<> type;
};


template<typename Sequence, typename T>
struct key_type<Sequence, T> : apply_wrap<
	detail::key_type_impl<typename sequence_tag<Sequence>::type>,
	Sequence, T
> {};

template <>
struct value_type<> {
	template <typename T0, typename T1, typename... Tn>
	struct apply : value_type<T0, T1> {};
};

template <typename Tag>
struct lambda<value_type<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef value_type<> result_;
	typedef value_type<> type;
};

template<typename Sequence, typename T>
struct value_type<Sequence, T> : apply_wrap<
	detail::value_type_impl<typename sequence_tag<Sequence>::type>,
	Sequence, T
> {};

}}}

#endif
