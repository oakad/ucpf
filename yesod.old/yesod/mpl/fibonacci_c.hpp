/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_YESOD_MPL_FIBONACCI_C_MAR_26_2014_1700)
#define UCPF_YESOD_MPL_FIBONACCI_C_MAR_26_2014_1700

#include <yesod/mpl/package.hpp>

namespace ucpf { namespace yesod { namespace mpl {

template <typename T, T N>
struct fibonacci_c {
	template <typename...>
	struct apply;

	template <typename U, U... Cn, U C0, U C1, U Cp>
	struct apply<
		package_c<U, Cn...>, package_c<U, C0, C1, Cp>
	> : apply<
		package_c<U, Cn..., C0 + C1>,
		typename std::conditional<
			Cp != 0, package_c<U, C1, C0 + C1, Cp - 1>, void
		>::type
	> {};

	template <typename U, U... Cn>
	struct apply<
		package_c<U, Cn...>, void
	> {
		typedef package_c<U, Cn...> type;
	};

	typedef typename apply<
		package_c<T>, package_c<T, 0, 1, N - 1>
	>::type type;
};

}}}

#endif
