/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

/*=============================================================================
    Based on extensions to boost::mpl library:

    Copyright (c) 2007-2010 Larry Evans

    Permission to copy, use, modify, sell and distribute this software is
    granted provided this copyright notice appears in all copies. This software
    is provided "as is" without express or implied warranty, and with no claim
    as to its suitability for any purpose.
==============================================================================*/

#if !defined(UCPF_YESOD_MPL_PACKAGE_RANGE_C_DEC_12_2013_1410)
#define UCPF_YESOD_MPL_PACKAGE_RANGE_C_DEC_12_2013_1410

#include <yesod/mpl/if.hpp>
#include <yesod/mpl/package.hpp>

namespace ucpf { namespace yesod { namespace mpl {
namespace detail {

template <typename Tpack, typename Tint>
struct package_range_forward;

template <typename T, T... Cn, T Cbegin, T Cend>
struct package_range_forward<
	package_c<T, Cn...>, package_c<T, Cbegin, Cend>
> : package_range_forward<
	package_c<T, Cend, Cn...>, package_c<T, Cbegin, T(Cend - 1)>
> {};

template <typename T, T... Cn, T Cbegin>
struct package_range_forward<
	package_c<T, Cn...>, package_c<T, Cbegin, Cbegin>
> {
	typedef package_c<T, Cbegin, Cn...> type;
};

template <typename Tpack, typename Tint>
struct package_range_backward;

template <typename T, T... Cn, T Cbegin, T Cend>
struct package_range_backward<
	package_c<T, Cn...>, package_c<T, Cbegin, Cend>
> : package_range_backward<
	package_c<T, Cn..., Cend>, package_c<T, Cbegin, T(Cend - 1)>
> {};

template <typename T, T... Cn, T Cbegin>
struct package_range_backward<
	package_c<T, Cn...>, package_c<T, Cbegin, Cbegin>
> {
	typedef package_c<T, Cn..., Cbegin> type;
};

}

template <typename T, T Cbegin, T Cend>
struct package_range_c : eval_if_c<
	(Cbegin == Cend), package_c<T>,
	eval_if_c<
		(Cbegin < Cend), detail::package_range_forward<
			package_c<T>, package_c<T, Cbegin, T(Cend - 1)>
		>,
		detail::package_range_backward<
			package_c<T>, package_c<T, Cend, T(Cbegin - 1)>
		>
	>
> {};

}}}

#endif
