/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_YESOD_CODER_DETAIL_PI_WORD_MAR_13_2014_1820)
#define UCPF_YESOD_CODER_DETAIL_PI_WORD_MAR_13_2014_1820

#include <cmath>
#include <limits>
#include <cstdint>

namespace ucpf { namespace yesod { namespace coder { namespace detail {

static inline uint32_t bbp_pi_word(uint32_t d)
{
	d <<= 3;
	auto exp_m16([](uint32_t exp, uint32_t mod) -> uint32_t {
		/* (16^exp) % mod */
		uint32_t exp2(31 - __builtin_clz(exp));
		auto p1(exp), p2(uint32_t(1) << exp2);
		uint64_t r(1);

		for (uint32_t c(0); c <= exp2; ++c) {
			if (p1 >= p2) {
				r <<= 4;
				r %= mod;
				p1 -= p2;
			}
			p2 >>= 1;
			if (p2 >= 1) {
				r *= r;
				r %= mod;
			}
		}
		return static_cast<uint32_t>(r);
	});

	auto sigma([d, exp_m16](int j) -> double {
		double s(0);
		uint32_t c(0);

		for (; c < d; ++c) {
			auto mod((c << 3) + j);
			auto p(d - c);
			double t(exp_m16(p, mod));
			s += t / mod;
			s = std::modf(s, &t);
		}

		while (true) {
			auto mod((c << 3) + j);
			double t(ldexp(1.0, -4 * (c - d)));
			t /= mod;
			if (t <= std::numeric_limits<double>::epsilon())
				break;

			s += t;
			s = std::modf(s, &t);
			++c;
		}

		return s;
	});

	auto val(sigma(1) * 4 - sigma(4) * 2 - sigma(5) - sigma(6));
	double dgt;
	val = std::modf(val, &dgt) + 1;

	val = std::modf(std::ldexp(val, 32), &dgt);
	return static_cast<uint32_t>(std::lrint(dgt));
}

static inline uint32_t bellard_pi_word(uint32_t d)
{
	static_assert(
		std::numeric_limits<long double>::digits
		> std::numeric_limits<double>::digits,
		"need true extended precision long double"
	);
	static constexpr uint32_t w0(0x243f6a88);

	if (!d)
		return w0;

	auto d_orig(d * 32 - 6);
	d = d_orig / 10;
	auto x_exp(32 + (d_orig % 10));

	auto exp_m1024([](uint32_t exp, uint32_t mod) -> uint32_t {
		/* (1024^exp) % mod */
		uint32_t exp2(31 - __builtin_clz(exp));
		auto p1(exp), p2(uint32_t(1) << exp2);
		uint64_t r(1);

		for (uint32_t c(0); c <= exp2; ++c) {
			if (p1 >= p2) {
				r <<= 10;
				r %= mod;
				p1 -= p2;
			}
			p2 >>= 1;
			if (p2 >= 1) {
				r *= r;
				r %= mod;
			}
		}
		return static_cast<uint32_t>(r);
	});

	auto sigma([d, exp_m1024](int j, int k) -> long double {
		long double s(0);
		uint32_t c(0);

		for (; c < d; ++c) {
			auto mod((k * c) + j);
			auto p(d - c);
			long double t(exp_m1024(p, mod));
			t /= mod;

			s += c & 1 ? -t : t;
			s = std::modf(s, &t);
		}

		while (true) {
			auto mod((k * c) + j);
			long double t(ldexp(1.0L, -10 * (c - d)));
			t /= mod;
			if (t <= std::numeric_limits<long double>::epsilon())
				break;

			s += c & 1 ? -t : t;
			s = std::modf(s, &t);
			++c;
		}

		return s;
	});

	auto val(
		- sigma(1, 4) * 32 - sigma(3, 4) + sigma(1, 10) * 256
		- sigma(3, 10) * 64 - sigma(5, 10) * 4 - sigma(7, 10) * 4
		+ sigma(9, 10)
	);
	long double dgt;
	val = std::modf(val, &dgt) + 1;

	val = std::modf(std::ldexp(val, x_exp), &dgt);
	return static_cast<uint32_t>(std::llrint(dgt));
}

}}}}
#endif