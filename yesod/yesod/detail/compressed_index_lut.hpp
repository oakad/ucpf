/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_YESOD_DETAIL_COMPRESSED_INDEX_LUT_JAN_15_2014_1540)
#define UCPF_YESOD_DETAIL_COMPRESSED_INDEX_LUT_JAN_15_2014_1540

namespace ucpf { namespace yesod { namespace detail {

template <typename T, size_t SrcOrder, size_t DstOrder>
struct compressed_index_lut {
	typedef uintptr_t word_type;
	constexpr static size_t bit_size = DstOrder << SrcOrder;
	constexpr static size_t bits_per_word = sizeof(word_type) * 8;
	constexpr static size_t word_size
	= (bit_size / bits_per_word) + ((bit_size % bits_per_word) ? 1 : 0);
	constexpr static T src_value_mask = (T(1) << SrcOrder) - 1;
	constexpr static word_type dst_value_mask
	= (word_type(1) << DstOrder) - 1;

	static_assert(DstOrder < bits_per_word, "");
	static_assert(DstOrder <= (sizeof(T) * 8), "");
	static_assert(SrcOrder <= (sizeof(T) * 8), "");

	compressed_index_lut() = default;

	compressed_index_lut(std::initializer_list<std::pair<T, T>> il)
	: bits{}
	{
		for (auto p: il)
			set(p.first, p.second);
	}

	T operator[](T src) const
	{
		src &= src_value_mask;
		auto bw(src / bits_per_word);
		auto bb(src % bits_per_word);
		auto ew((src + DstOrder) / bits_per_word);

		auto dst((bits[bw] >> bb) & dst_value_mask);

		if (bw == ew)
			return dst;
		else return dst | (
			(bits[ew] << (bits_per_word - bb)) & dst_value_mask
		);
	}

	void set(T src, T dst)
	{
		src &= src_value_mask;
		auto bw(src / bits_per_word);
		auto bb(src % bits_per_word);
		auto ew((src + DstOrder) / bits_per_word);

		bits[bw] &= ~(dst_value_mask << bb);
		bits[bw] |= (word_type(dst) & dst_value_mask) << bb;

		if (bw != ew) {
			bits[ew] &= ~(
				dst_value_mask
				>> (DstOrder + bb - bits_per_word)
			);
			bits[ew] |= (
				(word_type(dst) & dst_value_mask)
				>> (DstOrder + bb - bits_per_word)
			);
		}
	}

	word_type bits[word_size];
};

}}}
#endif
