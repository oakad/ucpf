/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_MINA_DETAIL_TO_ASCII_DECIMAL_U_20140624T2300)
#define UCPF_MINA_DETAIL_TO_ASCII_DECIMAL_U_20140624T2300

#include <mina/detail/bcd_to_ascii.hpp>

namespace ucpf { namespace mina { namespace detail {

template <typename T>
struct to_ascii_decimal_u;

template <>
struct to_ascii_decimal_u<uint8_t> {
	template <typename OutputIterator>
	to_ascii_decimal_u(OutputIterator &&sink, uint8_t v)
	{
		constexpr uint16_t divider_100(0x29);
		constexpr int shift_100(4);

		std::array<uint32_t, 1> bv{divider_100};
		bv[0] *= v;
		bv[0] >>= (8 + shift_100);
		bv[0] = (bv[0] << 8) | ascii_decimal_digits[v - bv[0] * 100];
		bcd_to_ascii(std::forward<OutputIterator>(sink), bv);
	}
};

template <>
struct to_ascii_decimal_u<uint16_t> {
	static uint32_t to_bcd(uint16_t v)
	{
		constexpr uint16_t divider_100(0x47af);
		constexpr int shift_100(7);

		uint32_t rv(0);

		auto xv(v);
		for (int s(0); s < 16; s += 8) {
			uint32_t acc(divider_100);
			acc = (acc * xv) >> 16;
			acc = (acc & 0xffff) + xv;
			acc = (acc >> shift_100) & 0xffff;

			rv |= uint32_t(
				ascii_decimal_digits[xv - acc * 100]
			) << s;
			xv = acc;
		}

		rv |= uint32_t(xv) << 16;
		return rv;
	}

	template <typename OutputIterator>
	to_ascii_decimal_u(OutputIterator &&sink, uint16_t v)
	{
		std::array<uint32_t, 1> bv{to_bcd(v)};
		bcd_to_ascii(std::forward<OutputIterator>(sink), bv);
	}
};

template <>
struct to_ascii_decimal_u<uint32_t> {
	static std::array<uint32_t, 2> to_bcd(uint32_t v)
	{
		constexpr uint32_t divider_100(0x51eb851f);
		constexpr int shift_100(5);

		std::array<uint32_t, 2> rv{0, 0};

		auto xv(v);
		for (int s(0); s < 32; s += 8) {
			uint64_t acc(divider_100);
			acc = (acc * xv) >> (32 + shift_100);

			rv[1] |= uint32_t(
				ascii_decimal_digits[xv - acc * 100]
			) << s;
			xv = acc;
		}

		rv[0] = ascii_decimal_digits[xv];
		return rv;
	}

	template <typename OutputIterator>
	to_ascii_decimal_u(OutputIterator &&sink, uint32_t v)
	{
		bcd_to_ascii(std::forward<OutputIterator>(sink), to_bcd(v));
	}
};

template <>
struct to_ascii_decimal_u<uint64_t> {
	template <typename OutputIterator>
	to_ascii_decimal_u(OutputIterator &&sink, uint64_t v)
	{
		constexpr uint64_t divider_10e8(0xabcc77118461cefd);
		constexpr int shift_10e8(26);

		std::array<uint32_t, 2> xx{
			uint32_t(v), uint32_t(v >> 32)
		};

		std::array<uint64_t, 2> xy{
			uint64_t(xx[1]) * 94967296u + uint64_t(xx[0]),
			uint64_t(xx[1]) * 42u
		};

		std::array<uint32_t, 3> bv{0, 0, 0};
		int dp(0);
		uint64_t c(0);
		for (uint64_t d: xy) {
			unsigned __int128 acc(d);
			acc += c;
			c = (acc * divider_10e8) >> (64 + shift_10e8);
			bv[2 - dp] = to_ascii_decimal_u<uint32_t>::to_bcd(
				acc - c * 100000000
			)[1];
			++dp;
		}

		bv[2 - dp] = to_ascii_decimal_u<uint32_t>::to_bcd(c)[1];

		bcd_to_ascii(std::forward<OutputIterator>(sink), bv);
	}
};

template <>
struct to_ascii_decimal_u<unsigned __int128> {
	template <typename OutputIterator>
	to_ascii_decimal_u(OutputIterator &&sink, unsigned __int128 v)
	{
		constexpr uint64_t divider_10e8(0xabcc77118461cefd);
		constexpr int shift_10e8(26);

		std::array<uint32_t, 4> xx{
			uint32_t(v), uint32_t(v >> 32), uint32_t(v >> 64),
			uint32_t(v >> 96)
		};

		std::array<uint64_t, 4> xy{
			uint64_t(xx[3]) * 43950336u
			+ uint64_t(xx[2]) * 9551616u
			+ uint64_t(xx[1]) * 94967296u
			+ uint64_t(xx[0]),
			uint64_t(xx[3]) * 43375935u
			+ uint64_t(xx[2]) * 67440737u
			+ uint64_t(xx[1]) * 42u,
			uint64_t(xx[3]) * 16251426u
			+ uint64_t(xx[2]) * 1844u,
			uint64_t(xx[3]) * 79228u
		};

		std::array<uint32_t, 5> bv{0, 0, 0, 0, 0};
		int dp(0);
		uint64_t c(0);
		for (uint64_t d: xy) {
			unsigned __int128 acc(d);
			acc += c;
			c = (acc * divider_10e8) >> (64 + shift_10e8);
			bv[4 - dp] = to_ascii_decimal_u<uint32_t>::to_bcd(
				acc - c * 100000000
			)[1];
			++dp;
		}

		bv[4 - dp] = to_ascii_decimal_u<uint32_t>::to_bcd(c)[1];

		bcd_to_ascii(std::forward<OutputIterator>(sink), bv);
	}
};

}}}
#endif
