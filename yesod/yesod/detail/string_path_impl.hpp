/*
 * Copyright (c) 2016 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_A6E95911F4526017550293E0CECA35C8)
#define HPP_A6E95911F4526017550293E0CECA35C8

#include <yesod/memory.hpp>
#include <yesod/detail/bitmap.hpp>
#include <yesod/detail/ptr_or_data.hpp>

#include <experimental/string_view>
#include <string>

namespace ucpf::yesod::detail {

template <size_t...>
struct string_path_impl;

template <>
struct string_path_impl<> {
	typedef size_t size_type;
	typedef std::experimental::basic_string_view<uint8_t> value_type;

	struct const_iterator : iterator::facade<
		const_iterator, value_type const,
		std::bidirectional_iterator_tag, value_type const
	> {
		const_iterator()
		: b_ptr(nullptr), data_ptr(nullptr), data_size(0), pos(0) {}

	private:
		template <size_t...>
		friend struct string_path_impl;
		friend struct iterator::core_access;

		const_iterator(
			uint8_t const *b_ptr_, uint8_t const *data_ptr_,
			size_type data_size_, size_type pos_
		) : b_ptr(b_ptr_), data_ptr(data_ptr_), data_size(data_size_),
		    pos(pos_)
		{}

		reference dereference() const
		{
			auto e_pos(bitmap::find_nth_one(
				b_ptr, 1, pos + 1, data_size
			));
			return value_type(data_ptr + pos, e_pos + 1);
		}

		bool equal(const_iterator const &other) const
		{
			return (data_ptr == other.data_ptr)
				&& (pos == other.pos);
		}

		void increment()
		{
			pos += bitmap::find_nth_one(
				b_ptr, 1, pos + 1, data_size
			) + 1;
		}

		void decrement()
		{
			pos -= bitmap::find_nth_one_r(
				b_ptr, 1, 0, pos
			);
		}

		void advance(difference_type count)
		{
			if (count >= 0)
				pos += bitmap::find_nth_one(
					b_ptr, count, pos + 1, data_size
				);
			else
				pos -= bitmap::find_nth_one_r(
					b_ptr, -count, 0, pos
				);	
		}

		difference_type distance_to(const_iterator const &other) const
		{
			return difference_type(other.pos) - pos;
		}

		uint8_t const *b_ptr;
		uint8_t const *data_ptr;
		size_type data_size;
		size_type pos;
	};

	static string_path_impl<> const &select(ptr_or_data const &p);

	template <typename SeqParser>
	static void populate_p(
		uint8_t *str, uint8_t *sep_map, SeqParser &&parser
	)
	{
		size_type pos(0);
		bitmap::set(sep_map, pos);

		while (parser.next_element(str, pos))
			bitmap::set(sep_map, pos);
	}

	template <typename StringPathType>
	static void populate_c(
		uint8_t *str, uint8_t *sep_map, size_type &pos,
		StringPathType &&other
	)
	{
		auto iter(other.begin());
		bitmap::copy(sep_map, pos, iter.b_ptr, 0, iter.data_size);
		__builtin_memcpy(str + pos, iter.data_ptr, iter.data_size);
		pos += iter.data_size;
	}

	static bool equals(
		const_iterator const &it0, const_iterator const &it1
	)
	{
		if (it0.data_size != it1.data_size)
			return false;

		if (__builtin_memcmp(
			it0.b_ptr, it1.b_ptr, bitmap::byte_count(it0.data_size)
		))
			return false;

		return !(__builtin_memcmp(
			it0.data_ptr, it1.data_ptr, it0.data_size
		));
	}

	virtual void deallocate(ptr_or_data &p) const
	{}

	virtual void copy(ptr_or_data &dst, ptr_or_data const &src) const = 0;
	virtual size_type value_count(ptr_or_data const &p) const = 0;
	virtual size_type byte_count(ptr_or_data const &p) const = 0;
	virtual const_iterator cbegin(ptr_or_data const &p) const = 0;
	virtual const_iterator cend(ptr_or_data const &p) const = 0;
};

template <>
struct string_path_impl<0> : string_path_impl<> {
	constexpr static size_type size_limit
	= ptr_or_data::data_size - 1 - bitmap::byte_count(
		ptr_or_data::data_size - 1
	);

	static void common_init(ptr_or_data &p, size_type str_sz)
	{
		p.reset();
		if (!str_sz)
			return;

		p.set_extra_bits(0, 0, 1);
		p.set_extra_bits(str_sz, 1, 7);
	}

	template <typename SeqParser>
	static void init_p(
		ptr_or_data &p, size_type str_sz, SeqParser &&parser
	)
	{
		common_init(p, str_sz);
		populate_p(
			p.bytes + bitmap::byte_count(str_sz) + 1,
			p.bytes + 1, std::forward<SeqParser>(parser)
		);
	}

	template <typename... StringPathType>
	static void init_c(
		ptr_or_data &p, size_type str_sz, StringPathType &&...other
	)
	{
		common_init(p, str_sz);
		size_type pos(0);
		(populate_c(
			p.bytes + bitmap::byte_count(str_sz) + 1,
			p.bytes + 1, pos, std::forward<StringPathType>(other)
		), ...);
	}

	static size_type storage_size(ptr_or_data const &p)
	{
		return 0;
	}

	virtual void copy(ptr_or_data &dst, ptr_or_data const &src) const
	{
		dst.copy_from(src);
	}

	virtual size_type value_count(ptr_or_data const &p) const
	{
		return bitmap::count_ones(p.bytes + 1, 0, byte_count(p));
	}

	virtual size_type byte_count(ptr_or_data const &p) const
	{
		return p.get_extra_bits(1, 7);
	}

	virtual const_iterator cbegin(ptr_or_data const &p) const
	{
		auto sz(byte_count(p));
		return const_iterator(
			p.bytes + 1, p.bytes + 1 + bitmap::byte_count(sz),
			sz, 0
		);
	}
	virtual const_iterator cend(ptr_or_data const &p) const
	{
		auto sz(byte_count(p));
		return const_iterator(
			p.bytes + 1, p.bytes + 1 + bitmap::byte_count(sz),
			sz, sz
		);
	}
};

template <>
struct string_path_impl<1> : string_path_impl<> {
	static void common_init(
		ptr_or_data &p, size_type str_sz,
		pmr::memory_resource *mem_alloc
	)
	{
		p.reset();
		p.set_extra_bit(0);
		if (mem_alloc)
			p.set_extra_bit(1);

		p.set_extra_bits(str_sz, 2, ptr_or_data::extra_type_bits - 2);
		auto sz(storage_size(p));

		p.ptr = mem_alloc
			? mem_alloc->allocate(sz)
			: pmr::new_delete_resource()->allocate(sz);
		memset(
			p.get_ptr_at<uint8_t *>(0), 0,
			bitmap::byte_count(str_sz)
		);

		if (mem_alloc)
			*p.get_ptr_at<pmr::memory_resource **>(
				sz - sizeof(pmr::memory_resource *)
			) = mem_alloc;
	}

	template <typename SeqParser>
	static void init_p(
		ptr_or_data &p, size_type str_sz,
		pmr::memory_resource *mem_alloc, SeqParser &&parser
	)
	{
		common_init(p, str_sz, mem_alloc);
		populate_p(
			p.get_ptr_at<uint8_t *>(bitmap::byte_count(str_sz)),
			p.get_ptr_at<uint8_t *>(0),
			std::forward<SeqParser>(parser)
		);
	}

	template <typename... StringPathType>
	static void init_c(
		ptr_or_data &p, size_type str_sz,
		pmr::memory_resource *mem_alloc, StringPathType &&...other
	)
	{
		common_init(p, str_sz, mem_alloc);
		size_type pos(0);

		(populate_c(
			p.get_ptr_at<uint8_t *>(bitmap::byte_count(str_sz)),
			p.get_ptr_at<uint8_t *>(0), pos,
			std::forward<StringPathType>(other)
		), ...);
	}

	static size_type storage_size(ptr_or_data const &p)
	{
		auto str_sz(p.get_extra_bits(
			2, ptr_or_data::extra_type_bits - 2
		));
		str_sz += bitmap::byte_count(str_sz);

		if (p.test_extra_bit(1)) {
			auto rem(str_sz % sizeof(pmr::memory_resource *));
			if (rem)
				str_sz += sizeof(pmr::memory_resource *) - rem;

			str_sz += sizeof(pmr::memory_resource *);
		}
		return str_sz;
	}

	virtual void deallocate(ptr_or_data &p) const
	{
		auto sz(storage_size(p));

		pmr::memory_resource *mem_alloc = p.test_extra_bit(1)
			? *p.get_ptr_at<pmr::memory_resource **>(
				sz - sizeof(pmr::memory_resource *)
			) : pmr::new_delete_resource();

		mem_alloc->deallocate(p.ptr, sz);
	}

	virtual void copy(ptr_or_data &dst, ptr_or_data const &src) const
	{
		dst.extra = src.extra;
		auto sz = storage_size(src);
		pmr::memory_resource *mem_alloc = src.test_extra_bit(1)
			? *src.get_ptr_at<pmr::memory_resource * const *>(
				sz - sizeof(pmr::memory_resource *)
			) : pmr::new_delete_resource();

		dst.ptr = mem_alloc->allocate(sz);
		__builtin_memcpy(dst.ptr, src.ptr, sz);
	}

	virtual size_type value_count(ptr_or_data const &p) const
	{
		return bitmap::count_ones(
			p.get_ptr_at<uint8_t const *>(0), 0, byte_count(p)
		);
	}

	virtual size_type byte_count(ptr_or_data const &p) const
	{
		return p.get_extra_bits(2, ptr_or_data::extra_type_bits - 2);
	}

	virtual const_iterator cbegin(ptr_or_data const &p) const
	{
		auto sz(byte_count(p));
		auto ptr(p.get_ptr_at<uint8_t const *>(0));
		return const_iterator(
			ptr, ptr + bitmap::byte_count(sz),
			sz, 0
		);
	}

	virtual const_iterator cend(ptr_or_data const &p) const
	{
		auto sz(byte_count(p));
		auto ptr(p.get_ptr_at<uint8_t const *>(0));
		return const_iterator(
			ptr, ptr + bitmap::byte_count(sz),
			sz, sz
		);
	}
};

constexpr static string_path_impl<0> string_path_impl_0;
constexpr static string_path_impl<1> string_path_impl_1;

string_path_impl<> const &string_path_impl<>::select(ptr_or_data const &p)
{
	if (p.test_extra_bit(0))
		return string_path_impl_1;
	else
		return string_path_impl_0;
}

}
#endif
