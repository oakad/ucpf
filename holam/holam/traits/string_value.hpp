/*
 * Copyright (c) 2016 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_4E17D50D00941F44FCFD5E01561235B7)
#define HPP_4E17D50D00941F44FCFD5E01561235B7

#include <holam/place_holder.hpp>

namespace ucpf { namespace holam {
namespace detail {

template <typename T, typename U = std::__void_t<>>
struct is_trait_disabled : std::false_type {};

template <typename T>
struct is_trait_disabled<
	T, std::__void_t<typename T::disabled>
>: T::disabled {};

}

namespace traits {

template <typename T, bool IsIntegral, bool IsFloating>
struct string_value_primitive;

template <typename T>
struct string_value_primitive<T, false, false> {
	typedef std::true_type disabled;

	template <typename OutputIterator>
	static bool apply(
		OutputIterator &iter, T const &val, place_holder const &ph
	)
	{
		return false;
	}
};

template <typename T>
struct string_value_primitive<T, true, false> {
	template <typename OutputIterator>
	static bool apply(
		OutputIterator &iter, T const &val, place_holder const &ph
	)
	{
		return true;
	}
};

template <typename T>
struct string_value_primitive<T, false, true> {
};

template <typename T>
struct string_value_primitive<T, true, true>: string_value_primitive<
	T, false, true
> {};

template <typename T>
struct string_value: string_value_primitive<
	T, std::is_integral<T>::value, std::is_floating_point<T>::value
> {
};

template <>
struct string_value<char const *> {
	template <typename OutputIterator>
	static bool apply(
		OutputIterator &iter, char const *val, place_holder const &ph
	)
	{
		while (*val)
			*iter++ = *val++;

		return true;
	}
};

}
}}
#endif
