/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_MINA_DETAIL_PACK_NOV_O7_2013_1700)
#define UCPF_MINA_DETAIL_PACK_NOV_O7_2013_1700

namespace ucpf { namespace mina { namespace detail {

constexpr unsigned long const small_int_mask         = 0x1f;
constexpr unsigned long const small_int_code_offset  = 0x10;
constexpr unsigned long const small_uint_code_offset = 0x30;

enum struct scalar_rank {
	i8   = 0,
	i16  = 1,
	i32  = 2,
	i64  = 3,
	i128 = 4
};

enum struct list_size_rank {
	l3  = 0,
	l8  = 1,
	l16 = 2,
	l24 = 3
};

constexpr std::array<std::array<std::uint8_t, 5>, 4> const int_list_code = {{
	{{ 0xe0, 0xc8, 0xb0, 0x98, 0x80 }},
	{{ 0x7d, 0x7a, 0x77, 0x74, 0x71 }},
	{{ 0x6d, 0x6a, 0x67, 0x64, 0x61 }},
	{{ 0x5d, 0x5a, 0x57, 0x54, 0x51 }}
}};

constexpr std::array<std::array<std::uint8_t, 5>, 4> const uint_list_code = {{
	{{ 0xe8, 0xd0, 0xb8, 0xa0, 0x88 }},
	{{ 0x7e, 0x7b, 0x78, 0x75, 0x72 }},
	{{ 0x6e, 0x6b, 0x68, 0x65, 0x62 }},
	{{ 0x5e, 0x5b, 0x58, 0x55, 0x52 }}
}};

constexpr std::array<std::array<std::uint8_t, 5>, 4> const float_list_code = {{
	{{ 0xf0, 0xd8, 0xc0, 0xa8, 0x90 }},
	{{ 0x7f, 0x7c, 0x79, 0x76, 0x73 }},
	{{ 0x6f, 0x6c, 0x69, 0x66, 0x63 }},
	{{ 0x5f, 0x5c, 0x59, 0x56, 0x53 }}
}};

constexpr uint8_t const tuple_start_code = 0x50;
constexpr uint8_t const byte_skip_code = 0x60;
constexpr uint8_t const tuple_end_code = 0x60;

template <unsigned int N, typename OutputIterator, typename T>
void pack_integral(OutputIterator &sink, T v)
{
	if (!N)
		return;

	*sink++ = v & 0xff;
	pack_integral<N - 1>(sink, v >> 8);
}

template <typename OutputIterator>
bool pack(OutputIterator &sink, std::int32_t v)
{
	if (((v > (-1 - (1 << 5))) && (v < (1 << 5))))
		*sink++ = (v & small_int_mask) + small_int_code_offset;
	else if (((v > (-1 - (1 << 8))) && (v < (1 << 8)))) {
		*sink++ = int_list_code[list_size_rank::l3][scalar_rank::i8];
		detail::pack_integral<1>(sink, v);
	} else if (((v > (-1 - (1 << 16))) && (v < (1 << 16)))) {
		*sink++ = int_list_code[list_size_rank::l3][scalar_rank::i16];
		detail::pack_integral<2>(sink, v);
	} else {
		*sink++ = int_list_code[list_size_rank::l3][scalar_rank::i32];
		detail::pack_integral<4>(sink, v);
	}
}

}}}
#endif
