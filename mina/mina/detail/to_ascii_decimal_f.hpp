/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * Based on algorithm from:
 *
 *     Printing Floating-Point Numbers Quickly and Accurately with Integers
 *     (Florian Loitsch) in Proceedings of the ACM SIGPLAN 2010 Conference
 *     on Programming Language Design and Implementation, PLDI 2010
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_MINA_DETAIL_TO_ASCII_DECIMAL_F_20140624T2300)
#define UCPF_MINA_DETAIL_TO_ASCII_DECIMAL_F_20140624T2300

#include <mina/detail/binary_pow_10.hpp>

namespace ucpf { namespace mina { namespace detail {

template <typename T>
struct to_ascii_decimal_f;

template <>
struct to_ascii_decimal_f<double> {
	struct fp_type {
		uint64_t m;
		int exp;

		fp_type(uint64_t m_, int exp_)
		: m(m_), exp(exp_)
		{}

		fp_type(fp_type const &other)
		: m(other.m), exp(other.exp)
		{}

		fp_type(double v)
		{
			constexpr static int m_shift = 11;
			constexpr static int e_shift = 52;
			constexpr static int e_offset = 1086;
			union {
				double f;
				uint64_t i;
			} xv;
			xv.f = v;
			m = xv.i << m_shift;
			exp = xv.i >> e_shift;
			exp -= e_offset;
			printf("xx %zx, %d\n", m, exp);
		}

		fp_type operator-(fp_type other) const
		{
			return fp_type(m - other.m, exp);
		}

		fp_type operator*(fp_type other) const
		{
			unsigned __int128 acc(m);
			acc *= other.m;
			acc += 0x8000000000000000ull; /* rounding */
			return fp_type(
				uint64_t(acc >> 64), exp + other.exp + 64
			);
		}

		std::pair<fp_type, fp_type> boundaries() const
		{
			constexpr static int exponent_bias = 1074;
			fp_type high((m << 1) + 1, exp - 1);
			high.normalize();
			fp_type low((m << 1) - 1, exp - 1);
			if (!m && (exp != exponent_bias)) {
				low.m = (m << 2) - 1;
				low.exp = exp - 2;
			}
			low.m <<= low.exp - high.exp;
			low.exp = high.exp;
			printf("bd (%zx, %d) - (%zx, %d)\n", low.m, low.exp, high.m, high.exp);
			return std::make_pair(low, high);
		}

		void normalize()
		{
			auto l(__builtin_clzll(m));
			m <<= l;
			exp -= l;
		}
	};

	template <typename OutputIterator>
	to_ascii_decimal_f(OutputIterator &&sink, double v)
	{
		constexpr static int minimal_target_exp = -60;
		constexpr static int mantissa_size = 64;

		fp_type xv(v);
		xv.normalize();
		printf("in %f, m: %zx, e: %d\n", v, xv.m, xv.exp);
		auto bd(xv.boundaries());

		auto exp_bd(binary_pow_10::lookup_exp_10<double>(
			minimal_target_exp - (xv.exp + mantissa_size)
		));
		fp_type scale(exp_bd.m, exp_bd.exp_2);
		auto s_xv(xv * scale);
		auto s_bd(std::make_pair(bd.first * scale, bd.second * scale));

		printf("s_xv %zx, %d\n", s_xv.m, s_xv.exp);
		printf("s_bd min %zx, %d\n", s_bd.first.m, s_bd.first.exp);
		printf("s_bd max %zx, %d\n", s_bd.second.m, s_bd.second.exp);
		printf("scale %zx, %d, %d\n", exp_bd.m, exp_bd.exp_10, exp_bd.exp_2);
		//int exp_adj;
		//auto x_bcd(to_bcd(s_xv, s_bd, exp_adj));
	}
};

}}}
#endif
