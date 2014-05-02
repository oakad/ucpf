/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_YESOD_DYNAMIC_BITSET_20140417T2300)
#define UCPF_YESOD_DYNAMIC_BITSET_20140417T2300

#include <limits>
#include <algorithm>
#include <yesod/allocator/array_helper.hpp>
#include <yesod/allocator/policy/power2.hpp>

namespace ucpf { namespace yesod {

template <
	typename Alloc = std::allocator<void>,
	typename AllocPolicy = allocator::policy::power2
> struct dynamic_bitset {
	typedef uintptr_t word_type;
	typedef allocator::array_helper<
		word_type, Alloc
	> allocator_helper_type;
	typedef typename allocator_helper_type::allocator_type allocator_type;
	typedef typename allocator_helper_type::size_type size_type;
	constexpr static size_type npos = ~size_type(0);

	static std::pair<word_type, word_type> split(
		word_type in, unsigned int pos, bool hl, bool lh
	) {
		auto rv(std::make_pair(
			in & ((base_bit >> pos) - 1),
			in & ~((base_bit >> pos) - 1)
		));
		if (hl)
			rv.second >>= (word_bits - pos);
		if (lh)
			rv.first <<= pos;

		return rv;
	}

	dynamic_bitset(Alloc const &a = Alloc())
	: bset(word_type(1), allocator_type(a))
	{}

	~dynamic_bitset()
	{
		if (!(std::get<0>(bset) & 1)) {
			auto ptr(reinterpret_cast<word_type *>(
				std::get<0>(bset)
			));
			auto sz((ptr[0] >> 1) + 1);
			allocator_helper_type::destroy(
				std::get<1>(bset), ptr, sz, true
			);
		}
	}

	allocator_type const &get_allocator() const
	{
		return std::get<1>(bset);
	}

	size_type size() const
	{
		auto s(get());
		return std::get<1>(s);
	}

	std::ostream &dump(std::ostream &os) const
	{
		constexpr static char const *z_str = "000000";
		constexpr static char const *o_str = "111111";

		auto s(get());
		size_type last_run(1);
		bool l_bit(std::get<0>(s)[0] & base_bit);

		for (size_type n(1); n < std::get<1>(s); ++n) {
			auto w_off(n / word_bits);
			auto b_off(n % word_bits);
			bool c_bit(std::get<0>(s)[w_off] & (base_bit >> b_off));

			if (l_bit == c_bit)
				++last_run;
			else {
				if (last_run < 6)
					os.write(
						l_bit ? o_str : z_str, last_run
					);
				else {
					os << '<' << (l_bit ? '1' : '0') << ':';
					os << last_run << '>';
				}

				last_run = 1;
				l_bit = c_bit;
			}
		}

		if (l_bit == std::get<2>(s)) {
			os << '<' << (l_bit ? '1' : '0') << ':';
			os << last_run << "...>";
		} else {
			if (last_run < 6)
				os.write(l_bit ? o_str : z_str, last_run);
			else {
				os << '<' << (l_bit ? '1' : '0') << ':';
				os << last_run << '>';
			}

			os << '<' << (l_bit ? '0' : '1') << "...>";
		}
		os << std::endl;
		return os;
	}

	bool test(size_type n) const
	{
		auto s(get());

		if (n < std::get<1>(s)) {
			auto w_off(n / word_bits);
			auto b_off(n % word_bits);
			return std::get<0>(s)[w_off] & (base_bit >> b_off);
		} else
			return std::get<2>(s);
	}

	dynamic_bitset &set(size_type n)
	{
		auto s(get());
		auto w_off(n / word_bits);
		auto b_off(n % word_bits);

		if (n < std::get<1>(s))
			std::get<0>(s)[w_off] |= (base_bit >> b_off);
		else if (!std::get<2>(s)) {
			s = grow_set(w_off + 1);
			std::get<0>(s)[w_off] |= (base_bit >> b_off);
		}

		return *this;
	}

	dynamic_bitset &reset(size_type n)
	{
		auto s(get());
		auto w_off(n / word_bits);
		auto b_off(n % word_bits);

		if (n < std::get<1>(s))
			std::get<0>(s)[w_off] &= ~(base_bit >> b_off);
		else if (std::get<2>(s)) {
			s = grow_set(w_off + 1);
			std::get<0>(s)[w_off] &= ~(base_bit >> b_off);
		}

		return *this;
	}

	size_type count(
		size_type begin_pos = 0, size_type end_pos = npos
	) const
	{
		auto s(get());

		auto x_end_pos(std::min(end_pos, std::get<1>(s)));

		if (begin_pos >= x_end_pos) {
			if (std::get<2>(s) && (end_pos > begin_pos))
				return end_pos - begin_pos;

			return 0;
		}

		auto w_off(begin_pos / word_bits);
		auto b_off(begin_pos % word_bits);
		auto ew_off(x_end_pos / word_bits);
		auto eb_off(x_end_pos % word_bits);

		auto w(std::get<0>(s)[w_off]);
		if (b_off)
			w &= (base_bit >> (b_off - 1)) - 1;

		size_type rv(0);

		while (w_off != ew_off) {
			rv += popcount(w);
			++w_off;
			w = std::get<0>(s)[w_off];
		}

		if (eb_off) {
			w &= ~((base_bit >> (eb_off - 1)) - 1);
			rv += popcount(w);
		}

		if ((end_pos > x_end_pos) && std::get<2>(s))
			rv += end_pos - x_end_pos;

		return rv;
	}

	size_type find_below(size_type n, size_type min_n = 0) const
	{
		auto s(get());

		if ((n > std::get<1>(s)) && std::get<2>(s)) {
			n -= 1;
			return n >= min_n ? n : npos;
		}

		if (n < min_n)
			return npos;

		--n;
		auto w_off(n / word_bits);
		auto b_off(n % word_bits);
		auto w(std::get<0>(s)[w_off]);

		w &= ~((base_bit >> b_off) - 1);

		if (w) {
			n = w_off * word_bits + (word_bits - ffs(w) - 1);
			return n >= min_n ? n : npos;
		} else if (!w_off)
			return npos;

		do {
			--w_off;
			n = w_off * word_bits + (word_bits - 1);
			w = std::get<0>(s)[w_off];
			if (w) {
				n -= ffs(w);
				return n >= min_n ? n : npos;
			}
		} while (w_off);

		return npos;
	}

	size_type find_above(size_type n, size_type max_n = npos) const
	{
		auto s(get());

		++n;
		if (n >= std::get<1>(s)) {
			if (std::get<2>(s) && (n < max_n))
				return n;
			else
				return npos;
		}

		auto w_off(n / word_bits);
		auto b_off(n % word_bits);
		auto w(std::get<0>(s)[w_off]);

		w &= (((base_bit >> b_off) - 1) << 1) | 1;

		if (w) {

			n = w_off * word_bits + (word_bits - fls(w) - 1);
			if (((n < std::get<1>(s)) || (std::get<2>(s)))
			    && (n < max_n))
				return n;
			else
				return npos;
		}
		
		++w_off;
		while (true) {
			n = w_off * word_bits;

			if (n >= max_n)
				return npos;

			if (n >= std::get<1>(s))
				return std::get<2>(s) ? std::get<1>(s) : npos;

			auto w(std::get<0>(s)[w_off]);
			if (!w)
				continue;

			n += (word_bits - fls(w) - 1);

			if (n >= max_n)
				return npos;

			if (n < std::get<1>(s))
				return n;
			else
				return std::get<2>(s) ? std::get<1>(s) : npos;

			++w_off;
		}
	}

private:
	constexpr static unsigned int word_bits
	= std::numeric_limits<word_type>::digits;
	constexpr static word_type base_bit
	= (~word_type(0)) ^ (~word_type(0) >> 1);

	std::tuple<word_type const *, size_type, bool> get() const
	{
		if (std::get<0>(bset) & 1)
			return std::make_tuple(
				&std::get<0>(bset), word_bits - 2,
				bool(std::get<0>(bset) & 2)
			);
		else {
			auto ptr(reinterpret_cast<word_type const *>(
				std::get<0>(bset)
			));
			return std::make_tuple(
				ptr + 1,  (ptr[0] >> 1) * word_bits,
				bool(ptr[0] & 1)
			);
		}
	}

	std::tuple<word_type *, size_type, bool> get()
	{
		if (std::get<0>(bset) & 1) {
			return std::make_tuple(
				&std::get<0>(bset), word_bits - 2,
				bool(std::get<0>(bset) & 2)
			);
		} else {
			auto ptr(reinterpret_cast<word_type *>(
				std::get<0>(bset)
			));
			return std::make_tuple(
				ptr + 1,  (ptr[0] >> 1) * word_bits,
				bool(ptr[0] & 1)
			);
		}
	}
	
	std::tuple<word_type *, size_type, bool> grow_set(size_type word_sz)
	{
		auto sz(AllocPolicy::best_size(word_sz + 1));
		auto n_ptr(allocator_helper_type::alloc(std::get<1>(bset), sz));

		if (std::get<0>(bset) & 1) {
			n_ptr[1] = std::get<0>(bset);
			n_ptr[0] = ((sz - 1) << 1)
				   | ((n_ptr[1] >> 1) & word_type(1));
			n_ptr[1] &= ~word_type(1);
			n_ptr[1] |= (n_ptr[1] >> 1) & word_type(1);
			std::get<0>(bset) = reinterpret_cast<word_type>(n_ptr);
		} else {
			auto ptr(reinterpret_cast<word_type *>(
				std::get<0>(bset)
			));
			auto o_sz(ptr[0] >> 1);
			word_type fill_v(0);

			n_ptr[0] = (sz - 1) << 1;
			if (ptr[0] & 1) {
				n_ptr[0] |= word_type(1);
				fill_v = ~fill_v;
			}
			std::copy_n(ptr + 1, o_sz, n_ptr + 1);
			std::fill_n(n_ptr + o_sz + 1, sz - o_sz - 1, fill_v);
			std::get<0>(bset) = reinterpret_cast<word_type>(n_ptr);
			allocator_helper_type::destroy(
				std::get<1>(bset), ptr, o_sz + 1, true
			);
		}
		return std::make_tuple(
			n_ptr + 1, (sz - 1) * word_bits, bool(n_ptr[0] & 1)
		);
	}

	std::tuple<word_type, allocator_type> bset;
};

template <typename Alloc, typename AllocPolicy>
constexpr typename dynamic_bitset<
	Alloc, AllocPolicy
>::size_type dynamic_bitset<Alloc, AllocPolicy>::npos;

}}
#endif
