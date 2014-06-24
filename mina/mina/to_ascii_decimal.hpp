/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_MINA_TO_ASCII_DECIMAL_20140623T2300)
#define UCPF_MINA_TO_ASCII_DECIMAL_20140623T2300

#include <yesod/bitops.hpp>

namespace ucpf { namespace mina {
namespace detail {

constexpr std::array<uint8_t, 100> ascii_decimal_digits = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
	0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
	0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
	0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
	0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99
};

template <typename OutputIterator, size_t N>
void bcd_to_ascii(OutputIterator &&sink, std::array<uint32_t, N> const &v)
{
	bool skip_zero(true);
	for (auto d: v) {
		int c(-1);
		if (skip_zero) {
			if (d) {
				c = (yesod::fls(d) | 3) - 3;
				skip_zero = false;
			}
		} else
			c = 28;

		for (; c >= 0; c -= 4)
			*sink++ = 0x30 + ((d >> c) & 0xf);
	}

	if (skip_zero)
		*sink++ = 0x30;
}

template <typename OutputIterator>
void to_ascii_decimal_u(OutputIterator &&sink, uint8_t v)
{
	constexpr uint16_t base_divider(0x29);
	constexpr int base_shift(4);

	auto acc(base_divider);
	acc *= v;
	std::array<uint32_t, 1> bv{uint32_t(acc) >> (8 + base_shift)};
	bv[0] = (bv[0] << 8) | ascii_decimal_digits[v - bv[0] * 100];
	bcd_to_ascii(std::forward<OutputIterator>(sink), bv);
}

template <typename T, bool IsFloat = false>
struct ascii_decimal_converter {
	template <typename OutputIterator>
	static void apply(OutputIterator &&sink, T v)
	{
		if (std::is_signed<T>::value) {
			typedef typename std::make_unsigned<T>::type U;

			if (v < 0) {
				*sink++ = '-';
				to_ascii_decimal_u(
					std::forward<OutputIterator>(sink),
					U(-v)
				);
			} else {
				*sink++ = '+';
				to_ascii_decimal_u(
					std::forward<OutputIterator>(sink),
					U(v)
				);
			}
		} else
			to_ascii_decimal_u(
				std::forward<OutputIterator>(sink), v
			);
	}
};

template <typename T>
struct ascii_decimal_converter<T, true> {
	template <typename OutputIterator>
	static void apply(OutputIterator &&sink, T v)
	{
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
