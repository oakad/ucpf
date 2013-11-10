/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_MINA_DETAIL_PACK_NOV_O7_2013_1700)
#define UCPF_MINA_DETAIL_PACK_NOV_O7_2013_1700

#include <cstdint>
#include <array>
#include <yesod/is_sequence.hpp>

namespace ucpf { namespace mina { namespace detail {

constexpr uint8_t const small_int_mask         = 0x1f;
constexpr uint8_t const small_int_code_offset  = 0x10;
constexpr uint8_t const small_uint_code_offset = 0x30;

struct scalar_rank {
	enum {
		ie   = -1,
		i8   = 0,
		i16  = 1,
		i32  = 2,
		i64  = 3,
		i128 = 4
	};

	template <typename T>
	constexpr static int from_type()
	{
		typedef std::integral_constant<
			int, 31 - __builtin_clz(sizeof(T))
		> log_type;
		typedef std::integral_constant<
			int,
			((1UL << log_type::value) < sizeof(T))
			? log_type::value + 1 : log_type::value
		> rv_type;

		return rv_type::value <= i128 ? rv_type::value : ie;
	}
};

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
		n_error = -1,
		n_signed = 0,
		n_unsigned = 1,
		n_float = 2
	};

	template <typename T>
	constexpr static int from_type()
	{
		typedef typename std::integral_constant<
			int,
			std::is_integral<T>::value ? 0 : (
				std::is_floating_point<T>::value
				? n_float : n_error
			)
		>::type int_type;
		typedef typename std::integral_constant<
			int,
			std::is_signed<T>::value ? n_signed : (
				std::is_unsigned<T>::value
				? n_unsigned : n_error
			)
		>::type sig_type;

		return int_type::value == 0 ? sig_type::value : int_type::value;
	}
};

constexpr std::array<
	std::array<std::array<uint8_t, 5>, 4>, 3
> const list_code = {{
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

constexpr uint8_t const tuple_start_code = 0x50;
constexpr uint8_t const byte_skip_code = 0x60;
constexpr uint8_t const tuple_end_code = 0x60;

template <unsigned int N, typename OutputIterator, typename T>
typename std::enable_if<N == 1, void>::type pack_integral(
	OutputIterator &&sink, T v
)
{
	*sink++ = static_cast<uint8_t>(v);
}

template <unsigned int N, typename OutputIterator, typename T>
typename std::enable_if<(N > 1), void>::type pack_integral(
	OutputIterator &&sink, T v
)
{
	*sink++ = static_cast<uint8_t>(v & 0xff);
	pack_integral<N - 1>(sink, v >> 8);
}

template <typename OutputIterator>
void pack(OutputIterator &&sink, int8_t v)
{
	/*constexpr*/ int8_t const bounds[2] = {
		-(int8_t(1) << 4) - 1, int8_t(1) << 4
	};

	if ((v > bounds[0]) && (v < bounds[1]))
		*sink++ = (v & small_int_mask) + small_int_code_offset;
	else {
		*sink++ = list_code[numeric_type_rank::n_signed]
				   [list_size_rank::l3]
				   [scalar_rank::i8];
		*sink++ = v;
	}
}

template <typename OutputIterator>
void pack(OutputIterator &&sink, int16_t v)
{
	/*constexpr*/ int16_t const bounds[2] = {
		-(int16_t(1) << 7) - 1, int16_t(1) << 7
	};

	if ((v > bounds[0]) && (v < bounds[1]))
		pack(sink, static_cast<int8_t>(v));
	else {
		*sink++ = list_code[numeric_type_rank::n_signed]
				   [list_size_rank::l3]
				   [scalar_rank::i16];
		pack_integral<2>(sink, v);
	}
}

template <typename OutputIterator>
void pack(OutputIterator &&sink, int32_t v)
{
	/*constexpr*/ int32_t const bounds[2] = {
		-(int32_t(1) << 15) - 1, int32_t(1) << 15
	};

	if ((v > bounds[0]) && (v < bounds[1]))
		pack(sink, static_cast<int16_t>(v));
	else {
		*sink++ = list_code[numeric_type_rank::n_signed]
				   [list_size_rank::l3]
				   [scalar_rank::i32];
		pack_integral<4>(sink, v);
	}
}

template <typename OutputIterator>
void pack(OutputIterator &&sink, int64_t v)
{
	/*constexpr*/ int64_t const bounds[2] = {
		-(int64_t(1) << 31) - 1, int64_t(1) << 31
	};

	if ((v > bounds[0]) && (v < bounds[1]))
		pack(sink, static_cast<int32_t>(v));
	else {
		*sink++ = list_code[numeric_type_rank::n_signed]
				   [list_size_rank::l3]
				   [scalar_rank::i64];
		pack_integral<8>(sink, v);
	}
}

template <typename OutputIterator>
void pack(OutputIterator &&sink, __int128 v)
{
	/*constexpr*/ __int128_t const bounds[2] = {
		-(__int128_t(1) << 63) - 1, __int128_t(1) << 63
	};

	if ((v > bounds[0]) && (v < bounds[1]))
		pack(sink, static_cast<int64_t>(v));
	else {
		*sink++ = list_code[numeric_type_rank::n_signed]
				   [list_size_rank::l3]
				   [scalar_rank::i128];
		pack_integral<16>(sink, v);
	}
}

template <typename OutputIterator>
void pack(OutputIterator &&sink, uint8_t v)
{
	/*constexpr*/ uint8_t const bound(uint8_t(1) << 5);

	if (v < bound)
		*sink++ = (v & small_int_mask) + small_uint_code_offset;
	else {
		*sink++ = list_code[numeric_type_rank::n_unsigned]
				   [list_size_rank::l3]
				   [scalar_rank::i8];
		*sink++ = v;
	}
}

template <typename OutputIterator>
void pack(OutputIterator &&sink, uint16_t v)
{
	/*constexpr*/ uint16_t const bound(uint16_t(1) << 8);

	if (v < bound)
		pack(sink, static_cast<uint8_t>(v));
	else {
		*sink++ = list_code[numeric_type_rank::n_unsigned]
				   [list_size_rank::l3]
				   [scalar_rank::i16];
		pack_integral<2>(sink, v);
	}
}

template <typename OutputIterator>
void pack(OutputIterator &&sink, uint32_t v)
{
	/*constexpr*/ uint32_t const bound(uint32_t(1) << 16);

	if (v < bound)
		pack(sink, static_cast<uint16_t>(v));
	else {
		*sink++ = list_code[numeric_type_rank::n_unsigned]
				   [list_size_rank::l3]
				   [scalar_rank::i32];
		pack_integral<4>(sink, v);
	}
}

template <typename OutputIterator>
void pack(OutputIterator &&sink, uint64_t v)
{
	/*constexpr*/ uint64_t const bound(uint64_t(1) << 32);

	if (v < bound)
		pack(sink, static_cast<uint32_t>(v));
	else {
		*sink++ = list_code[numeric_type_rank::n_unsigned]
				   [list_size_rank::l3]
				   [scalar_rank::i64];
		pack_integral<8>(sink, v);
	}
}

template <typename OutputIterator>
void pack(OutputIterator &&sink, unsigned __int128 v)
{
	/*constexpr*/ unsigned __int128 const bound(
		static_cast<unsigned __int128>(1) << 64
	);

	if (v < bound)
		pack(sink, static_cast<uint64_t>(v));
	else {
		*sink++ = list_code[numeric_type_rank::n_unsigned]
				   [list_size_rank::l3]
				   [scalar_rank::i128];
		pack_integral<16>(sink, v);
	}
}

template <typename OutputIterator, typename Sequence>
typename std::enable_if<
	yesod::is_sequence<Sequence>::value
	& std::is_integral<typename Sequence::value_type>::value, void
>::type pack(OutputIterator &&sink, Sequence const &v)
{
	auto sz(v.size());
	constexpr auto n_rank(numeric_type_rank::from_type<
		typename Sequence::value_type
	>());
	auto l_rank(list_size_rank::from_size(sz));
	constexpr auto s_rank(
		scalar_rank::from_type<typename Sequence::value_type>()
	);

	if (l_rank == list_size_rank::l3)
		*sink++ = list_code[n_rank][l_rank][s_rank]
			  | uint8_t(sz - 1);
	else {
		*sink++ = list_code[n_rank][l_rank][s_rank];
		for (auto cnt(l_rank); cnt; --cnt) {
			*sink++ = sz & 0xff;
			sz >>= 8;
		}
	}

	for (auto xv: v)
		pack_integral<1 << s_rank>(sink, xv);
}

}}}
#endif
