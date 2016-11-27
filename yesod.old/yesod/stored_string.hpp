/*
 * Copyright (c) 2016 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_6CDF7C8C964324F0718AC0E4706DE405)
#define HPP_6CDF7C8C964324F0718AC0E4706DE405

#include <yesod/storage/utility.hpp>

namespace ucpf { namespace yesod {

template <typename CharType, typename StorageType>
struct stored_string {
	typedef CharType char_type;
	typedef StorageType storage_type;
	typedef typename storage_type::size_type size_type;

	static_assert(
		std::is_pod<CharType>::value, "std::is_pod<CharType>::value"
	);

	stored_string()
	{
		s.size = short_string_flag;
	}

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

	size_type size() const
	{
		if (s.size & short_string_flag)
			return s.size >> 1;
		else
			return l.size >> 1;
	}

#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__

	size_type size() const
	{
		if (s.size & short_string_flag)
			return s.size & 0x7f;
		else
			return l.size;
	}
#else
#error Unsupported endianess
#endif

	void release(storage_type const &st)
	{
		size_type sz = size();

		if (sz > short_string_sz) {
			storage::free_n(st, l.s_ptr, sz);
			l.~long_string();
		}

		s.size = short_string_flag;
	}

	char_type at(size_type pos) const
	{
		if (is_short())
			return s.s[pos];
		else
			return *(l.s_ptr + pos);
	}

	template <typename Iterator>
	void assign(Iterator first, std::size_t count, storage_type const &st)
	{
		release(st);
		if (count > short_string_sz) {
			::new (&l.s_ptr) char_ptr(
				storage::alloc_copy<char_type>(
					st, count, first
				)
			);
		} else {
			for (std::size_t pos(0); pos < count; ++pos) {
				s.s[pos] = *first;
				++first;
			}
		}

		size(count);
	}

private:
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

	static constexpr uint8_t short_string_flag = 0x01;

	void size(size_type sz)
	{
		if (sz > short_string_sz)
			l.size = sz << 1;
		else
			s.size = (sz << 1) | short_string_flag;
	}

#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN_

	static constexpr uint8_t short_string_flag = 0x80;

	void size(size_type sz)
	{
		if (sz > short_string_sz)
			l.size = sz;
		else
			s.size = sz | short_string_flag;
	}

#else
#error Unsupported endianess
#endif

	bool is_short() const
	{
		return s.size & short_string_flag;
	}

	typedef typename storage_type::template pointer<char_type> char_ptr;

	struct long_string {
		size_type size;
		char_ptr s_ptr;
	};

	static constexpr size_type short_string_sz
	= (sizeof(long_string) / sizeof(char_type)) - 1;

	struct short_string {
		union {
			uint8_t size;
			char_type dummy;
		};
		char_type s[short_string_sz];
	};

	union {
		short_string s;
		long_string l;
	};
};

}}
#endif
