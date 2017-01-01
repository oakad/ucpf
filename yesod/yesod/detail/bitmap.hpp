/*
 * Copyright (c) 2016 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_5C0BC431BBCC799AF8B5E168C9C09CF0)
#define HPP_5C0BC431BBCC799AF8B5E168C9C09CF0

#include <yesod/detail/bitops.hpp>
#include <limits>

namespace ucpf::yesod::detail {

template <typename BitmapImpl>
struct bitmap_base {
	typedef uintptr_t word_type;
	typedef size_t size_type;
	typedef BitmapImpl bitmap_type;

	constexpr static size_type word_type_bits = std::numeric_limits<
		word_type
	>::digits;

	constexpr static uintptr_t word_type_alignment_mask
	= sizeof(word_type) - 1;

	constexpr static word_type word_type_zeros = word_type(0);
	constexpr static word_type word_type_ones = ~word_type(0);

	constexpr static auto byte_count(size_type sz)
	{
		return (sz >> 3) + (
			sz & 7 ? 1 : 0
		);
	}

	constexpr static auto word_count(size_type sz)
	{
		return (sz / word_type_bits) + (
			sz % word_type_bits ? 1 : 0
		);
	}

	static auto to_word_ptr(
		uint8_t const *ptr, size_type &byte_offset
	)
	{
		auto ptr_val(reinterpret_cast<uintptr_t>(ptr));
		byte_offset = ptr_val & word_type_alignment_mask;
		return reinterpret_cast<word_type const *>(
			ptr_val & ~word_type_alignment_mask
		);
	}

	static auto count_ones(uint8_t const *b_map, size_type count)
	{
		size_type first_word_offset, last_word_offset;
		auto first_word(to_word_ptr(b_map, first_word_offset));
		auto last_word(to_word_ptr(b_map + count, last_word_offset));
		auto first_word_mask(first_word_offset
			? bitmap_type::word_bit_mask(
				first_word_offset << 3,
				word_type_bits - (first_word_offset << 3)
			)
			: word_type_ones
		);
		auto last_word_mask(last_word_offset
			? ~bitmap_type::word_bit_mask(
				last_word_offset << 3,
				word_type_bits - (last_word_offset << 3)
			)
			: word_type_zeros
		);

		if (first_word == last_word)
			return popcount(
				*first_word & first_word_mask & last_word_mask
			);
		else {
			auto rv(popcount(*first_word & first_word_mask));
			for (
				++first_word; first_word < last_word;
				++first_word
			)
				rv += popcount(*first_word);

			return rv + popcount(*last_word & last_word_mask);
		}
	}
};

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

struct bitmap : bitmap_base<bitmap> {
	static bool get(uint8_t *b_map, size_type pos)
	{
		return b_map[pos >> 3] & (1 << (pos & 7));
	}

	static void set(uint8_t *b_map, size_type pos)
	{
		b_map[pos >> 3] |= 1 << (pos & 7);
	}

	constexpr static word_type word_bit_mask(
		size_type offset, size_type count
	)
	{
		return ((word_type(1) << count) - 1) << offset;
	}
};

#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__

struct bitmap : bitmap_base<bitmap> {
	static bool get(uint8_t *b_map, size_type pos)
	{
		return b_map[pos >> 3] & (1 << (8 - (pos & 7)));
	}

	static void set(uint8_t *b_map, size_type pos)
	{
		b_map[pos >> 3] |= 1 << (8 - (pos & 7));
	}

	constexpr static word_type word_bit_mask(
		size_type offset, size_type count
	)
	{
		return ((word_type(1) << count) - 1) << (
			word_type_bits - offset - count
		);
	}
};

#else
#error Unknown endianness!
#endif

}
#endif
