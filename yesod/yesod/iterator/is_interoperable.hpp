/*
 * Copyright (c) 2017 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_D975410E690EE6757F8CBAF7DBB52028)
#define HPP_D975410E690EE6757F8CBAF7DBB52028

#include <type_traits>

namespace ucpf::yesod::iterator {

template <typename T0, typename T1>
using is_interoperable = std::integral_constant<
	bool,
	std::is_convertible<T0, T1>::value
	| std::is_convertible<T1, T0>::value
>;

template <typename T0, typename T1, typename Return>
using enable_if_interoperable_t = std::enable_if_t<
	is_interoperable<T0, T1>::value, Return
>;

}
#endif
