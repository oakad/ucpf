/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

/*=============================================================================
    Based on implementation of boost::mpl library:

    Copyright (c) 2001-2002 Peter Dimov
    Copyright (c) 2001-2004 Aleksey Gurtovoy

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#if !defined(UCPF_YESOD_MPL_ARG_DEC_12_2013_1400)
#define UCPF_YESOD_MPL_ARG_DEC_12_2013_1400

#include <yesod/mpl/void.hpp>
#include <yesod/mpl/integral.hpp>

namespace ucpf { namespace yesod { namespace mpl {

template <long N, long Pos = 0>
struct arg {
	constexpr static long value = N;
	typedef arg<N + 1> next;

	template <typename...>
	struct apply;

	template <typename T0>
	struct apply<T0> {
		typedef typename std::conditional<
			!(N - Pos), T0, void_
		>::type type;
	};

	template <typename T0, typename... Tn>
	struct apply<T0, Tn...> {
		typedef typename std::conditional<
			!(N - Pos), T0,
			typename arg<
				N, Pos + 1
			>::template apply<Tn...>::type
		>::type type;
	};
};

template <>
struct arg<-1> {
	constexpr static long value = -1;

	template <typename...>
	struct apply;

	template <typename T0>
	struct apply<T0> {
		typedef T0 type;
	};

	template <typename T0, typename... Tn>
	struct apply<T0, Tn...> {
		typedef T0 type;
	};
};

template <typename T>
struct is_placeholder : false_type {};

template <long N>
struct is_placeholder<arg<N, 0>> : true_type {};

}}}

#endif
