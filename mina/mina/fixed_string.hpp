/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_MINA_FIXED_STRING_20141103T1940)
#define UCPF_MINA_FIXED_STRING_20141103T1940

#include <ostream>
#include <cstring>
#include <yesod/allocator/array_helper.hpp>

namespace ucpf { namespace mina {

struct fixed_string {
	typedef uint8_t value_type;
	typedef std::size_t size_type;
	typedef value_type *iterator;
	typedef value_type const *const_iterator;
	typedef value_type &reference;
	typedef value_type const &const_reference;
	typedef value_type *pointer;
	typedef value_type const *const_pointer;

	static fixed_string make()
	{
		fixed_string s;

		s.s_str.count = flag_bit;
		return s;
	}

	template <typename Alloc>
	static fixed_string make_s(Alloc const &a, char const *other)
	{
		return make_r(
			a, other,
			other + std::char_traits<char>::length(other)
		);
	}

	template <typename Alloc, typename Iterator>
	static fixed_string make_r(
		Alloc const &a, Iterator first, Iterator last
	)
	{
		typedef yesod::allocator::array_helper<value_type, Alloc> a_h;
		auto sz(std::distance(first, last));
		fixed_string s;

		if (sz > static_cast<decltype(sz)>(small_string_size)) {
			s.l_str.val_ptr = a_h::alloc_r(a, first, last);
			s.l_str.count = sz << size_shift;
		} else {
			a_h::make_r(a, s.s_str.val, first, last);
			s.s_str.count = static_cast<value_type>(sz);
			s.s_str.count <<= size_shift;
			s.s_str.count |= flag_bit;
		}

		return s;
	}

	template <typename Alloc>
	static fixed_string make(
		Alloc const &a, size_type n, value_type c
	)
	{
		typedef yesod::allocator::array_helper<value_type, Alloc> a_h;
		fixed_string s;

		if (n > small_string_size) {
			s.l_str.val_ptr = a_h::alloc_n(a, n, c);
			s.l_str.count = n << size_shift;
		} else {
			a_h::make_n(a, s.s_str.val, n, c);
			s.s_str.count = static_cast<value_type>(n);
			s.s_str.count <<= size_shift;
			s.s_str.count |= flag_bit;
		}

		return s;
	}

	template <typename Alloc, typename... Args>
	static fixed_string make(Alloc const &a, Args&&... args)
	{
		typedef yesod::allocator::array_helper<value_type, Alloc> a_h;
		fixed_string s;
		std::array<size_type, sizeof...(args)> arg_sz{args.size()...};

		size_type c_sz(0);
		for (auto sz: arg_sz)
			c_sz += sz;

		pointer p(nullptr);
		if (c_sz > small_string_size) {
			p = a_h::alloc_n(a, c_sz);
			s.l_str.val_ptr = p;
			s.l_str.count = c_sz << size_shift;
		} else {
			p = s.s_str.val;
			s.s_str.count = static_cast<value_type>(c_sz);
			s.s_str.count <<= size_shift;
			s.s_str.count |= flag_bit;
		}

		emplace_next(a, p, std::forward<Args>(args)...); 
		return s;
	}

	template <typename Alloc>
	static void destroy(Alloc const &a, fixed_string &s)
	{
		typedef yesod::allocator::array_helper<value_type, Alloc> a_h;

		a_h::destroy(
			a, s.str_ptr(), s.size(), !(s.s_str.count & flag_bit)
		);
		s = make();
	}

	size_type size() const
	{
		if (s_str.count & flag_bit)
			return (s_str.count >> size_shift) & short_size_mask;
		else
			return (l_str.count >> size_shift) & long_size_mask;
	}

	bool empty() const
	{
		return s_str.count == flag_bit;
	}

	reference operator[](size_type pos)
	{
		return *(str_ptr() + pos);
	}

	const_reference operator[](size_type pos) const
	{
		return *(str_ptr() + pos);
	}

	iterator begin()
	{
		return str_ptr();
	}

	const_iterator begin() const
	{
		return str_ptr();
	}

	iterator end()
	{
		return str_ptr() + size();
	}

	const_iterator end() const
	{
		return str_ptr() + size();
	}

	friend std::ostream &operator<<(
		std::ostream &os, fixed_string const &s
	)
	{
		return os.write(
			reinterpret_cast<char const *>(s.str_ptr()), s.size()
		);
	}

private:
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	constexpr static int size_shift = 1;
	constexpr static value_type short_size_mask = ~value_type(0);
	constexpr static size_type long_size_mask = ~size_type(0);
	constexpr static value_type flag_bit = 1;
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	constexpr static int size_shift = 0;
	constexpr static value_type short_size_mask = (~value_type(0)) >> 1;
	constexpr static size_type long_size_mask = (~size_type(0)) >> 1;
	constexpr static value_type flag_bit = short_size_mask + 1;
#else
#error Unsupported endianess
#endif

	template <typename Alloc, typename S0>
	static void emplace_next(Alloc const &a, pointer p, S0 &&s0)
	{
		typedef yesod::allocator::array_helper<value_type, Alloc> a_h;

		a_h::make_r(a, p, s0.begin(), s0.end());
	}

	template <typename Alloc, typename S0, typename... Sn>
	static void emplace_next(
		Alloc const &a, pointer p, S0 &&s0, Sn&&... sn
	)
	{
		typedef yesod::allocator::array_helper<value_type, Alloc> a_h;

		a_h::make_r(a, p, s0.begin(), s0.end());
		emplace_next(a, p + s0.size(), std::forward<Sn>(sn)...);
	}

	pointer str_ptr()
	{
		return (s_str.count & flag_bit) ? s_str.val : l_str.val_ptr;
	}

	const_pointer str_ptr() const
	{
		return (s_str.count & flag_bit) ? s_str.val : l_str.val_ptr;
	}

	struct long_string {
		size_type count;
		value_type *val_ptr;
	};

	constexpr static std::size_t small_string_size
	= sizeof(long_string) - 1;

	struct [[gnu::packed]] short_string {
		value_type count;
		value_type val[small_string_size];
	};

	union {
		long_string l_str;
		short_string s_str;
	};
};

}}

#endif
