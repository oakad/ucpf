/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_YESOD_DETAIL_COMPRESSED_INDEX_MAP_JAN_15_2014_1540)
#define UCPF_YESOD_DETAIL_COMPRESSED_INDEX_MAP_JAN_15_2014_1540

namespace ucpf { namespace yesod { namespace detail {

template <typename T, size_t SrcOrder, size_t DstOrder>
struct compressed_index_map_a {
	static_assert(
		sizeof(T) <= sizeof(uint64_t), "sizeof(T) <= sizeof(uint64_t)"
	);

	typedef typename std::conditional<
		(sizeof(T) > sizeof(unsigned long)), uint64_t, unsigned long
	>::type word_type;

	constexpr static size_t bit_size = DstOrder << SrcOrder;
	constexpr static size_t bits_per_word = sizeof(word_type) * 8;
	constexpr static size_t word_size
	= (bit_size / bits_per_word) + ((bit_size % bits_per_word) ? 1 : 0);
	constexpr static T src_value_mask = (T(1) << SrcOrder) - 1;
	constexpr static word_type dst_value_mask
	= (word_type(1) << DstOrder) - 1;

	static_assert(DstOrder < bits_per_word, "DstOrder < bits_per_word");
	static_assert(
		DstOrder <= (sizeof(T) * 8), "DstOrder <= (sizeof(T) * 8)"
	);
	static_assert(
		SrcOrder <= (sizeof(T) * 8), "SrcOrder <= (sizeof(T) * 8)"
	);

	compressed_index_map_a() = default;

	compressed_index_map_a(std::initializer_list<std::pair<T, T>> il)
	: bits()
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

private:
	word_type bits[word_size];
};

template <typename T, T Default, size_t SrcOrder, size_t DstOrder>
struct compressed_index_map_b {
	static_assert(
		sizeof(T) <= sizeof(uint64_t), "sizeof(T) <= sizeof(uint64_t)"
	);

	typedef typename std::conditional<
		(sizeof(T) > sizeof(unsigned long)), uint64_t, unsigned long
	>::type word_type;

	constexpr static size_t flag_bit_size = size_t(1) << SrcOrder;
	constexpr static size_t bit_size
	= (size_t(1) << DstOrder) * DstOrder + flag_bit_size;
	constexpr static size_t bits_per_word = sizeof(word_type) * 8;
	constexpr static size_t word_size
	= (bit_size / bits_per_word) + ((bit_size % bits_per_word) ? 1 : 0);
	constexpr static T src_value_mask = (T(1) << SrcOrder) - 1;
	constexpr static word_type dst_value_mask
	= (word_type(1) << DstOrder) - 1;

	static_assert(DstOrder < SrcOrder, "DstOrder < SrcOrder");
	static_assert(DstOrder < bits_per_word, "DstOrder < bits_per_word");
	static_assert(
		DstOrder <= (sizeof(T) * 8), "DstOrder <= (sizeof(T) * 8)"
	);

	compressed_index_map_b()
	: bits{}
	{}

	compressed_index_map_b(std::initializer_list<std::pair<T, T>> il)
	: bits{}
	{
		for (auto p: il)
			set(p.first, p.second);
	}

	T operator[](T src) const
	{
		src &= src_value_mask;
		auto iw(src / bits_per_word);
		auto ib(src % bits_per_word);

		if (!(bits[iw] & (word_type(1) << ib)))
			return Default;

		auto pos((count_below(src) * DstOrder) + flag_bit_size);
		return get_at(pos);
	}

	T set(T src, T dst)
	{
		src &= src_value_mask;
		auto iw(src / bits_per_word);
		auto ib(src % bits_per_word);

		auto pos(
			(count_below(src) * DstOrder) + flag_bit_size
		);
		word_type v(dst &= dst_value_mask);

		if (bits[iw] & (word_type(1) << ib))
			set_at(pos, v);
		else {
			if (count_below(flag_bit_size) >= dst_value_mask)
				return Default;

			insert_at(pos, v);
			bits[iw] |= word_type(1) << ib;
		}

		return v;
	}

