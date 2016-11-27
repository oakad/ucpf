/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_YESOD_MPL_INTEGRAL_DEC_11_2013_1245)
#define UCPF_YESOD_MPL_INTEGRAL_DEC_11_2013_1245

#include <type_traits>

namespace ucpf { namespace yesod { namespace mpl {
namespace detail {

struct integral_c_tag {
	static const int value = 0;
};

}

template <typename T, T v>
struct integral_constant : std::integral_constant<T, v> {
	typedef integral_constant type;
	typedef detail::integral_c_tag tag;
	typedef integral_constant<T, static_cast<T>((v + 1))> next;
	typedef integral_constant<T, static_cast<T>((v - 1))> prior;
};

template <bool value>
using bool_ = integral_constant<bool, value>;

typedef bool_<false> false_type;
typedef bool_<true> true_type;

template <char value>
using char_ = integral_constant<char, value>;

template <unsigned char value>
using uchar_ = integral_constant<unsigned char, value>;

template <short value>
using short_ = integral_constant<short, value>;

template <unsigned short value>
using ushort_ = integral_constant<unsigned short, value>;

template <int value>
using int_ = integral_constant<int, value>;

template <unsigned int value>
using uint_ = integral_constant<unsigned int, value>;

template <long value>
using long_ = integral_constant<long, value>;

template <unsigned long value>
using ulong_ = integral_constant<unsigned long, value>;

}}}

#endif
