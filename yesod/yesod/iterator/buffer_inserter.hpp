/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_YESOD_ITERATOR_BUFFER_INSERTER_20140616T930)
#define UCPF_YESOD_ITERATOR_BUFFER_INSERTER_20140616T930

#include <yesod/detail/placement_array.hpp>

namespace ucpf { namespace yesod { namespace iterator {

template <
	typename ValueType, size_t BlockSize,
	typename Alloc = std::allocator<void>
> struct buffer_inserter : std::iterator<
	std::output_iterator_tag, ValueType, void, void, void
> {
	static_assert(BlockSize > 1, "BlockSize > 1");

	buffer_inserter(bool create = false)
	{
		if (!create)
			return;

		
	}

	buffer_inserter(buffer_inserter const &other)
	: base_ptr(other.base_ptr)
	{}

	buffer_inserter(buffer_inserter &&other)
	: base_ptr(std::move(other.base_ptr))
	{}

	buffer_inserter &operator=(buffer_inserter const &other)
	{
		base_ptr = other.base_ptr;
		return *this;
	}

	buffer_inserter &operator=(buffer_inserter &&other)
	{
		base_ptr = std::move(other.base_ptr);
		return *this;
	}

	buffer_inserter &operator=(ValueType const &value)
	{
		auto n_pos(get_next_pos());
		return *this;
	}

	buffer_inserter &operator=(ValueType &&value)
	{
		auto n_pos(get_next_pos());
		return *this;
	}

	buffer_inserter &operator*() const
	{
		return *this;
	}

	buffer_inserter &operator++()
	{
		return *this;
	}

	buffer_inserter &operator++(int)
	{
		return *this;
	}

private:
	struct node_type {
		node *prev;
		std::atomic<size_t> pos;
		placement_array<ValueType, BlockSize, Alloc> items;
	};

	struct base_type {
		std::atomic<node_type *> node_ptr;
		node_type node;
	};

	node_type *alloc_node()
	{
		return nullptr;
	}

	std::pair<node_type *, size_t> get_next_pos()
	{
		std::pair<node_type *, size_t> rv(nullptr, 0);

		if (!base_ptr)
			return rv;

		while (true) {
			rv.first = base_ptr->node_ptr.load();
			rv.second = node->head.fetch_add(1);
			if (rv.second < BlockSize)
				break;
			else if (rv.second == BlockSize) {
				rv.first = alloc_node();
				rv.first->prev = node;
				rv.first->pos = 1;
				rv.second = 0;
				base_ptr->node_ptr = rv.first;
				break;
			}

			std::atomic_thread_fence(std::memory_order_seq_cst);
		};

		return rv;
	}

	counted_ptr<base_type> base_ptr;
};

}}}
#endif
