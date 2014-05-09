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

	dynamic_bitset(Alloc const &a = Alloc())
	: bset(word_type(1), allocator_type(a))
	{}

	dynamic_bitset(dynamic_bitset const &other)
	: bset(other.bset)
	{
		if (!(std::get<0>(bset) & 1)) {
			auto ptr(reinterpret_cast<word_type *>(
				std::get<0>(bset)
			));
			auto sz((ptr[0] >> 1) + 1);
			auto n_ptr(allocator_helper_type::alloc(
				std::get<1>(bset), ptr, ptr + sz
			));
			std::get<0>(bset) = reinterpret_cast<word_type>(n_ptr);
		}
	}

	dynamic_bitset(
		dynamic_bitset const &other, size_type left_reserve,
		size_type right_reserve
	): bset(other.bset)
	{
		auto sc(short_count());
		if (sc != npos) {
			sc += left_reserve + right_reserve;
			if (sc < (word_bits - 2)) {
				auto w(std::get<0>(bset));
				if (!(w & 2)) {
					w &= ~word_type(1);
					w >>= left_reserve;
					std::get<0>(bset) = ~w;
				} else {
					w = ~w;
					w >>= left_reserve;
					std::get<0>(bset) = ~w;
				}
			}
		}

		auto s(get());
		auto b_sz(std::get<1>(s) + left_reserve + right_reserve);
		size_type w_sz(AllocPolicy::best_size((b_sz / word_bits) + 2));
		auto n_ptr(allocator_helper_type::alloc_n(
			std::get<1>(bset), w_sz,
			std::get<2>(s) ? ~word_type(0) : word_type(0)
		));
		n_ptr[0] = (w_sz - 1) << 1;

		auto w_pos(left_reserve / word_bits);
		auto b_pos(left_reserve % word_bits);
		auto w_end((left_reserve + std::get<1>(s)) / word_bits);
		size_type o_pos(0);

		for (; w_pos < w_end; ++w_pos) {
			auto w(std::get<0>(s)[o_pos]);
			if (!std::get<2>(s)) {
				n_ptr[w_pos + 1] |= w >> b_pos;
				n_ptr[w_pos + 2] |= w << (word_bits - b_pos);
			} else {
				w = ~w;
				n_ptr[w_pos + 1] &= w >> b_pos;
				n_ptr[w_pos + 2] &= w << (word_bits - b_pos);
			}
			++o_pos;
		}

		auto end_mask(~((word_type(1) << (
			word_bits - (std::get<1>(s) % word_bits)
		)) - 1));
		auto w(std::get<0>(s)[o_pos]);

		if (!std::get<2>(s)) {
			w &= end_mask;
			n_ptr[w_end + 1] |= w << (word_bits - b_pos);
		} else {
			n_ptr[0] |= 1;
			w = ~w;
			w &= end_mask;
			n_ptr[w_end + 1] &= w << (word_bits - b_pos);
		}

		std::get<0>(bset) = reinterpret_cast<word_type>(n_ptr);
	}

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

	dynamic_bitset &set()
	{
		auto s(get());
		auto w_off((std::get<1>(s) - 1) / word_bits);
		auto b_off((std::get<1>(s) - 1) % word_bits);

		for (size_type n(0); n < w_off; ++n)
			std::get<0>(s)[n] = ~word_type(0);

		std::get<0>(s)[w_off] |= ~((base_bit >> b_off) - 1);
		set_fill_bit(true);

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

	dynamic_bitset &reset()
	{
		auto s(get());
		auto w_off((std::get<1>(s) - 1) / word_bits);
		auto b_off((std::get<1>(s) - 1) % word_bits);

		for (size_type n(0); n < w_off; ++n)
			std::get<0>(s)[n] = 0;

		std::get<0>(s)[w_off] &= (base_bit >> b_off) - 1;
		set_fill_bit(false);
		return *this;
	}

	template <bool CountOne>
	size_type count(
		size_type begin_pos = 0, size_type end_pos = npos
	) const
	{
		auto s(get());
		auto x_end_pos(std::min(end_pos, std::get<1>(s)));

		if (begin_pos >= x_end_pos) {
			if (
				(std::get<2>(s) == CountOne)
				&& (end_pos > begin_pos)
			)
				return end_pos - begin_pos;

			return 0;
		}

		auto w_off(begin_pos / word_bits);
		auto b_off(begin_pos % word_bits);
		auto ew_off(x_end_pos / word_bits);
		auto eb_off(x_end_pos % word_bits);

		auto w(std::get<0>(s)[w_off]);
		if (!CountOne)
			w = ~w;

		if (b_off)
			w &= (base_bit >> (b_off - 1)) - 1;

		size_type rv(0);

		while (w_off <= ew_off) {
			rv += popcount(w);
			++w_off;
			w = std::get<0>(s)[w_off];
			if (!CountOne)
				w = ~w;
		}

		if (eb_off) {
			w &= ~((base_bit >> (eb_off - 1)) - 1);
			rv += popcount(w);
		}

		if ((end_pos > x_end_pos) && (std::get<2>(s) == CountOne))
			rv += end_pos - x_end_pos;

		return rv;
	}

	template <bool FindOne>
	size_type find_below(size_type n, size_type min_n = 0) const
	{
		auto s(get());

		if ((n > std::get<1>(s)) && (std::get<2>(s) == FindOne)) {
			n -= 1;
			return n >= min_n ? n : npos;
		}

		if (n <= min_n)
			return npos;

		--n;
		auto w_off(n / word_bits);
		auto b_off(n % word_bits);
		auto w(std::get<0>(s)[w_off]);

		if (!FindOne)
			w = ~w;

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
			if (!FindOne)
				w = ~w;

			if (w) {
				n -= ffs(w);
				return n >= min_n ? n : npos;
			}
		} while (w_off);

		return npos;
	}

	template <bool FindOne>
	size_type find_above(size_type n, size_type max_n = npos) const
	{
		auto s(get());

		++n;
		if (n >= std::get<1>(s)) {
			if ((std::get<2>(s) == FindOne) && (n < max_n))
				return n;
			else
				return npos;
		}

		auto w_off(n / word_bits);
		auto b_off(n % word_bits);
		auto w(std::get<0>(s)[w_off]);

		if (!FindOne)
			w = ~w;

		w &= (((base_bit >> b_off) - 1) << 1) | 1;


		if (w) {
			n = w_off * word_bits + (word_bits - fls(w) - 1);
			if ((
				(n < std::get<1>(s))
				|| (std::get<2>(s) == FindOne)
			) && (n < max_n))
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
				return (std::get<2>(s) == FindOne)
				       ? std::get<1>(s) : npos;

			auto w(std::get<0>(s)[w_off]);
			if (!FindOne)
				w = ~w;

			if (!w)
				continue;

			n += (word_bits - fls(w) - 1);

			if (n >= max_n)
				return npos;

			if (n < std::get<1>(s))
				return n;
			else
				return (std::get<2>(s) == FindOne)
				       ? std::get<1>(s) : npos;

			++w_off;
		}
	}

	void swap(dynamic_bitset &other)
	{
		std::swap(bset, other.bset);
	}

	bool valid(size_type n) const
	{
		auto s(get());
		return n < std::get<1>(s);
	}

