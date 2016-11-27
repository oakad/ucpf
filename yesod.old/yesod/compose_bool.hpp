/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_YESOD_COMPOSE_BOOL_NOV_14_2013_1140)
#define UCPF_YESOD_COMPOSE_BOOL_NOV_14_2013_1140

#include <type_traits>

namespace ucpf { namespace yesod {

template <typename W, W w, bool... Vn>
struct compose_bool;

template <typename W, W w, bool V0>
struct compose_bool<W, w, V0> {
	constexpr static size_t ord = 1;
	constexpr static W value = V0 ? w | W(1) : w & ~W(1);
	typedef std::integral_constant<W, value> type;
};

template <typename W, W w, bool V0, bool... Vn>
struct compose_bool<W, w, V0, Vn...> {
	constexpr static size_t ord = 1 + compose_bool<W, w, Vn...>::ord;
	constexpr static W value = V0 ? (
		compose_bool<W, w, Vn...>::value
		| (W(1) << compose_bool<W, w, Vn...>::ord)
	) : (
		compose_bool<W, w, Vn...>::value
		& ~(W(1) << compose_bool<W, w, Vn...>::ord)
	);

	typedef std::integral_constant<W, value> type;
};

}}
#endif
