/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_YESOD_DETAIL_BELLARD_PI_DIGIT_MAR_13_2014_1820)
#define UCPF_YESOD_DETAIL_BELLARD_PI_DIGIT_MAR_13_2014_1820

#include <cmath>
#include <cstdint>

namespace ucpf { namespace yesod { namespace detail {

struct bellard_pi_digit {
	static int power_mod_2(int exp, int mod)
	{
		int c (1);
		for (int e(1); e < exp; ++e)
			c = (2 * c) % mod;

		return c;
	}

	static double sigma(int p, int a, int b, int c)
	{
		int exp, bprec(-10), n(0), den;
		double prev, sum(0), num, tmp;

		while (true) {
			den = a * n + b;
			exp = c + p - 1 - 10 * n;

			printf("xx %d %d\n", den, exp);
			if (exp > 0)
				num = std::modf(
					(1.0 * power_mod_2(exp, den)) / den,
					&tmp
				);
			else {
				num = 1;
				den <<= -exp;
			}

			prev = sum;
			if (n & 1)
				sum -= num / den;
			else
				sum += num / den;

			if ((sum == prev) && (exp < bprec))
				break;
			++n;
			if (n > 5)
				break;
		}
		return sum;
	}

	bellard_pi_digit(int p_)
	: p(p_)
	{}

	int p;

	operator bool() const
	{
		auto acc(sigma(p, 10, 1, 2));
		acc -= sigma(p, 10, 3, 0);
		acc -= sigma(p, 4, 1, -1);
		acc -= sigma(p, 10, 5, -4);
		acc -= sigma(p, 10, 7, -4);
		acc += sigma(p, 10, 9, -6);
		acc -= sigma(p, 4, 3, -6);

		printf("aaa %f\n", acc);
		return acc > 0.5 || acc < -0.5;
	}

	operator uint32_t()
	{
		uint32_t rv(0);
		for (auto c(0); c < 32; ++c) {
			if (bool(*this))
				rv |= 1;

			rv <<= 1;
			++p;
		}
		return rv;
	}
};

}}}
#endif