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

namespace ucpf::yesod::detail {

template <size_t...>
struct string_path_impl;

template <>
struct string_path_impl<> {
	typedef size_t size_type;

	struct element {
	private:
		element(uint8_t const *data_, size_type size_)
		: data(data_), size(size_)
		{}

		uint8_t const *data;
		size_type size;
	};

	static string_path_impl<> const &select(ptr_or_data const &p);

	template <typename SeqParser>
	static void populate(
		uint8_t *str, uint8_t *sep_map, SeqParser &&parser
	)
	{
		size_type pos(0);
		bitmap::set(sep_map, pos);

		while (parser.next_element(str, pos))
			bitmap::set(sep_map, pos);
	}

	virtual void deallocate(ptr_or_data &p) const
	{}

	virtual void copy(ptr_or_data &dst, ptr_or_data const &src) const = 0;
	virtual element element_at(
		ptr_or_data const &p, size_type pos
	) const = 0;
	virtual size_type element_count(ptr_or_data const &p) const = 0;
	virtual size_type byte_count(ptr_or_data const &p) const = 0;
};

template <>
struct string_path_impl<0> : string_path_impl<> {
	constexpr static size_type size_limit
	= ptr_or_data::data_size - 1 - bitmap::byte_count(
		ptr_or_data::data_size - 1
	);

	template <typename SeqParser>
	static void init(ptr_or_data &p, size_type str_sz, SeqParser &&parser)
	{
		p.reset();
		if (!str_sz)
			return;

		p.set_extra_bits(0, 0, 1);
		p.set_extra_bits(str_sz, 1, 7);

		populate(
			p.bytes + bitmap::byte_count(str_sz) + 1,
			p.bytes + 1, std::forward<SeqParser>(parser)
		);
	}

	static size_type storage_size(ptr_or_data const &p)
	{
		return 0;
	}

	virtual void copy(ptr_or_data &dst, ptr_or_data const &src) const
	{
		dst.copy_from(src);
	}

	virtual element element_at(ptr_or_data const &p, size_type pos) const
	{
		auto bmap_sz(bitmap::byte_count(byte_count()));
	}

	virtual size_type element_count(ptr_or_data const &p) const
	{
		auto bmap_sz(bitmap::byte_count(byte_count()));
		return bitmap::count_ones(p.bytes + 1, 0, bmap_sz << 3);
	}

	virtual size_type byte_count(ptr_or_data const &p) const
	{
		return p.get_extra_bits(1, 7);
	}
};

template <>
struct string_path_impl<1> : string_path_impl<> {
	template <typename SeqParser>
	static void init(
		ptr_or_data &p, size_type str_sz, SeqParser &&parser,
		pmr::memory_resource *mem_alloc
	)
	{
		p.reset();
		p.set_extra_bits(mem_alloc ? 3 : 2, 0, 2);
		p.set_extra_bits(str_sz, 2, ptr_or_data::extra_type_bits - 2);
		auto sz(storage_size(p));

		p.ptr = mem_alloc
			? mem_alloc->allocate(sz)
			: pmr::new_delete_resource()->allocate(sz);
		memset(
			p.get_ptr_at<uint8_t *>(0), 0,
			bitmap::byte_count(str_sz)
		);

		populate(
			p.get_ptr_at<uint8_t *>(bitmap::byte_count(str_sz)),
			p.get_ptr_at<uint8_t *>(0),
			std::forward<SeqParser>(parser)
		);

		*p.get_ptr_at<pmr::memory_resource **>(
			sz - sizeof(pmr::memory_resource *)
		) = mem_alloc;
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
			? *src.get_ptr_at<pmr::memory_resource **>(
				sz - sizeof(pmr::memory_resource *)
			) : pmr::new_delete_resource();

		dst.ptr = mem_alloc->allocate(sz);
		__builtin_memcpy(dst.ptr, src.ptr, sz);
	}

	virtual size_type element_count(ptr_or_data const &p) const
	{
		auto bmap_sz(bitmap::byte_count(byte_count()));
		return bitmap::count_ones(
			p.get_ptr_at<uint8_t const *>(0), 0,
			bmap_sz << 3
		);
	}

	virtual size_type byte_count(ptr_or_data const &p) const
	{
		return p.get_extra_bits(2, ptr_or_data::extra_type_bits - 2);
	}
};

constexpr static string_path_impl<0> string_path_impl_0;
constexpr static string_path_impl<1> string_path_impl_1;

string_path_impl<> const &string_path_impl<>::select(ptr_or_data const &p)
{
	return p.test_extra_bit(0) ? string_path_impl_1 : string_path_impl_0;
}

}
#endif
