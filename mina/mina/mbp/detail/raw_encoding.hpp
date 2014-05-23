/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_MINA_MBP_DETAIL_RAW_ENCODING_20131118T1700)
#define UCPF_MINA_DETAIL_RAW_ENCODING_20131118T1700

#include <array>
#include <cstdint>
#include <yesod/bitops.hpp>

namespace ucpf { namespace mina { namespace mbp { namespace detail {

constexpr uint8_t const small_int_mask         = 0x1f;
constexpr uint8_t const small_int_code_offset  = 0x10;
constexpr uint8_t const small_uint_code_offset = 0x30;

struct scalar_rank {
	enum {
		ie   = -1,
		i4   = 0,
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

constexpr uint8_t tuple_start_code = 0x50;
constexpr uint8_t byte_skip_code = 0x60;
constexpr uint8_t tuple_end_code = 0x70;

}}}}

#endif
