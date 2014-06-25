/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

template <>
struct to_ascii_decimal_u<uint64_t> {
	template <typename OutputIterator>
	to_ascii_decimal_u(OutputIterator &&sink, uint64_t v)
	{
		constexpr uint32_t divider_10000(0xd1b71759);
		constexpr int shift_10000(13);

		std::array<uint16_t, 4> xx{
			uint16_t(v), uint16_t(v >> 16), uint16_t(v >> 32),
			uint16_t(v >> 48)
		};

		std::array<uint32_t, 4> xy{
			656u * xx[3] + 7296u * xx[2] + 5536u * xx[1] + xx[0],
			7671u * xx[3] + 9496u * xx[2] + 6u * xx[1],
			4749u * xx[3] + 42u * xx[2],
			281u * xx[3]
		};

		std::array<uint32_t, 3> bv{0, 0, 0};
		int shift(0);
		uint32_t c(0);
		for (uint32_t d: xy) {
			uint64_t acc(d);
			acc += c;
			c = (acc * divider_10000) >> (32 + shift_10000);
			bv[2 - (shift >> 5)] |= to_ascii_decimal_u<
				uint16_t
			>::to_bcd(
				acc - c * 10000
			) << (shift & 0x1f);
			shift += 16;
		}

		bv[2 - (shift >> 5)] |= to_ascii_decimal_u<uint16_t>::to_bcd(
			c
		) << (shift & 0x1f);

		bcd_to_ascii(std::forward<OutputIterator>(sink), bv);
	}
};
