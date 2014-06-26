/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_MINA_TO_ASCII_DECIMAL_20140623T2300)
#define UCPF_MINA_TO_ASCII_DECIMAL_20140623T2300

#include <mina/detail/to_ascii_decimal_u.hpp>

namespace ucpf { namespace mina {
namespace detail {

template <typename T, bool IsFloat = false>
struct ascii_decimal_converter {
	template <typename OutputIterator>
	static void apply(OutputIterator &&sink, T v)
	{
		typedef typename std::make_unsigned<T>::type U;

		if (std::is_signed<T>::value) {
			if (v < 0) {
				*sink++ = '-';
				to_ascii_decimal_u<U>(
					std::forward<OutputIterator>(sink),
					U(-v)
				);
			} else {
				*sink++ = '+';
				to_ascii_decimal_u<U>(
					std::forward<OutputIterator>(sink),
					U(v)
				);
			}
		} else
			to_ascii_decimal_u<U>(
				std::forward<OutputIterator>(sink), v
			);
	}
};

template <typename T>
struct ascii_decimal_converter<T, true> {
	template <typename OutputIterator>
	static void apply(OutputIterator &&sink, T v)
	{
		if (std::signbit(v)) {
			*sink++ = '-';
			to_ascii_decimal_f<T>(
				std::forward<OutputIterator>(sink), std::abs(v)
			);
		} else {
			*sink++ = '+';
			to_ascii_decimal_f<T>(
				std::forward<OutputIterator>(sink), v
			);
		}
	}
};

}

template <typename OutputIterator, typename T>
void to_ascii_decimal(OutputIterator &&sink, T v)
{
	detail::ascii_decimal_converter<
		T, std::is_floating_point<T>::value
	>::apply(std::forward<OutputIterator>(sink), v);
}

}}

#endif
