/*
 * Copyright (c) 2017 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_FF77219245BE9042F3558855A5F5B830)
#define HPP_FF77219245BE9042F3558855A5F5B830

#include <yesod/detail/bitops.hpp>

namespace ucpf::yesod::detail {

template <typename SizeType, size_t FirstNodeOrder, size_t NodeOrder>
struct paged_storage_layout {
	typedef SizeType size_type;
	constexpr static size_t size_type_bits = std::numeric_limits<
		size_type
	>::digits;

	static_assert(FirstNodeOrder > 0);
	static_assert(FirstNodeOrder <= size_type_bits);
	static_assert((FirstNodeOrder + NodeOrder) <= size_type_bits);

	constexpr static size_t max_height = (
		size_type_bits - FirstNodeOrder
	) ? (
		((size_type_bits - FirstNodeOrder) / NodeOrder)
		+ (((size_type_bits - FirstNodeOrder) % NodeOrder) ? 1 : 0)
	) : 0;

	struct slice_info {
		constexpr size_type resolve_offset(size_type pos) const
		{
			return (pos >> shift) & mask;
		}

		size_type mask;
		uint8_t shift;
	};

	struct slice_table_t {
		constexpr slice_table_t()
		: slices{}, heights{}
		{
			slices[0] = slice_info {
				FirstNodeOrder < size_type_bits
				?(size_type(1) << FirstNodeOrder) - 1
				: ~size_type(0),
				0
			};
			for (size_t h(1); h <= max_height; ++h) {
				slices[h] = slice_info {
					(size_type(1) << NodeOrder) - 1,
					uint8_t(
						FirstNodeOrder
						+ (h - 1) * NodeOrder
					)
				};
			}

			size_t pos(size_type_bits);
			do {
				--pos;
				auto b_pos(size_type_bits - pos - 1);
				if (b_pos >= FirstNodeOrder) {
					b_pos -= FirstNodeOrder;
					heights[pos] = b_pos / NodeOrder + 1;
				} else
					heights[pos] = 0;
			} while (pos);
		}

		slice_info slices[max_height + 1];
		uint8_t heights[size_type_bits];
	};

	constexpr static slice_table_t slice_table =  slice_table_t();

	constexpr static size_type offset_at_height(
		size_type pos, size_type h
	)
	{
		return slice_table.slices[h].resolve_offset(pos);
	}

	constexpr static size_t height_at_offset(size_type pos)
	{
		return pos ? slice_table.heights[clz(pos)] : 0;
	}

	paged_storage_layout() = delete;
};

template <typename SizeType, size_t FirstNodeOrder, size_t NodeOrder>
constexpr typename paged_storage_layout<
	SizeType, FirstNodeOrder, NodeOrder
>::slice_table_t paged_storage_layout<
	SizeType, FirstNodeOrder, NodeOrder
>::slice_table;

}
#endif
