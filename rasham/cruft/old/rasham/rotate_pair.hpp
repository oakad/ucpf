/*
 * Copyright (C) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(_RASHAM_ROTATE_PAIR_HPP)
#define _RASHAM_ROTATE_PAIR_HPP

#include <cmath>

namespace rasham
{

template <typename base_type, typename exp_type>
constexpr base_type pow(base_type base, exp_type exp)
{
	return exp ? base * pow(base, exp - 1) : 1;
}

template <
	typename first_type, typename second_type,
	int first_order, int second_order,
	int radix = 10
> static inline void rotate_pair(first_type &first, second_type &second, int n)
{
	if (second_order > first_order)
		return rotate_pair<
			second_type, first_type, second_order,
			first_order, radix
		>(second, first, n);

	constexpr first_type f_max(pow<first_type, int>(
		first_type(radix), first_order
	));
	constexpr second_type s_max(pow<second_type, int>(
		second_type(radix), second_order
	));

	n = n % (first_order + second_order);
	if (n < 0)
		n = first_order + second_order + n;

	if (n <= second_order) {
		first_type p(std::pow<first_type, int>(
			first_type(radix), n
		));

		first_type f1((first % (f_max / p)) * p);
		first_type f2(second / (s_max / p));
		second_type s1((second % (s_max / p)) * p);
		second_type s2(first / (f_max / p));
		first = f1 + f2;
		second = s1 + s2;
	} else if (n > first_order) {
		first_type p(std::pow<first_type, int>(
			first_type(radix), first_order + second_order - n
		));

		first_type f1(first / p);
		first_type f2((second % p) * (f_max / p));
		second_type s1(second / p);
		second_type s2((first % p) * (s_max / p));
		first = f1 + f2;
		second = s1 + s2;
	} else {
		first_type p(std::pow<first_type, int>(
			first_type(radix), n
		));

		first_type f1(first / (f_max / (p / s_max)));
		first_type f2(second * (p / s_max));
		first_type f3((first % (f_max / p)) * p);
		second = (first / (f_max / p)) % s_max;
		first = f1 + f2 + f3;
	}
}

}
#endif
