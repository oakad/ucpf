/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_MINA_MBP_RAW_ENCODING_20131118T1700)
#define UCPF_MINA_MBP_RAW_ENCODING_20131118T1700

#include <array>
#include <cstdint>
#include <yesod/float.hpp>
#include <yesod/bitops.hpp>
#include <yesod/compose_bool.hpp>

namespace ucpf { namespace mina { namespace mbp {

template <typename T>
struct custom {
	template <typename OutputIterator>
	static void pack(OutputIterator &&sink, T &&v);

	template <typename ForwardIterator>
	static bool unpack(ForwardIterator &first, ForwardIterator last, T &&v);
};

namespace detail {

constexpr int small_int_neg_base         = -32;
constexpr uint8_t small_int_sign_bit     = 0x10;
constexpr uint8_t small_int_mask         = 0x1f;
constexpr uint8_t small_int_code_offset  = 0x10;
constexpr uint8_t small_uint_code_offset = 0x30;

struct scalar_rank {
	enum {
		ie   = -1,
		i5   = 0,
		i8   = 1,
		i16  = 2,
		i32  = 3,
		i64  = 4,
		i128 = 5
	};

	constexpr static std::array<int, 6> order = {{
		5, 8, 16, 32, 64, 128
	}};

	template <typename T>
	constexpr static int from_type()
	{
		typedef typename std::remove_reference<T>::type Tr;
		typedef std::integral_constant<
			int, yesod::order_base_2(sizeof(Tr)) + 1
		> rv_type;

		return rv_type::value <= i128 ? rv_type::value : ie;
	}

	template <typename T>
	static int from_value(T v)
	{
		typedef typename std::remove_reference<T>::type Tr;
		auto s_rank(from_type<Tr>());
		if (!std::is_integral<Tr>::value)
			return s_rank;

		while (s_rank > 0) {
			auto max_val(Tr(1) << order[s_rank - 1]);
			if (std::is_signed<Tr>::value) {
				max_val >>= 1;
				auto min_val = Tr(-1) - max_val;

				if ((v > min_val) && (v < max_val))
					s_rank--;
				else
					return s_rank;
			} else {
				if (v < max_val)
					s_rank--;
				else
					return s_rank;
			}
		}
		return s_rank;
	}
};

constexpr std::array<int, 6> scalar_rank::order;

constexpr uint8_t small_list_size_mask = 0x07;

struct list_size_rank {
	enum {
		le  = -1,
		l3  = 0,
		l8  = 1,
		l16 = 2,
		l24 = 3
	};