private:
	constexpr static unsigned int word_bits
	= std::numeric_limits<word_type>::digits;
	constexpr static word_type base_bit
	= (~word_type(0)) ^ (~word_type(0) >> 1);

	size_type short_count() const
	{
		auto w(std::get<0>(bset));

		if (!(w & 1))
			return npos;

		if (w & 2)
			w = ~w;
		else
			w &= ~word_type(1);

		if (!w)
			return 0;
		auto p1(fls(w));
		auto p2(ffs(w));
		return p1 - p2;
	}

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
		auto n_ptr(allocator_helper_type::alloc_n(
			std::get<1>(bset), sz
		));

		if (std::get<0>(bset) & 1) {
			n_ptr[1] = std::get<0>(bset);
			n_ptr[0] = ((sz - 1) << 1)
				   | ((n_ptr[1] >> 1) & word_type(1));
			auto w(n_ptr[1] & 2 ? ~word_type(0) : word_type(0));
			if (!w)
				n_ptr[1] &= ~word_type(1);

			std::fill_n(n_ptr + 2, sz - 2, w);
			std::get<0>(bset) = reinterpret_cast<word_type>(n_ptr);
		} else {
			auto ptr(reinterpret_cast<word_type *>(
				std::get<0>(bset)
			));
			auto o_sz(ptr[0] >> 1);
			word_type w(0);

			n_ptr[0] = (sz - 1) << 1;
			if (ptr[0] & 1) {
				n_ptr[0] |= word_type(1);
				w = ~w;
			}
			std::copy_n(ptr + 1, o_sz, n_ptr + 1);
			std::fill_n(n_ptr + o_sz + 1, sz - o_sz - 1, w);
			std::get<0>(bset) = reinterpret_cast<word_type>(n_ptr);
			allocator_helper_type::destroy(
				std::get<1>(bset), ptr, o_sz + 1, true
			);
		}
		return std::make_tuple(
			n_ptr + 1, (sz - 1) * word_bits, bool(n_ptr[0] & 1)
		);
	}

	void set_fill_bit(bool b)
	{
		if (std::get<0>(bset) & 1) {
			if (b)
				std::get<0>(bset) |= word_type(2);
			else
				std::get<0>(bset) &= ~word_type(2);
		} else {
			auto ptr(reinterpret_cast<word_type *>(
				std::get<0>(bset)
			));
			if (b)
				ptr[0] |= word_type(1);
			else
				ptr[0] &= ~word_type(1);
		}
	}

	std::tuple<word_type, allocator_type> bset;
};

template <typename Alloc, typename AllocPolicy>
constexpr typename dynamic_bitset<
	Alloc, AllocPolicy
>::size_type dynamic_bitset<Alloc, AllocPolicy>::npos;

}}
#endif
