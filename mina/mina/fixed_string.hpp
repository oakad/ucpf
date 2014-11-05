/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_MINA_FIXED_STRING_20141103T1940)
#define UCPF_MINA_FIXED_STRING_20141103T1940

#include <yesod/allocator/array_helper.hpp>

namespace ucpf { namespace mina {

struct fixed_string {
	typedef uint8_t value_type;
	typedef std::size_t size_type;
	typedef value_type *iterator;
	typedef value_type const *const_iterator;
	typedef value_type &reference;
	typedef value_type const &const_reference;

	static fixed_string make()
	{
		fixed_string s;

		s.s_str.count = flag_bit;
		return s;
	}

	template <typename Alloc, typename Iterator>
	static fixed_string make(
		Alloc const &a, Iterator first, Iterator last
	)
	{
		allocator::array_helper<value_type, Alloc> a_h;
		auto sz(std::distance(first, last));

		if (sz > small_string_size) {
			l_str.count = sz << size_shift;
			l_str.val_ptr = a_h::alloc(a, first, last);
		} else {
			s_str.count = static_cast<value_type>(sz);
			s_str.count <<= size_shift;
			s_str.count |= flag_bit;
			a_h::make(a, s_str.val, first, last);
		}

		return rv;
	}

	template <typename Alloc>
	static fixed_string make(
		Alloc const &a, size_type n, value_type c
	)
	{
		allocator::array_helper<value_type, Alloc> a_h;

		if (n > small_string_size) {
			l_str.count = n << size_shift;
			l_str.val_ptr = a_h::alloc_n(a, n, c);
		} else {
			s_str.count = static_cast<value_type>(n);
			s_str.count <<= size_shift;
			s_str.count |= flag_bit;
			a_h::make_n(a, s_str.val, n, c);
		}

		return rv;
	}

	template <typename Alloc>
	static void destroy(Alloc const &a, fixed_string &s)
	{
		allocator::array_helper<value_type, Alloc> a_h;

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

	reference operator[](size_type pos)
	{
		return *(str_ptr() + pos);
	}

	const_reference operator[](size_type pos) const
	{
		return *(str_ptr() + pos);
	}

	friend std::ostream &operator<<(
		std::ostream &os, fixed_string const &s
	)
	{
		return os.write(str_ptr(), size());
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

	value_type *str_ptr()
	{
		return (s_str.count & flag_bit) ? s_str.val : l_str.val_ptr;
	}

	value_type const *str_ptr() const
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
