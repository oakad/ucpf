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

		fp_type(fp_type other)
		: m(other.m), exp(other.exp)
		{}

		fp_type(double v)
		{
			auto xv(std::frexp(v, &exp));
			m = std::llround(xv);
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
	};

	static int k_comp(int exp, int a)
	{
		constexpr double inv_log2_10 = 0.30102999566398114;
		return std::ceil((a - exp + 63) * inv_log2_10);
	}

	template <typename OutputIterator>
	to_ascii_decimal_f(OutputIterator &&sink, double v)
	{
		bool sign(std::signbit(v));
		fp_type xv(std::abs(v));
	}
};

}}}
#endif
