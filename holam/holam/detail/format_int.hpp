/*
 * Copyright (c) 2016 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_68986CB6A90888CD877D695C72A6314B)
#define HPP_68986CB6A90888CD877D695C72A6314B

namespace ucpf { namespace holam { namespace detail {

constexpr std::uint8_t ascii_decimal_digits[100] = {
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
struct bcd_converter<std::uint32_t> {
	typedef std::array<std::uint8_t, 5> bcd_value_type;

	static void to_bcd(bcd_value_type &out, std::uint32_t in)
	{
		constexpr uint32_t divider_100(0x51eb851f);
		constexpr int shift_100(5);

		for (auto c(out.size() - 1); c; --c) {
			std::uint64_t acc(divider_100);
			acc = (acc * in) >> (32 + shift_100);

			out[c] = ascii_decimal_digits[
				in - std::uint32_t(acc * 100)
			];
			in = acc;
		}

		out[0] = ascii_decimal_digits[in];
	}
};

template <std::size_t N>
std::size_t bcd_count_digits(std::array<std::uint8_t, N> const &val)
{
	std::size_t rv(val.size() * 2);

	for (auto d: val) {
		if (d) {
			if (!(d & 0xf0))
				rv -= 1;

			break;
		} else
			rv -= 2;
	}
	return rv;
}

template <std::size_t N, typename ArgDef>
void emit_bcd_value(
	std::array<std::uint8_t, N> const &val, std::size_t count,
	ArgDef &arg_def
)
{
	for (auto c(2 * val.size() - count); c < 2 * val.size(); ++c) {
		if (c & 1)
			arg_def.formatter.emit_char(0x30 | (val[c >> 1] & 0xf));
		else
			arg_def.formatter.emit_char(0x30 | (val[c >> 1] >> 4));
	}
}

template <typename T, typename ArgDef>
void format_signed(T val_, ArgDef &arg_def)
{
	typedef typename std::make_unsigned<T>::type u_type;
	typedef bcd_converter<u_type> cnv_type;
	typename cnv_type::bcd_value_type bcd_value;

	bool sign = std::signbit(val_);
	u_type val(sign ? -val_ : val_);

	cnv_type::to_bcd(bcd_value, val);
	auto m_size(bcd_count_digits(bcd_value));

	if (sign)
		arg_def.formatter.emit_char('-');

	emit_bcd_value(bcd_value, m_size, arg_def);
}

}}}
#endif
