/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

/*=============================================================================
    Based on implementation of boost::mpl library:

    Copyright (c) 2000-2008 Aleksey Gurtovoy

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#if !defined(UCPF_YESOD_MPL_MIN_MAX_DEC_17_2013_1730)
#define UCPF_YESOD_MPL_MIN_MAX_DEC_17_2013_1730

#include <yesod/mpl/if.hpp>
#include <yesod/mpl/comparison.hpp>

namespace ucpf { namespace yesod { namespace mpl {

template <typename...>
struct min;

template <>
struct min<> {
	template <typename... Tn>
	struct apply : min<Tn...> {};
};

template <typename Tag>
struct lambda<min<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef min<> result_;
	typedef min<> type;
};

template <typename T0, typename T1>
struct min<T0, T1> : std::conditional<less<T0, T1>::value, T0, T1> {};

template <typename T0, typename... Tn>
struct min<T0, Tn...> : std::conditional<
	less<T0, typename min<Tn...>::type>::value,
	T0, typename min<Tn...>::type
> {};

template <typename...>
struct max;

template <>
struct max<> {
	template <typename... Tn>
	struct apply : max<Tn...> {};
};

template <typename Tag>
struct lambda<max<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef max<> result_;
	typedef max<> type;
};

template <typename T0, typename T1>
struct max<T0, T1> : std::conditional<less<T0, T1>::value, T1, T0> {};

template <typename T0, typename... Tn>
struct max<T0, Tn...> : std::conditional<
	less<T0, typename max<Tn...>::type>::value,
	typename max<Tn...>::type, T0
> {};

}}}

#endif
