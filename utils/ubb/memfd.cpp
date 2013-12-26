/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#include "memfd.hpp"

#include <algorithm>

namespace ubb {

memfd::memfd_base memfd::base;

size_t memfd::rw_data_node::read(char *buf, size_t count, off_t offset) const
{
	auto e_off(std::min(size_t(offset), size));
	auto e_sz(std::min(count, size - e_off));
	auto e_rem(e_sz);
	auto buf_off(0);
	auto b_start(e_off >> block_order);

	while (e_rem) {
		auto b_off(e_off & ((size_t(1) << block_order) - 1));
		auto b_sz(std::min(
			(size_t(1) << block_order) - b_off, e_rem
		));
		auto iter(data.find(b_start));

		if (iter != data.cend())
			std::memcpy(buf + buf_off, iter->second, b_sz);
		else
			std::memset(buf + buf_off, 0, b_sz);

		buf_off += b_sz;
		e_rem -= b_sz;
		++b_start;
	}
	return e_sz;
}

size_t memfd::rw_data_node::write(char const *buf, size_t count, off_t offset)
{
	auto e_rem(count);
	auto buf_off(0);
	auto b_start(size_t(offset) >> block_order);
	auto hint(data.begin());

	while (e_rem) {
		auto b_off(size_t(offset) & ((size_t(1) << block_order) - 1));
		auto b_sz(std::min(
			(size_t(1) << block_order) - b_off, e_rem
		));
		auto iter(data.find(b_start));
		if (iter != data.cend()) {
			std::memcpy(iter->second + b_off, buf + buf_off, b_sz);
			hint = iter;
		} else {
			auto p(reinterpret_cast<char *>(
				std::calloc(1, size_t(1) << block_order)
			));
			if (!p)
				break;
			hint = data.emplace_hint(
				hint, std::make_pair(b_start, p)
			);
			std::memcpy(p + b_off, buf + buf_off, b_sz);
		}
		buf_off += b_sz;
		e_rem -= b_sz;
		++b_start;
	}
	size = std::max(size, offset +  count - e_rem);
	return count - e_rem;
}

void memfd::rw_data_node::truncate(off_t length)
{
	if (size_t(length) < size) {
		auto b_start(length >> block_order);
		auto iter(data.find(b_start));
		if (std::distance(iter, data.end()) > 1) {
			std::for_each(
				iter, data.end(), [](
					decltype(data)::value_type const &p
				) { free(p.second); }
			);
			data.erase(iter, data.end());
		}
	}
	size = length;
}

}
