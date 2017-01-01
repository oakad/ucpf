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

	virtual size_type element_count(ptr_or_data const &p) const = 0;
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

		p.set_extra_bits(0, 0, 2);
		p.set_extra_bits(str_sz, 2, 6);

		populate(
			p.bytes + bitmap::byte_count(str_sz) + 1,
			p.bytes + 1, std::forward<SeqParser>(parser)
		);
	}

	static size_type storage_size(ptr_or_data const &p)
	{
		return 0;
	}

	virtual size_type element_count(ptr_or_data const &p) const
	{
		auto str_sz(p.get_extra_bits(2, 6));
		return bitmap::count_ones(
			p.bytes + 1, bitmap::byte_count(str_sz)
		);
	}
};

template <>
struct string_path_impl<1> : string_path_impl<0> {
	constexpr static size_type size_limit = sizeof(
		ptr_or_data::extra_type
	) - 1 - bitmap::byte_count(
		sizeof(ptr_or_data::extra_type) - 1
	);

	template <typename SeqParser>
	static void init(
		ptr_or_data &p, size_type str_sz, SeqParser &&parser, 
		pmr::memory_resource *mem_alloc
	)
	{
		string_path_impl<0>::init(
			p, str_sz, std::forward<SeqParser>(parser)
		);
		p.ptr = mem_alloc;
		p.set_extra_bits(1, 0, 2);
	}

	static size_type storage_size(ptr_or_data const &p)
	{
		return 0;
	}
};

template <>
struct string_path_impl<2> : string_path_impl<> {
	template <typename SeqParser>
	static void init(
		ptr_or_data &p, size_type str_sz, SeqParser &&parser
	)
	{
		p.reset();
		p.set_extra_bits(2, 0, 2);
		p.set_extra_bits(str_sz, 2, ptr_or_data::extra_type_bits - 2);
		p.ptr = pmr::new_delete_resource()->allocate(
			storage_size(p)
		);
		memset(
			p.get_ptr_at<uint8_t *>(0), 0,
			bitmap::byte_count(str_sz)
		);

		populate(
			p.get_ptr_at<uint8_t *>(bitmap::byte_count(str_sz)),
			p.get_ptr_at<uint8_t *>(0),
			std::forward<SeqParser>(parser)
		);
	}

	static size_type storage_size(ptr_or_data const &p)
	{
		auto str_sz(p.get_extra_bits(
			2, ptr_or_data::extra_type_bits - 2
		));
		return str_sz + bitmap::byte_count(str_sz);
	}

	virtual void deallocate(ptr_or_data &p) const
	{
		pmr::new_delete_resource()->deallocate(
			p.ptr, storage_size(p)
		);
	}

	virtual size_type element_count(ptr_or_data const &p) const
	{
		auto str_sz(p.get_extra_bits(
			2, ptr_or_data::extra_type_bits - 2
		));
		return bitmap::count_ones(
			p.get_ptr_at<uint8_t const *>(0),
			bitmap::byte_count(str_sz)
		);
	}
};

template <>
struct string_path_impl<3> : string_path_impl<2> {
	template <typename SeqParser>
	static void init(
		ptr_or_data &p, size_type str_sz, SeqParser &&parser,
		pmr::memory_resource *mem_alloc
	)
	{
		p.reset();
		p.set_extra_bits(3, 0, 2);
		p.set_extra_bits(str_sz, 2, ptr_or_data::extra_type_bits - 2);
		auto sz(storage_size(p));
		p.ptr = mem_alloc->allocate(sz);
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
		auto sz(string_path_impl<2>::storage_size(p));
		auto rem(sz % sizeof(pmr::memory_resource *));
		if (rem)
			sz += sizeof(pmr::memory_resource *) - rem;

		return sz + sizeof(pmr::memory_resource *);
	}

	virtual void deallocate(ptr_or_data &p) const
	{
		auto sz(storage_size(p));
		auto mr(*p.get_ptr_at<pmr::memory_resource **>(
			sz - sizeof(pmr::memory_resource *)
		));
		mr->deallocate(p.ptr, sz);
		return;
	}
};

constexpr static string_path_impl<0> string_path_impl_0;
constexpr static string_path_impl<1> string_path_impl_1;
constexpr static string_path_impl<2> string_path_impl_2;
constexpr static string_path_impl<3> string_path_impl_3;

string_path_impl<> const &string_path_impl<>::select(ptr_or_data const &p)
{
	constexpr static void *dispatch_tbl[] = {
		&&impl_0, &&impl_1, &&impl_2, &&impl_3
	};
	goto *dispatch_tbl[p.get_extra_bits(0, 2)];
impl_0:
	return string_path_impl_0;
impl_1:
	return string_path_impl_1;
impl_2:
	return string_path_impl_2;
impl_3:
	return string_path_impl_3;
}

}
#endif
