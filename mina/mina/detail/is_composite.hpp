/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_MINA_DETAIL_IS_COMPOSITE_20140523T1630)
#define UCPF_MINA_DETAIL_IS_COMPOSITE_20140523T1630

#include <type_traits>

namespace ucpf { namespace mina { namespace detail {

template <typename T, typename P>
struct is_composite {
	template <typename U, U>
	struct wrapper {};

	template <typename U>
	static std::true_type test(
		U *, wrapper<void (U::*)(P &, bool), &U::mina_pack> * = nullptr
	);

	static std::false_type test(...);

	typedef decltype(test(static_cast<T *>(nullptr))) type;
	static constexpr bool value = type::value;
};

}}}

#endif

