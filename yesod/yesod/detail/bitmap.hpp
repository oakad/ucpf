/*
 * Copyright (c) 2016-2017 Alex Dubov <oakad@yahoo.com>
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

	static auto count_ones(
		uint8_t const *b_map, size_type first_, size_type last_
	)
	{
		word_ref<decltype(b_map), true> first(b_map, first_);
		word_ref<decltype(b_map), false> last(b_map, last_);

		if (first.ptr == last.ptr)
			return popcount(*first.ptr & first.mask & last.mask);
		else {
			auto rv(popcount(*first.ptr & first.mask));
			for (
				++first.ptr;
				first.ptr < last.ptr;
				++first.ptr
			)
				rv += popcount(*first.ptr);

			return rv + popcount(*last.ptr & last.mask);
		}
	}

	static auto find_nth_one(
		uint8_t const *b_map, size_type pos, size_type first_,
		size_type last_
	)
	{
		word_ref<decltype(b_map), true> first(b_map, first_);
		word_ref<decltype(b_map), false> last(b_map, last_);

		if (first.ptr == last.ptr) {
			auto w(*first.ptr & first.mask & last.mask);

			if (size_type(popcount(w)) >= pos)
				return bitmap_type::find_nth_one(w, pos)
				       - first.offset;
			else
				return last_ - first_;
		} else {
			auto w(*first.ptr & first.mask);
			size_type pc(popcount(w));
			if (pc >= pos)
				return bitmap_type::find_nth_one(
					w, pos
				) - first.offset;

			auto b_pos(word_type_bits - first.offset);
			pos -= pc;

			for (++first.ptr; first.ptr < last.ptr; ++first.ptr) {
				w = *first.ptr;
				pc = popcount(w);
				if (pc >= pos)
					return bitmap_type::find_nth_one(
						w, pos
					) + b_pos;
				b_pos += word_type_bits;
				pos -= pc;
			}

			w = *last.ptr & last.mask;
			pc = popcount(w);
			if (pc >= pos)
				return bitmap_type::find_nth_one(
					w, pos
				) + b_pos;
			else
				return last_ - first_;
		}
	}

	static auto find_nth_one_r(
		uint8_t const *b_map, size_type pos, size_type first_,
		size_type last_
	)
	{
		word_ref<decltype(b_map), true> first(b_map, first_);
		word_ref<decltype(b_map), false> last(b_map, last_);

		if (first.ptr == last.ptr) {
			auto w(*first.ptr & first.mask & last.mask);

			if (size_type(popcount(w)) >= pos)
				return last.offset - bitmap_type::find_nth_one_r(
					w, pos
				);
			else
				return last_ - first_;
		} else {
			auto w(*last.ptr & last.mask);
			size_type pc(popcount(w));

			if (pc >= pos)
				return last.offset - bitmap_type::find_nth_one_r(
					w, pos
				);

			pos -= pc;
			auto b_pos(last.offset);

			for (--last.ptr; last.ptr > first.ptr; --last.ptr) {
				w = *last.ptr;
				pc = popcount(w);
				b_pos += word_type_bits;
				if (pc >= pos)
					return b_pos - bitmap_type::find_nth_one_r(
						w, pos
					);
				pos -= pc;
			}

			w = *first.ptr & first.mask;
			b_pos += word_type_bits;
			pc = popcount(w);
			if (pc >= pos)
				return b_pos - bitmap_type::find_nth_one_r(
					w, pos
				);
			else
				return first_;
		}
	}

	static void copy(
		uint8_t *dst_map, size_type dst_first_, uint8_t const *src_map,
		size_type src_first_, size_type src_last_
	)
	{
		word_ref<decltype(dst_map), true> dst_first(
			dst_map, dst_first_
		);

		word_ref<decltype(src_map), true> src_first(
			src_map, src_first_
		);
		word_ref<decltype(src_map), false> src_last(
			src_map, src_last_
		);

		if (src_first.ptr == src_last.ptr) {
			auto w(bitmap_type::unshift(
				*src_first.ptr & src_first.mask & src_last.mask,
				src_first.offset
			));
			bitmap_type::copy_bits(
				dst_first, w, src_last_ - src_first_
			);
		} else {
			auto w(bitmap_type::unshift(
				*src_first.ptr & src_first.mask,
				src_first.offset
			));
			bitmap_type::copy_bits(
				dst_first, w, word_type_bits - src_first.offset
			);
			for (
				++src_first.ptr;
				src_first.ptr < src_last.ptr;
				++src_first.ptr
			)
				bitmap_type::copy_word(
					dst_first, *src_first.ptr
				);

			w = *src_last.ptr & src_last.mask;
			bitmap_type::copy_bits(dst_first, w, src_last.offset);
		}
	}

	template <typename BitmapPtr, bool IsFirst>
	struct word_ref {
		word_ref(BitmapPtr b_map, size_type b_offset)
		{
			auto ptr_val(reinterpret_cast<uintptr_t>(b_map));
			ptr_val += b_offset >> 3;
			offset = b_offset & 7;
			offset += (ptr_val & word_type_alignment_mask) << 3;
			ptr = reinterpret_cast<decltype(ptr)>(
				ptr_val & ~word_type_alignment_mask
			);
			if (IsFirst)
				mask = offset ? bitmap_type::word_bit_mask(
					offset, word_type_bits - offset
				) : ~word_type(0);
			else
				mask = offset ? ~bitmap_type::word_bit_mask(
					offset, word_type_bits - offset
				) : word_type(0);
		}

		void increment()
		{
			++offset;
			if (offset == word_type_bits) {
				offset = 0;
				++ptr;
			}
		}

		void decrement()
		{
			if (offset)
				--offset;
			else {
				offset = word_type_bits - 1;
				--ptr;
			}
		}

		bool test() const
		{
			return *ptr & bitmap_type::word_bit_mask(offset, 1);
		}

		std::conditional_t<std::is_const<
			std::remove_pointer_t<BitmapPtr>
		>::value, word_type const *, word_type *> ptr;
		size_type offset;
		word_type mask;
	};
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

	static size_type find_nth_one(
		uint8_t const *b_map, size_type pos, size_type first,
		size_type last
	)
	{
		return bitmap_base<bitmap>::find_nth_one(
			b_map, pos, first, last
		);
	}

	static auto find_nth_one_r(
		uint8_t const *b_map, size_type pos, size_type first,
		size_type last
	)
	{
		return bitmap_base<bitmap>::find_nth_one_r(
			b_map, pos, first, last
		);
	}
private:
	friend struct bitmap_base<bitmap>;

	constexpr static word_type word_bit_mask(
		size_type offset, size_type count
	)
	{
		return ((word_type(1) << count) - 1) << offset;
	}

	constexpr static auto unshift(word_type w, size_type offset)
	{
		return w >> offset;
	}

	static size_type find_nth_one(word_type w, size_type pos)
	{
		size_type w_pos(0);
		while (true) {
			auto shift = ctz(w);
			w >>= shift + 1;
			w_pos += shift;
			--pos;
			if (!pos)
				return w_pos;
			++w_pos;
		}
	}

	static size_type find_nth_one_r(word_type w, size_type pos)
	{
		size_type w_pos(0);
		while (true) {
			auto shift = clz(w);
			w <<= shift + 1;
			w_pos += shift;
			--pos;
			if (!pos)
				return word_type_bits - w_pos - 1;
			++w_pos;
		}
	}

	template <typename WordRef>
	static void copy_word(WordRef &dst, word_type w)
	{
		if (dst.offset) {
			*dst.ptr &= word_bit_mask(0, dst.offset);
			*dst.ptr |= w << dst.offset;
			++dst.ptr;
			*dst.ptr &= word_bit_mask(
				dst.offset, word_type_bits - dst.offset
			);
			*dst.ptr |= w >> dst.offset;
		} else {
			*dst.ptr = w;
			++dst.ptr;
		}
	}

	template <typename WordRef>
	static void copy_bits(WordRef &dst, word_type w, size_type count)
	{
		if (!count)
			return;

		if (count == word_type_bits) {
			copy_word(dst, w);
			return;
		}

		auto rem(word_type_bits - dst.offset);
		if (rem < count) {
			*dst.ptr &= word_bit_mask(0, dst.offset);
			*dst.ptr |= w << dst.offset;
			++dst.ptr;
			w >>= rem;
			dst.offset = count - rem;
			*dst.ptr &= word_bit_mask(
				dst.offset, word_type_bits - dst.offset
			);
			*dst.ptr |= w;
		} else if (rem > count) {
			*dst.ptr &= ~word_bit_mask(dst.offset, count);
			*dst.ptr |= w << dst.offset;
			dst.offset += count;
		} else {
			*dst.ptr &= word_bit_mask(0, dst.offset);
			*dst.ptr |= w << dst.offset;
			++dst.ptr;
			dst.offset = 0;
		}
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

	static size_type find_nth_one(
		uint8_t const *b_map, size_type pos, size_type first,
		size_type last
	)
	{
		return bitmap_base<bitmap>::find_nth_one(
			b_map, pos, first, last
		);
	}

	static auto find_nth_one_r(
		uint8_t const *b_map, size_type pos, size_type first,
		size_type last
	)
	{
		return bitmap_base<bitmap>::find_nth_one_r(
			b_map, pos, first, last
		);
	}

private:
	friend struct bitmap_base<bitmap>;

	constexpr static word_type word_bit_mask(
		size_type offset, size_type count
	)
	{
		return ((word_type(1) << count) - 1) << (
			word_type_bits - offset - count
		);
	}

	constexpr static auto unshift(word_type w, size_type offset)
	{
		retunr w << offset;
	}

	static auto find_nth_one(word_type w, size_type pos)
	{
		size_type w_pos(0);
		while (true) {
			auto shift = clz(w);
			w <<= shift + 1;
			w_pos += shift;
			--pos;
			if (!pos)
				return w_pos;
			++w_pos;
		}
	}

	static size_type find_nth_one_r(word_type w, size_type pos)
	{
		size_type w_pos(0);
		while (true) {
			auto shift = ctz(w);
			w >>= shift + 1;
			w_pos += shift;
			--pos;
			if (!pos)
				return word_type_bits - w_pos - 1;
			++w_pos;
		}
	}

	template <typename WordRef>
	static void copy_word(WordRef &dst, word_type w)
	{
		if (dst.offset) {
			*dst.ptr &= word_bit_mask(0, dst.offset);
			*dst.ptr |= w >> dst.offset;
			++dst.ptr;
			*dst.ptr &= word_bit_mask(
				dst.offset, word_type_bits - dst.offset
			);
			*dst.ptr |= w << dst.offset;
		} else {
			*dst.ptr = w;
			++dst.ptr;
		}
	}

	template <typename WordRef>
	static void copy_bits(WordRef &dst, word_type w, size_type count)
	{
		if (!count)
			return;

		if (count == word_type_bits) {
			copy_word(dst, w);
			return;
		}

		auto rem(word_type_bits - dst.offset);
		if (rem < count) {
			*dst.ptr &= word_bit_mask(0, dst.offset);
			*dst.ptr |= w >> dst.offset;
			++dst.ptr;
			w <<= rem;
			dst.offset = count - rem;
			*dst.ptr &= word_bit_mask(
				dst.offset, word_type_bits - dst.offset
			);
			*dst.ptr |= w;
		} else if (rem > count) {
			*dst.ptr &= ~word_bit_mask(dst.offset, count);
			*dst.ptr |= w >> dst.offset;
			dst.offset += count;
		} else {
			*dst.ptr &= word_bit_mask(0, dst.offset);
			*dst.ptr |= w >> dst.offset;
			++dst.ptr;
			dst.offset = 0;
		}
	}
};

#else
#error Unknown endianness!
#endif

}
#endif
