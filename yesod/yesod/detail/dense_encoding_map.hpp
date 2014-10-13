/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_YESOD_DETAIL_DENSE_ENCODING_MAP_20141013T2000)
#define UCPF_YESOD_DETAIL_DENSE_ENCODING_MAP_20141013T2000

#include <tuple>
#include <limits>
#include <yesod/allocator/array_helper.hpp>

namespace ucpf { namespace yesod { namespace detail {

template <typename T, typename Alloc = std::allocator<void>>
struct dense_encoding_map {
	typedef T value_type;
	typedef typename std::make_unsigned<value_type>::type index_type;
	typedef allocator::array_helper<
		index_type, Alloc
	> allocator_helper_type;
	typedef typename allocator_helper_type::allocator_type allocator_type;

	constexpr std::size_t size() const
	{
		return std::size_t(1) << std::numeric_limits<
			index_type
		>::digits;
	}

	dense_encoding_map(allocator_type const &a = allocator_type())
	: tup_items_alloc(allocator_helper_type::alloc_n(a, 2 * size()), a)
	{
		reset();
	}

	~dense_encoding_map()
	{
		allocator_helper_type::destroy(
			std::get<1>(tup_items_alloc),
			std::get<0>(tup_items_alloc),
			2 * size(), true
		);
	}

	void reset()
	{
		for (std::size_t c(0); c < size(); ++c) {
			std::get<0>(tup_items_alloc)[
				2 * c
			] = static_cast<index_type>(c);
			std::get<0>(tup_items_alloc)[
				2 * c + 1
			] = static_cast<index_type>(c);
		}
	}

	void set(value_type v, index_type id)
	{
		auto iv(static_cast<index_type>(v));
		auto other_id(std::get<0>(tup_items_alloc)[iv * 2]);
		auto other_iv(std::get<0>(tup_items_alloc)[id * 2 + 1]);
		std::get<0>(tup_items_alloc)[iv * 2] = id;
		std::get<0>(tup_items_alloc)[id * 2 + 1] = iv;
		std::get<0>(tup_items_alloc)[other_iv * 2] = other_id;
		std::get<0>(tup_items_alloc)[other_id * 2 + 1] = other_iv;
	}

	index_type index(value_type v) const
	{
		return std::get<0>(tup_items_alloc)[
			static_cast<index_type>(v) * 2
		];
	}

	value_type value(index_type id) const
	{
		return static_cast<value_type>(
			std::get<0>(tup_items_alloc)[id * 2 + 1]
		);
	}

	std::tuple<index_type *, allocator_type> tup_items_alloc;
};

}}}
#endif
