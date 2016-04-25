/*
 * Copyright (c) 2014-2016 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_68986CB6A90888CD877D695C72A6314B)
#define HPP_68986CB6A90888CD877D695C72A6314B

#include <holam/support/int128.hpp>

namespace ucpf { namespace holam { namespace detail {

constexpr uint8_t ascii_decimal_digits[100] = {
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

template <typename T>
struct bcd_converter;

template <>
struct bcd_converter<uint8_t> {
	constexpr static std::size_t bcd_value_size = 2;

	static void to_bcd(uint8_t *out, uint8_t in)
	{
		constexpr uint8_t divider_100(0x29);
		constexpr int shift_100(4);

		uint16_t acc(divider_100);
		acc = (acc * in) >> (8 + shift_100);
		out[1] = ascii_decimal_digits[in - uint8_t(acc * 100)];
		out[0] = ascii_decimal_digits[acc];
        }
};

template <>
struct bcd_converter<uint16_t> {
	constexpr static std::size_t bcd_value_size = 3;

	static void to_bcd(uint8_t *out, uint16_t in)
	{
		constexpr uint16_t divider_1e4(0xa36f);
		constexpr int shift_1e4(14); // adj = true

		uint32_t acc(divider_1e4);
		acc *= in;
		acc >>= 16;
		acc += in;
		acc >>= shift_1e4;
		out[0] =  ascii_decimal_digits[acc];
		to_bcd_4(out + 1, in - uint16_t(acc * 10000));
	}

	static void to_bcd_4(uint8_t *out, uint16_t in)
	{
		constexpr uint16_t divider_100(0x47af);
		constexpr int shift_100(7); // adj = true

		uint32_t acc(divider_100);
		acc *= in;
		acc >>= 16;
		acc += in;
		acc >>= shift_100;
		out[0] = ascii_decimal_digits[acc];
		out[1] = ascii_decimal_digits[in - uint16_t(acc * 100)];
	}
};

template <>
struct bcd_converter<uint32_t> {
	constexpr static std::size_t bcd_value_size = 5;

	static void to_bcd(uint8_t *out, uint32_t in)
	{
		constexpr uint32_t divider_1e8(0x55e63b89);
		constexpr int shift_1e8(25);

		uint64_t acc(divider_1e8);
		acc *= in;
		acc >>= 32 + shift_1e8;
		out[0] = ascii_decimal_digits[acc];
		to_bcd_8(out + 1, in - uint32_t(acc * 100000000));
	}

	static void to_bcd_8(uint8_t *out, uint32_t in)
	{
		constexpr uint32_t divider_1e4(0xd1b71759);
		constexpr int shift_1e4(13);

		uint64_t acc(divider_1e4);
		acc *= in;
		acc >>= 32 + shift_1e4;
		bcd_converter<uint16_t>::to_bcd_4(out, uint16_t(acc));
		bcd_converter<uint16_t>::to_bcd_4(out + 2, uint16_t(
			in - uint32_t(acc * 10000)
		));
	}
};

template <>
struct bcd_converter<uint64_t> {
	constexpr static std::size_t bcd_value_size = 10;

	static void to_bcd(uint8_t *out, uint64_t in)
	{
		constexpr uint64_t divider_10e8(0xabcc77118461cefd);
		constexpr int shift_10e8(26);

		uint128_t acc((in >> 32) * 94967296 + (in & 0xffffffff));
		uint64_t carry(
			(acc * divider_10e8) >> (64 + shift_10e8)
		);
		bcd_converter<uint32_t>::to_bcd_8(
			out + 6, uint32_t(acc - carry * 100000000)
		);

		acc = (in >> 32) * 42;
		acc += carry;
		carry = (acc * divider_10e8) >> (64 + shift_10e8);
		bcd_converter<uint32_t>::to_bcd_8(
			out + 2, uint32_t(acc - carry * 100000000)
		);

		bcd_converter<uint16_t>::to_bcd_4(out, uint16_t(carry));
        }
};

template <>
struct bcd_converter<uint128_t> {
	constexpr static std::size_t bcd_value_size = 20;

	static void to_bcd(uint8_t *out, uint128_t in)
	{
		constexpr uint64_t divider_10e8(0xabcc77118461cefd);
		constexpr int shift_10e8(26);

		uint32_t xx[4] = {
			uint32_t(in),
			uint32_t(in >> 32),
			uint32_t(in >> 64),
			uint32_t(in >> 96)
		};

		uint64_t xy[4] = {
			uint64_t(xx[3]) * 43950336
			+ uint64_t(xx[2]) * 9551616
			+ uint64_t(xx[1]) * 94967296
			+ uint64_t(xx[0]),
			uint64_t(xx[3]) * 43375935
			+ uint64_t(xx[2]) * 67440737
			+ uint64_t(xx[1]) * 42,
			uint64_t(xx[3]) * 16251426
			+ uint64_t(xx[2]) * 1844,
			uint64_t(xx[3]) * 79228
		};

		int out_pos(16);
		uint64_t carry(0);
		for (int in_pos(0); in_pos < 4; ++in_pos) {
			uint128_t acc(xy[in_pos]);
			acc += carry;
			carry = uint64_t(
				(acc * divider_10e8) >> (64 + shift_10e8)
			);
			bcd_converter<uint32_t>::to_bcd_8(
				out + out_pos,
				uint32_t(acc - carry * 100000000)
			);
			out_pos -= 4;
		}
		bcd_converter<uint32_t>::to_bcd_8(out, uint32_t(carry));
        }
};

std::size_t bcd_count_digits(uint8_t const *val, std::size_t count)
{
	std::size_t rv(count * 2);

	for (std::size_t c(0); c < count; ++c) {
		if (val[c]) {
			if (!(val[c] & 0xf0))
				rv -= 1;

			break;
		} else
			rv -= 2;
	}
	return rv;
}

template <typename ArgDef>
void emit_bcd_value(
	uint8_t const *val, std::size_t pos, std::size_t count, ArgDef &arg_def
)
{
	if (pos & 1) {
		arg_def.formatter.emit_char(0x30 | (val[pos >> 1] & 0xf));
		++pos;
		--count;
	}

	while (count >= 2) {
		arg_def.formatter.emit_char(0x30 | (val[pos >> 1] >> 4));
		arg_def.formatter.emit_char(0x30 | (val[pos >> 1] & 0xf));
		pos -= 2;
		count -= 2;
	}

	if (count)
		arg_def.formatter.emit_char(0x30 | (val[pos >> 1] >> 4));
}

template <typename T, typename ArgDef, bool IsSighed = true>
struct format_integral_t {
	static void apply(T const &val_, ArgDef &arg_def)
	{
		typedef typename std::make_unsigned<T>::type u_type;
		typedef bcd_converter<u_type> cnv_type;
		uint8_t bcd_value[cnv_type::bcd_value_size];

		bool sign = std::signbit(val_);
		u_type val(sign ? -val_ : val_);

		cnv_type::to_bcd(bcd_value, val);
		auto m_size(bcd_count_digits(
			bcd_value, cnv_type::bcd_value_size
		));

		if (sign)
			arg_def.formatter.emit_char('-');

		emit_bcd_value(
			bcd_value,
			2 * cnv_type::bcd_value_size - m_size,
			m_size,
			arg_def
		);
	}
};

template <typename T, typename ArgDef>
struct format_integral_t<T, ArgDef, false> {
	static void apply(T const &val, ArgDef &arg_def)
	{
		typedef bcd_converter<T> cnv_type;
		uint8_t bcd_value[cnv_type::bcd_value_size];

		cnv_type::to_bcd(bcd_value, val);
		auto m_size(bcd_count_digits(
			bcd_value, cnv_type::bcd_value_size
		));

		emit_bcd_value(
			bcd_value,
			2 * cnv_type::bcd_value_size - m_size,
			m_size,
			arg_def
		);
	}
};

template <typename T, typename ArgDef>
void format_integral(T const &val, ArgDef &arg_def)
{
	format_integral_t<
		T, ArgDef, std::is_signed<T>::value
	>::apply(val, arg_def);
}

}}}
#endif
