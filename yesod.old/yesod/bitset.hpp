/*
 * Copyright (c) 2014-2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_446C8F1134E9795DFF6B38E2A03EF9DE)
#define HPP_446C8F1134E9795DFF6B38E2A03EF9DE

#include <limits>
#include <yesod/bitops.hpp>

namespace ucpf { namespace yesod {
namespace detail {

template <std::size_t N, bool Long = false>
struct intrusive_bitset_impl {
	typedef uintptr_t word_type;
	typedef std::size_t size_type;
	constexpr static size_type word_bits = std::numeric_limits<
		word_type
	>::digits;

	constexpr static size_type bit_count = N;
	constexpr static size_type word_count = 1;
	constexpr static size_type full_word_count = (
		bit_count < word_bits ? 0 : 1
	);
	constexpr static word_type word_mask = (
		bit_count < word_bits
	) ? ((word_type(1) << bit_count) - 1) : ~word_type(0);

	static_assert(N > 0, "N > 0");

	static void set(word_type *bset)
	{
		*bset = word_mask;		
	}	

	static void set(word_type *bset, size_type pos)
	{
		*bset |= word_type(1) << pos;
	}

	static void set(word_type *bset, size_type pos, size_type length)
	{
		*bset |= ((word_type(1) << length) - 1) << pos;
	}

	static void reset(word_type *bset)
	{
		*bset = 0;
	}

	static void reset(word_type *bset, size_type pos)
	{
		*bset &= ~(word_type(1) << pos);
	}

	static void reset(word_type *bset, size_type pos, size_type length)
	{
		*bset &= ~(((word_type(1) << length) - 1) << pos);
	}

	static bool test(word_type const *bset, size_type pos)
	{
		return *bset & (word_type(1) << pos);
	}

	static size_type count(word_type const *bset)
	{
		return yesod::popcount(*bset & word_mask);
	}

	static size_type find_first_one(word_type const *bset, size_type first)
	{
		if (first >= bit_count)
			return bit_count;

		auto w((*bset & word_mask) >> first);
		return w ? (yesod::ctz(w) + first) : bit_count;
	}

	static size_type find_first_zero(word_type const *bset, size_type first)
	{
		if (first >= bit_count)
			return bit_count;

		auto w(~((*bset & word_mask) >> first));
		return w ? (yesod::ctz(w) + first) : bit_count;
	}

	static size_type find_zero_range_first(
		word_type const *bset, size_type first, size_type length
	)
	{
		if ((first + length) > bit_count)
			return bit_count;

		auto mask(~word_mask | ((size_type(1) << first) - 1));
		auto w(~(*bset | mask));

		size_type bl(yesod::popcount(w));
		if (bl == (bit_count - first))
			return first;
		else if (bl < length)
			return bit_count;

		bl = length;

		while (bl > 1) {
			auto shift(bl >> 1);
			w &= w >> shift;
			bl -= shift;
		}

		return w ? yesod::ctz(w) : bit_count;
	}

	static size_type find_zero_range_best(
		word_type const *bset, size_type first, size_type length
	)
	{
		size_type last_pos(bit_count);
		size_type last_over(bit_count);
		size_type pos(bit_count);

		while (true) {
			pos = find_zero_range_first(bset, first, length);
			first = find_first_one(bset, pos);

			if (first == bit_count)
				break;

			auto rl(first - pos - length);
			if (!rl)
				return pos;
			else if (rl < last_over) {
				last_over = rl;
				last_pos = pos;
			}
		}
		return pos;
	}

	template <typename Pred>
	static bool for_each_one(
		word_type const *bset, size_type first, Pred &&pred
	)
	{
		auto mask(((word_type(1) << first) - 1) ^ word_mask);
		auto w(*bset & mask);

		while (w) {
			auto pos(yesod::ctz(w));
			if (pred(pos))
				return true;

			w ^= word_type(1) << pos;
		}
		return false;
	}

	template <typename Pred>
	static bool for_each_zero(
		word_type const *bset, size_type first, Pred &&pred
	)
	{
		auto w(~*bset);
		return for_each_zero(&w, first, std::forward<Pred>(pred));
	}

};

template <std::size_t N>
struct intrusive_bitset_impl<N, true> {
	typedef uintptr_t word_type;
	typedef std::size_t size_type;

	constexpr static size_type word_bits = std::numeric_limits<
		word_type
	>::digits;
	constexpr static size_type bit_count = N;
	constexpr static size_type full_word_count = bit_count / word_bits;
	constexpr static size_type tail_word_bits = (
		bit_count - full_word_count * word_bits
	);
	constexpr static size_type word_count = (
		tail_word_bits ? (full_word_count + 1) : full_word_count
	);
	constexpr static word_type last_word_mask = (
		tail_word_bits ? (
			(word_type(1) << tail_word_bits) - 1
		) : ~word_type(0)
	);

	static void set(word_type *bset)
	{
		for (auto p(bset); p < (bset + full_word_count); ++p)
			*p = ~word_type(0);

		if (word_count > full_word_count)
			bset[full_word_count] |= last_word_mask;
	}

	static void set(word_type *bset, size_type pos)
	{
		bset[pos / word_bits] |= word_type(1) << (pos % word_bits);
	}

	static void set(word_type *bset, size_type pos, size_type length)
	{
		auto w_pos(pos / word_bits);

		{
			auto w_off(pos % word_bits);
			if (w_off) {
				auto x_len(word_bits - w_off);

				if (w_pos == last_word_mask) {
					if (w_off < tail_word_bits)
						x_len = tail_word_bits - w_off;
					else
						return;
						
				}
				if (x_len > length)
					x_len = length;

				bset[w_pos] |= ((
					word_type(1) << x_len
				) - 1) << w_off;

				length -= x_len;
				if (!length)
					return;

				++w_pos;
			}
		}

		while (w_pos < full_word_count) {
			if (length >= word_bits) {
				bset[w_pos] = ~word_type(0);
				length -= word_bits;
			} else {
				bset[w_pos] |= (word_type(1) << length) - 1;
				return;
			}

			++w_pos;
		}

		if (!length)
			return;

		if (word_count == full_word_count)
			return;

		if (length > tail_word_bits)
			length = tail_word_bits;

		bset[w_pos] |= (word_type(1) << length) - 1;
	}

	static void reset(word_type *bset)
	{
		for (auto p(bset); p < (bset + full_word_count); ++p)
			*p = 0;

		if (word_count > full_word_count)
			bset[full_word_count] &= !last_word_mask;
	}

	static void reset(word_type *bset, size_type pos)
	{
		bset[pos / word_bits] &= ~(word_type(1) << (pos % word_bits));
	}

	static void reset(word_type *bset, size_type pos, size_type length)
	{
		auto w_pos(pos / word_bits);

		{
			auto w_off(pos % word_bits);
			if (w_off) {
				auto x_len(word_bits - w_off);

				if (w_pos == last_word_mask) {
					if (w_off < tail_word_bits)
						x_len = tail_word_bits - w_off;
					else
						return;
						
				}
				if (x_len > length)
					x_len = length;

				bset[w_pos] &= ~(((
					word_type(1) << x_len
				) - 1) << w_off);

				length -= x_len;
				if (!length)
					return;

				++w_pos;
			}
		}

		while (w_pos < full_word_count) {
			if (length >= word_bits) {
				bset[w_pos] = word_type(0);
				length -= word_bits;
			} else {
				bset[w_pos] &= ~((word_type(1) << length) - 1);
				return;
			}

			++w_pos;
		}

		if (!length)
			return;

		if (word_count == full_word_count)
			return;

		if (length > tail_word_bits)
			length = tail_word_bits;

		bset[w_pos] &= ~((word_type(1) << length) - 1);
	}

	static bool test(word_type const *bset, size_type pos)
	{
		return bset[pos / word_bits] & (
			word_type(1) << (pos % word_bits)
		);
	}

	static size_type count(word_type const *bset)
	{
		size_type rv(0);
		for (auto p(bset); p < (bset + full_word_count); ++p)
			rv += yesod::popcount(*p);

		if (word_count > full_word_count)
			rv += yesod::popcount(
				bset[full_word_count] & last_word_mask
			);

		return rv;
	}

	static size_type find_first_one(word_type const *bset, size_type first)
	{
		if (first >= bit_count)
			return bit_count;

		auto w_pos(first / word_bits);

		{
			auto w_off(first % word_bits);
			if (w_off) {
				auto w(
					bset[w_pos]
					& ~((word_type(1) << w_off) - 1)
				);

				if (w_pos == full_word_count)
					w &= last_word_mask;

				if (w)
					return yesod::ctz(w)
					       + w_pos * word_bits;
				else if (w_pos == full_word_count)
					return bit_count;

				++w_pos;
			}
		}

		while (w_pos < full_word_count) {
			auto w(bset[w_pos]);
			if (w)
				return yesod::ctz(w) + w_pos * word_bits;

			++w_pos;
		}

		if (word_count == full_word_count)
			return bit_count;

		auto w(bset[full_word_count] & last_word_mask);

		return w ? yesod::ctz(w) + full_word_count * word_bits
			 : bit_count;
	}

	static size_type find_first_zero(word_type const *bset, size_type first)
	{
		if (first >= bit_count)
			return bit_count;

		auto w_pos(first / word_bits);

		{
			auto w_off(first % word_bits);
			if (w_off) {
				auto w(~(
					bset[w_pos]
					| ((word_type(1) << w_off) - 1)
				));
				if (w_pos == full_word_count)
					w &= last_word_mask;

				if (w)
					return yesod::ctz(w) + w_pos * word_bits;
				else if (w_pos == full_word_count)
					return bit_count;

				++w_pos;
			}
		}

		while (w_pos < full_word_count) {
			auto w(~bset[w_pos]);
			if (w)
				return yesod::ctz(w) + w_pos * word_bits;

			++w_pos;
		}

		if (word_count == full_word_count)
			return bit_count;

		auto w(~bset[full_word_count] & last_word_mask);

		return w ? yesod::ctz(w) + full_word_count * word_bits
			 : bit_count;
	}

	static size_type find_zero_range_first(
		word_type const *bset, size_type first, size_type length
	)
	{
		if (length < (word_bits - 1))
			return find_zero_range_first_short(bset, first, length);
		else
			return find_zero_range_first_long(bset, first, length);
	}

	static size_type find_zero_range_best(
		word_type const *bset, size_type first, size_type length
	)
	{
		size_type last_pos(bit_count);
		size_type last_over(bit_count);
		size_type pos(bit_count);

		while (true) {
			pos = find_zero_range_first(bset, first, length);
			first = find_first_one(bset, pos);

			if (first == bit_count)
				break;

			auto rl(first - pos - length);
			if (!rl)
				return pos;
			else if (rl < last_over) {
				last_over = rl;
				last_pos = pos;
			}
		}
		return pos;
	}

	template <typename Pred>
	static bool for_each_one(
		word_type const *bset, size_type first, Pred &&pred
	)
	{
		if (first >= bit_count)
			return false;

		auto w_pos(first / word_bits);

		{
			auto w_off(first % word_bits);
			auto w(bset[w_pos] & ~((word_type(1) << w_off) - 1));
			first -= w_off;
			if (w_pos == full_word_count)
				w &= last_word_mask;

			while (w) {
				auto pos(yesod::ctz(w));
				if (pred(first + pos))
					return true;

				w ^= word_type(1) << pos;
			}
			first += word_bits;
		}

		++w_pos;
		while (w_pos < full_word_count) {
			auto w(bset[w_pos]);
			while (w) {
				auto pos(yesod::ctz(w));
				if (pred(first + pos))
					return true;

				w ^= word_type(1) << pos;
			}

			first += word_bits;
			++w_pos;
		}

		if (word_count == full_word_count)
			return false;

		auto w(bset[full_word_count] & last_word_mask);
		while (w) {
			auto pos(yesod::ctz(w));
			if (pred(first + pos))
				return true;

			w ^= word_type(1) << pos;
		}

		return false;
	}

	template <typename Pred>
	static bool for_each_zero(
		word_type const *bset, size_type first, Pred &&pred
	)
	{
		if (first >= bit_count)
			return false;

		auto w_pos(first / word_bits);

		{
			auto w_off(first % word_bits);
			auto w(~bset[w_pos] & ~((word_type(1) << w_off) - 1));
			first -= w_off;
			if (w_pos == full_word_count)
				w &= last_word_mask;

			while (w) {
				auto pos(yesod::ctz(w));
				if (pred(first + pos))
					return true;

				w ^= word_type(1) << pos;
			}
			first += word_bits;
		}

		++w_pos;
		while (w_pos < full_word_count) {
			auto w(~bset[w_pos]);
			while (w) {
				auto pos(yesod::ctz(w));
				if (pred(first + pos))
					return true;

				w ^= word_type(1) << pos;
			}

			first += word_bits;
			++w_pos;
		}

		if (word_count == full_word_count)
			return false;

		auto w(~bset[full_word_count] & last_word_mask);

		while (w) {
			auto pos(yesod::ctz(w));
			if (pred(first + pos))
				return true;

			w ^= word_type(1) << pos;
		}

		return false;
	}

private:
	static size_type find_zero_range_first_short(
		word_type const *bset, size_type first, size_type length
	)
	{
		typedef intrusive_bitset_impl<
			word_bits, false
		> wb_type;

		if ((first + length) > bit_count)
			return bit_count;

		size_type w_pos(first / word_bits);
		size_type r_len(0);
		size_type r_pos(w_pos * word_bits);

		{
			size_type w_off(first % word_bits);
			if (w_off) {
				auto w(bset[w_pos]);
				if (w_pos == full_word_count)
					w |= ~last_word_mask;

				auto x_pos(wb_type::find_zero_range_first(
					&w, w_off, length
				));

				if (x_pos < word_bits)
					return r_pos + x_pos;

				r_len = yesod::clz(w);
				r_pos += word_bits - r_len;
				++w_pos;
			}
		}

		while (w_pos < full_word_count) {
			auto w(bset[w_pos]);
			if (w) {
				r_len += yesod::ctz(w);
				if (r_len >= length)
					return r_pos;
				else
					r_pos = w_pos * word_bits;

				auto x_pos(wb_type::find_zero_range_first(
					&w, 0, length
				));

				if (x_pos < word_bits)
					return r_pos + x_pos;

				r_len = yesod::clz(w);
				r_pos += word_bits - r_len;
			} else
				return r_pos;

			++w_pos;
		}

		if (r_len >= length)
			return r_pos;

		if (full_word_count == word_count)
			return bit_count;

		auto w(bset[w_pos]);
		w |= ~last_word_mask;

		r_len += yesod::ctz(w);
		if (r_len >= length)
			return r_pos;

		r_pos = wb_type::find_zero_range_first(&w, 0, length);
		return r_pos < word_bits ? (full_word_count * word_bits + r_pos)
					 : bit_count;
	}

	static size_type find_zero_range_first_long(
		word_type const *bset, size_type first, size_type length
	)
	{
		if ((first + length) > bit_count)
			return bit_count;

		size_type w_pos(first / word_bits);
		size_type r_len(0);
		size_type r_pos(w_pos * word_bits);

		{
			size_type w_off(first % word_bits);
			if (w_off) {
				if (w_pos == full_word_count)
					return bit_count;

				auto w(bset[w_pos]);

				r_len = yesod::clz(w);
				r_pos += word_bits - r_len;
				++w_pos;
			}
		}

		while (w_pos < full_word_count) {
			auto w(bset[w_pos]);
			if (w) {
				r_len += yesod::ctz(w);
				if (r_len >= length)
					return r_pos;

				r_len = yesod::clz(w);
				r_pos = w_pos * word_bits + word_bits - r_len;
			} else {
				r_len += word_bits;
				if (r_len >= length)
					return r_pos;
			}
			++w_pos;
		}

		if (r_len >= length)
			return r_pos;

		if (full_word_count == word_count)
			return bit_count;

		auto w(bset[w_pos]);
		w &= last_word_mask;

		r_len += w ? yesod::ctz(w) : tail_word_bits;

		return r_len >= length ? r_pos : bit_count;
	}
};

}

template <std::size_t N>
using intrusive_bitset = detail::intrusive_bitset_impl<
	N, (N > detail::intrusive_bitset_impl<N, false>::word_bits)
>;

template <std::size_t N>
struct bitset {
	typedef intrusive_bitset<N> impl_type;
	typedef typename impl_type::size_type size_type;

	void set()
	{
		impl_type::set(bset);
	}

	void set(size_type pos)
	{
		impl_type::set(bset, pos);
	}

	void set(size_type pos, size_type length)
	{
		impl_type::set(bset, pos, length);
	}

	void reset()
	{
		impl_type::reset(bset);
	}

	void reset(size_type pos)
	{
		impl_type::reset(bset, pos);
	}

	void reset(size_type pos, size_type length)
	{
		impl_type::reset(bset, pos, length);
	}

	auto test(size_type pos) const
	{
		return impl_type::test(bset, pos);
	}

	auto count() const
	{
		return impl_type::count(bset);
	}

	auto size() const
	{
		return impl_type::bit_count;
	}

	auto find_first_one(size_type first) const
	{
		return impl_type::find_first_one(bset, first);
	}

	auto find_first_zero(size_type first) const
	{
		return impl_type::find_first_zero(bset, first);
	}

	auto find_zero_range_first(size_type first, size_type length) const
	{
		return impl_type::find_zero_range_first(bset, first, length);
	}

	auto find_zero_range_best(size_type first, size_type length) const
	{
		return impl_type::find_zero_range_best(bset, first, length);
	}

	template <typename Pred>
	auto for_each_one(size_type first, Pred &&pred) const
	{
		return impl_type::for_each_one(
			bset, first, std::forward<Pred>(pred)
		);
	}

	template <typename Pred>
	auto for_each_zero(size_type first, Pred &&pred) const
	{
		return impl_type::for_each_zero(
			bset, first, std::forward<Pred>(pred)
		);
	}

private:
	typename impl_type::word_type bset[impl_type::word_count];
};

}}

#endif
