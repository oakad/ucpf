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

	typedef std::integral_constant<int, 0> none_type;
	typedef std::integral_constant<int, 1> unary_type;
	typedef std::integral_constant<int, 2> binary_type;

	template <typename U>
	static binary_type test(
		U *, wrapper<void (U::*)(P &, bool), &U::mina_pack> * = nullptr
	);

	template <typename U>
	static unary_type test(
		U *, wrapper<void (U::*)(P &), &U::mina_pack> * = nullptr
	);

	static none_type test(...);

	typedef decltype(test(static_cast<T *>(nullptr))) type;
	static constexpr int value = type::value;
};

}}}

#endif

