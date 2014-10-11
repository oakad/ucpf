/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_YESOD_BITSET_20140916T1800)
#define UCPF_YESOD_BITSET_20140916T1800

#include <limits>
#include <yesod/bitops.hpp>

namespace ucpf { namespace yesod {

template <std::size_t N>
struct bitset {
	typedef uintptr_t word_type;
	typedef std::size_t size_type;
	constexpr static size_type bit_count = N;
	constexpr static size_type word_bits = std::numeric_limits<
		word_type
	>::digits;
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

	void reset()
	{
		for (auto p(bset); p < (bset + word_count); ++p)
			*p = 0;
	}

	void reset(size_type pos)
	{
		bset[pos / word_bits] &= ~(word_type(1) << (pos % word_bits));
	}

	bool test(size_type pos) const
	{
		return bset[pos / word_bits] & (
			word_type(1) << (pos % word_bits)
		);
	}

	size_type find_first_one(size_type first) const
	{
		if (first >= bit_count)
			return bit_count;

		auto w_pos(first / word_bits);
		auto w_off(first % word_bits);

		auto w(bset[w_pos] & ~((word_type(1) << w_off) - 1));
		if (w)
			return yesod::ffs(w) - 1 + w_pos * word_bits;

		for(++w_pos; w_pos < word_count; ++w_pos) {
			w = bset[w_pos];
			if (w)
				return yesod::ffs(w) - 1 + w_pos * word_bits;
		}

		return bit_count;
	}

	size_type find_first_zero(size_type first) const
	{
		if (first >= bit_count)
			return bit_count;

		auto w_pos(first / word_bits);
		auto w_off(first % word_bits);

		auto w(~bset[w_pos] & ~((word_type(1) << w_off) - 1));
		if (w)
			return yesod::ffs(w) - 1 + w_pos * word_bits;

		for(++w_pos; w_pos < word_count; ++w_pos) {
			w = ~bset[w_pos];
			if (w)
				return yesod::ffs(w) - 1 + w_pos * word_bits;
		}

		return bit_count;
	}

	template <typename Pred>
	bool for_each_one(size_type first, Pred &&pred) const
	{
		auto w_pos(first / word_bits);
		auto w_off(first % word_bits);
		auto pos(w_pos * word_bits);
		auto w(bset[w_pos] & ~((word_type(1) << w_off) - 1));

		while (w) {
			auto shift(yesod::ffs(w) - 1);
			w ^= word_type(1) << shift;
			if (pred(pos + shift))
				return true;
		}

		for (++w_pos; w_pos < word_count; ++w_pos) {
			auto w(bset[w_pos]);
			pos = w_pos * word_bits;
			while (w) {
				auto shift(yesod::ffs(w) - 1);
				w ^= word_type(1) << shift;
				if (pred(pos + shift))
					return true;
			}
		}
		return false;
	}

	template <typename Pred>
	bool for_each_zero(size_type first, Pred &&pred) const
	{
		auto w_pos(first / word_bits);
		auto w_off(first % word_bits);
		auto pos(w_pos * word_bits);
		auto w(~bset[w_pos] & ~((word_type(1) << w_off) - 1));

		while (w) {
			auto shift(yesod::ffs(w) - 1);
			w ^= word_type(1) << shift;
			if (pred(pos + shift))
				return true;
		}

		for (++w_pos; w_pos < word_count; ++w_pos) {
			auto w(~bset[w_pos]);
			pos = w_pos * word_bits;
			while (w) {
				auto shift(yesod::ffs(w) - 1);
				w ^= word_type(1) << shift;
				if (pred(pos + shift))
					return true;
			}
		}
		return false;
	}

	word_type bset[word_count];
};

}}
#endif
