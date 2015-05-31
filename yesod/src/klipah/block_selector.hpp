/*
 * Copyright (c) 2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#include <yesod/bitops.hpp>

namespace ucpf { namespace yesod { namespace allocator { namespace klipah {

struct klipah_policy_0 {
	constexpr static std::size_t min_block_order = 3;
	constexpr static std::size_t max_block_order = 17;
	constexpr static std::size_t block_order_diff = 6;
	constexpr static std::size_t block_class_count = 64;
	constexpr static std::size_t max_size_step
	= max_block_order + 1 - block_order_diff;

	constexpr static auto discrete_size(std::size_t size)
	{
		auto ord(fls(size));
		auto size_step(
			ord >= (min_block_order + block_order_diff)
			? ord - block_order_diff
			: min_block_order
		);

		if (size_step > max_size_step)
			size_step = max_size_step;

		size |= (std::size_t(1) << size_step) - 1;
		size += 1;
		return std::make_pair(size, ord);
	}
};

}}}}