	T erase(T src)
	{
		src &= src_value_mask;
		auto iw(src / bits_per_word);
		auto ib(src % bits_per_word);

		if (!(bits[iw] & (word_type(1) << ib)))
			return Default;

		auto pos((count_below(src) * DstOrder) + flag_bit_size);
		auto v(get_at(pos));
		erase_at(pos);
		bits[iw] &= ~(word_type(1) << ib);
		return v;
	}

private:
	T get_at(size_t pos) const
	{
		auto bw(pos / bits_per_word);
		auto bb(pos % bits_per_word);
		auto ew((pos + DstOrder) / bits_per_word);

		auto dst((bits[bw] >> bb) & dst_value_mask);

		if (bw == ew)
			return dst;
		else return dst | (
			(bits[ew] << (bits_per_word - bb)) & dst_value_mask
		);
	}

	void set_at(size_t pos, word_type dst)
	{
		auto bw(pos / bits_per_word);
		auto bb(pos % bits_per_word);
		auto ew((pos + DstOrder) / bits_per_word);

		bits[bw] &= ~(dst_value_mask << bb);
		bits[bw] |= dst << bb;

		if (bw != ew) {
			bits[ew] &= ~(
				dst_value_mask
				>> (DstOrder + bb - bits_per_word)
			);
			bits[ew] |= (
				dst >> (DstOrder + bb - bits_per_word)
			);
		}
	}

	void insert_at(size_t pos, word_type dst)
	{
		auto bw(pos / bits_per_word);
		auto bb(pos % bits_per_word);
		auto ew((pos + DstOrder) / bits_per_word);

		auto wx(bits[bw] & ((word_type(1) << bb) - 1));
		auto wy(bits[bw] & ~((word_type(1) << bb) - 1));

		if (bw == ew)
			bits[bw] = wx | (dst << bb) | (wy << DstOrder);
		else {
			bits[bw] = wx | (dst << bb);
			wx = bits[ew];
			bits[ew] <<= DstOrder;
			bits[ew] |= (wy >> bb) << (
				DstOrder + bb - bits_per_word
			);
			bits[ew] |= dst >> (DstOrder + bb - bits_per_word);

			wy = wx;
			bw = ew;
		}

		for (auto wp(bw + 1); wp < word_size; ++wp) {
			wx = bits[wp];
			bits[wp] <<= DstOrder;
			bits[wp] |= wy >> (bits_per_word - DstOrder);
			wy = wx;
		}
	}

	void erase_at(size_t pos)
	{
		auto bw(pos / bits_per_word);
		auto bb(pos % bits_per_word);
		auto ew((pos + DstOrder) / bits_per_word);

		auto wx(bits[bw] & ((word_type(1) << bb) - 1));
		if (bw == ew) {
			auto wy(bits[bw] & ~((
				word_type(1) << (bb + DstOrder) - 1
			)));

			bits[bw] = wx | (wy >> DstOrder);
		} else {
			bits[bw] = wx | bits[ew] << (
				2 * bits_per_word - DstOrder - bb
			);
			bits[ew] >>= DstOrder;
			bw = ew;
		}

		for (auto wp(bw + 1); wp < word_size; ++wp) {
			bits[bw] |= bits[wp] << (
				bits_per_word - DstOrder
			);
			bits[wp] >>= DstOrder;
			++bw;
		}
	}

	static size_t popcount(word_type x)
	{
		if (sizeof(word_type) > sizeof(unsigned long))
			return __builtin_popcountll(x);
		else
			return __builtin_popcountl(x);
	}

	size_t count_below(size_t pos) const
	{
		auto iw(pos / bits_per_word);
		auto ib(pos % bits_per_word);

		size_t cnt(0);
		for (size_t wp(0); wp < iw; ++wp)
			cnt += popcount(bits[wp]);

		cnt += popcount(bits[iw] & ((word_type(1) << ib) - 1));
		return cnt;
	}

	word_type bits[word_size];
};

}}}
#endif
