/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_YESOD_BITSET_20140916T1800)
#define UCPF_YESOD_BITSET_20140916T1800

#include <yesod/bitops.hpp>

namespace ucpf { namespace yesod {

template <std::size_t N>
struct bitset {
	typedef uintptr_t word_type;
	typedef std::size_t size_type;
	constexpr static size_type bit_count = N;
	constexpr static size_type word_bits = sizeof(word_type) * 8;
	constexpr static size_type word_count = (
		bit_count % word_bits
	) ? (bit_count / word_bits + 1) : (bit_count / word_bits);
	constexpr static word_type last_word_mask = (
		bit_count % word_bits
	) ? (word_type(1) << (bit_count % word_bits)) - 1 : ~word_type(0);

	static_assert(N, "N != 0");

	void set()
	{
		for (auto p(bset); p < (bset + word_count); ++p)
			*p = ~uintptr_t(0);
	}

	void set(size_type pos)
	{
		bset[pos / word_bits] |= word_type(1) << (pos % word_bits);
	}

	void set(size_type pos, word_type mask)
	{
		auto w_pos(pos / word_bits);
		auto w_off(pos % word_bits);

		bset[w_pos] |= mask << w_off;
		if (w_off && (w_pos < (word_count - 1)))
			bset[w_pos + 1] |= mask >> (word_bits - w_off);
	}

	void reset()
	{
		for (auto p(bset); p < (bset + word_count); ++p)
			*p = 0;
	}

	void reset(size_type pos)
	{
		bset[pos / word_bits] &= ~(word_type(1) << (pos % word_bits));
	}

	void reset(size_type pos, word_type mask)
	{
		auto w_pos(pos / word_bits);
		auto w_off(pos % word_bits);

		bset[w_pos] &= mask << w_off;
		if (w_off && (w_pos < (word_count - 1)))
			bset[w_pos + 1] &= mask >> (word_bits - w_off);
	}

	bool test(size_type pos) const
	{
		return bset[pos / word_bits] & (
			word_type(1) << (pos % word_bits)
		);
	}

	word_type word_at(size_type pos) const
	{
		auto w_pos(pos / word_bits);
		auto w_off(pos % word_bits);

		if (w_off && (w_pos < (word_count - 1)))
			return (bset[w_pos] >> w_off)
			       | (bset[w_pos + 1] << (word_bits - w_off));
		else
			return bset[w_pos] >> w_off;
	}

	size_type find_first_set(size_type first) const
	{
		if (first >= bit_count)
			return bit_count;

		auto fw(first / word_bits);
		auto lw(word_count - 1);

		auto w(bset[fw]);
		if (fw == lw) {
			w &= last_word_mask;
			w >>= first % word_bits;
			if (w)
				return first + yesod::ctz(w);
			else
				return bit_count;
		}

		w >>= first % word_bits;
		if (w)
			return first + yesod::ctz(w);

		for (++fw; fw < lw; ++fw) {
			w = bset[fw];
			if (w)
				return fw * word_bits + yesod::ctz(w);
		}

		w = bset[lw];
		w &= last_word_mask;
		if (w)
			return lw * word_bits + yesod::ctz(w);

		return bit_count;
	}

	template <typename Pred>
	void for_each_set(Pred &&pred) const
	{
		for (size_type c(0); c < (word_count - 1); ++c) {
			auto pos(c * word_bits);
			auto w(bset[c]);
			while (w) {
				auto shift(yesod::ffs(w));
				pos += shift + 1;
				w >>= shift + 1;
				pred(pos - 1);
			}
		}

		auto pos((word_count - 1) * word_bits);
		auto w(bset[word_count - 1] & last_word_mask);
		while (w) {
			auto shift(yesod::ffs(w));
			pos += shift + 1;
			w >>= shift + 1;
			pred(pos - 1);
		}
	}

	word_type bset[word_count];
};

}}
#endif