	static int from_size(size_t count)
	{
		if (count <= (1 << 24)) {
			if (count <= (1 << 16)) {
				if (count <= (1 << 8))
					return (count <= (1 << 3)) ? l3 : l8;
				else
					return l16;
			} else
				return l24;
		} else
			return le;
	}
};

struct numeric_type_rank {
	enum {
		n_invalid = -1,
		n_signed = 0,
		n_unsigned = 1,
		n_float = 2
	};
};

constexpr std::array<
	std::array<std::array<uint8_t, 5>, 4>, 3
> list_code = {{
	{{
		{{ 0xe0, 0xc8, 0xb0, 0x98, 0x80 }},
		{{ 0x7d, 0x7a, 0x77, 0x74, 0x71 }},
		{{ 0x6d, 0x6a, 0x67, 0x64, 0x61 }},
		{{ 0x5d, 0x5a, 0x57, 0x54, 0x51 }}
	}},
	{{
		{{ 0xe8, 0xd0, 0xb8, 0xa0, 0x88 }},
		{{ 0x7e, 0x7b, 0x78, 0x75, 0x72 }},
		{{ 0x6e, 0x6b, 0x68, 0x65, 0x62 }},
		{{ 0x5e, 0x5b, 0x58, 0x55, 0x52 }}
	}},
	{{
		{{ 0xf0, 0xd8, 0xc0, 0xa8, 0x90 }},
		{{ 0x7f, 0x7c, 0x79, 0x76, 0x73 }},
		{{ 0x6f, 0x6c, 0x69, 0x66, 0x63 }},
		{{ 0x5f, 0x5c, 0x59, 0x56, 0x53 }}
	}}
}};

struct field_class {
	/* For each header field xx_yyy_zzz:
	 * xx: numeric type rank (11 - illegal)
	 * yyy: list size rank (111 - illegal)
	 * zzz: scalar rank (111 - illegal)
	 */
	constexpr static std::array<uint8_t, 256> header_value_code = {{
		0b11111111, 0b11111111, 0b11111111, 0b11111111,
		0b11111111, 0b11111111, 0b11111111, 0b11111111,
		0b11111111, 0b11111111, 0b11111111, 0b11111111,
		0b11111111, 0b11111111, 0b11111111, 0b11111111, // 0f
		0b00000000, 0b00000000, 0b00000000, 0b00000000,
		0b00000000, 0b00000000, 0b00000000, 0b00000000,
		0b00000000, 0b00000000, 0b00000000, 0b00000000,
		0b00000000, 0b00000000, 0b00000000, 0b00000000, // 1f
		0b00000000, 0b00000000, 0b00000000, 0b00000000,
		0b00000000, 0b00000000, 0b00000000, 0b00000000,
		0b00000000, 0b00000000, 0b00000000, 0b00000000,
		0b00000000, 0b00000000, 0b00000000, 0b00000000, // 2f
		0b01000000, 0b01000000, 0b01000000, 0b01000000,
		0b01000000, 0b01000000, 0b01000000, 0b01000000,
		0b01000000, 0b01000000, 0b01000000, 0b01000000,
		0b01000000, 0b01000000, 0b01000000, 0b01000000, // 3f
		0b01000000, 0b01000000, 0b01000000, 0b01000000,
		0b01000000, 0b01000000, 0b01000000, 0b01000000,
		0b01000000, 0b01000000, 0b01000000, 0b01000000,
		0b01000000, 0b01000000, 0b01000000, 0b01000000, // 4f
		0b11111111, 0b00011101, 0b01011101, 0b10011101,
		0b00011100, 0b01011100, 0b10011100, 0b00011011,
		0b01011011, 0b10011011, 0b00011010, 0b01011010,
		0b10011010, 0b00011001, 0b01011001, 0b10011001, // 5f
		0b11111111, 0b00010101, 0b01010101, 0b10010101,
		0b00010100, 0b01010100, 0b10010100, 0b00010011,
		0b01010011, 0b10010011, 0b00010010, 0b01010010,
		0b10010010, 0b00010001, 0b01010001, 0b10010001, // 6f
		0b11111111, 0b00001101, 0b01001101, 0b10001101,
		0b00001100, 0b01001100, 0b10001100, 0b00001011,
		0b01001011, 0b10001011, 0b00001010, 0b01001010,
		0b10001010, 0b00001001, 0b01001001, 0b10001001, // 7f
		0b00000101, 0b00000101, 0b00000101, 0b00000101,
		0b00000101, 0b00000101, 0b00000101, 0b00000101,
		0b01000101, 0b01000101, 0b01000101, 0b01000101,
		0b01000101, 0b01000101, 0b01000101, 0b01000101, // 8f
		0b10000101, 0b10000101, 0b10000101, 0b10000101,
		0b10000101, 0b10000101, 0b10000101, 0b10000101,
		0b00000100, 0b00000100, 0b00000100, 0b00000100,
		0b00000100, 0b00000100, 0b00000100, 0b00000100, // 9f
		0b01000100, 0b01000100, 0b01000100, 0b01000100,
		0b01000100, 0b01000100, 0b01000100, 0b01000100,
		0b10000100, 0b10000100, 0b10000100, 0b10000100,
		0b10000100, 0b10000100, 0b10000100, 0b10000100, // af
		0b00000011, 0b00000011, 0b00000011, 0b00000011,
		0b00000011, 0b00000011, 0b00000011, 0b00000011,
		0b01000011, 0b01000011, 0b01000011, 0b01000011,
		0b01000011, 0b01000011, 0b01000011, 0b01000011, // bf
		0b10000011, 0b10000011, 0b10000011, 0b10000011,
		0b10000011, 0b10000011, 0b10000011, 0b10000011,
		0b00000010, 0b00000010, 0b00000010, 0b00000010,
		0b00000010, 0b00000010, 0b00000010, 0b00000010, // cf
		0b01000010, 0b01000010, 0b01000010, 0b01000010,
		0b01000010, 0b01000010, 0b01000010, 0b01000010,
		0b10000010, 0b10000010, 0b10000010, 0b10000010,
		0b10000010, 0b10000010, 0b10000010, 0b10000010, // df
		0b00000001, 0b00000001, 0b00000001, 0b00000001,
		0b00000001, 0b00000001, 0b00000001, 0b00000001,
		0b01000001, 0b01000001, 0b01000001, 0b01000001,
		0b01000001, 0b01000001, 0b01000001, 0b01000001, // ef
		0b10000001, 0b10000001, 0b10000001, 0b10000001,
		0b10000001, 0b10000001, 0b10000001, 0b10000001,
		0b11111111, 0b11111111, 0b11111111, 0b11111111,
		0b11111111, 0b11111111, 0b11111111, 0b11111111
	}};

	static field_class from_header(uint8_t h)
	{
		field_class rv;
		auto value_cls(header_value_code[h]);
		rv.numeric_type_r = value_cls >> 6;
		if (rv.numeric_type_r > numeric_type_rank::n_float)
			rv.numeric_type_r = numeric_type_rank::n_invalid;

		rv.list_size_r = (value_cls >> 3) & 7;
		if (rv.list_size_r > list_size_rank::l24)
			rv.list_size_r = list_size_rank::le;

		rv.scalar_r = value_cls & 7;
		if (rv.scalar_r > scalar_rank::i128)
			rv.scalar_r = scalar_rank::ie;

		return rv;
	}

	int scalar_r;
	int list_size_r;
	int numeric_type_r;
};

constexpr uint8_t tuple_start_code = 0x50;
constexpr uint8_t byte_skip_code = 0x60;
constexpr uint8_t tuple_end_code = 0x70;

struct kind_flags {
	enum {
		integral = 1,
		float_t  = 2,
		sequence = 4
	};
};

template <typename T, int Kind, bool IsSequence = false>
struct classify {
	constexpr static int value = ucpf::yesod::compose_bool<
		int, Kind, yesod::is_floating_point<T>::value,
		std::is_integral<T>::value
	>::value;
	typedef typename std::integral_constant<int, value> type;
};

template <typename T, int Kind>
struct classify<T, Kind, true> {
	constexpr static int value = classify<
		typename T::value_type, Kind | kind_flags::sequence, false
	>::value;
	typedef typename std::integral_constant<int, value> type;
};

}
}}}

#endif
