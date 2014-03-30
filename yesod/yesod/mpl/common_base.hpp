/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_YESOD_MPL_COMMON_BASE_MAR_29_2014_1300)
#define UCPF_YESOD_MPL_COMMON_BASE_MAR_29_2014_1300

#include <type_traits>

namespace ucpf { namespace yesod { namespace mpl {

template <typename...>
struct common_base;

template <typename T0, typename T1>
struct common_base<T0, T1> {
	static constexpr bool base0
	= std::is_same<T0, T1>::value || std::is_base_of<T0, T1>::value;
	static constexpr bool base1 = std::is_base_of<T1, T0>::value;

	typedef typename std::enable_if<
		base0 || base1, typename std::conditional<
			base0, T0, T1
		>::type
	>::type type;
};

template <typename T0, typename... Tn>
struct common_base<T0, Tn...> : common_base<
	T0, typename common_base<Tn...>::type
> {};

}}}

#endif
