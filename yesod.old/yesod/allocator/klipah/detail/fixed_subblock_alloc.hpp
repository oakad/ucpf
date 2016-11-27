/*
 * Copyright (c) 2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_61F2F54C2244F2056AFAD24CB06D3133)
#define HPP_61F2F54C2244F2056AFAD24CB06D3133

#include <mutex>

namespace ucpf { namespace yesod { namespace allocator { namespace klipah {
namespace detail {

template <typename Policy, std::size_t BlockSize, std::size_t MacroBlockSize>
struct fixed_subblock_alloc : Policy::object_manager_type {
	typedef typename Policy::block_type::template rebind<
		BlockSize
	> block_type;
	typedef typename Policy::block_type::template rebind<
		MacroBlockSize
	> macro_block_type;
	typedef typename std::pointer_traits<
		typename Policy::pointer
	>::template rebind<block_type> block_pointer;
	typedef typename std::pointer_traits<
		typename Policy::pointer
	>::template rebind<macro_block_type> macro_block_pointer;
	typedef typename Policy::mutex_type mutex_type;
	typedef typename Policy::object_manager_type object_manager_type;

	constexpr static std::size_t blocks_per_macro_block
	= macro_block_type::storage_size / sizeof(block_type);

	static_assert(
		blocks_per_macro_block > 1,
		"blocks_per_macro_block > 1"
	);
	static_assert(
		alignof(block_type) <= alignof(macro_block_type),
		"alignof(block_type) <= alignof(macro_block_type)"
	);
	static_assert(
		alignof(block_pointer) <= alignof(block_type),
		"alignof(block_pointer) <= alignof(block_type)"
	);

	fixed_subblock_alloc(
		object_manager_type const &mngr = object_manager_type{}
	) : object_manager_type(mngr)
	{
		this->init_mutex(block_lock);
	}

	template <typename MacroBlockConsumer>
	void compact(MacroBlockConsumer &&c)
	{

	}

	template <typename MacroBlockProvider>
	block_pointer allocate(MacroBlockProvider &&p)
	{
		std::unique_lock<mutex_type> lg(block_lock);
		if (free_list) {
			auto p(free_list);
			auto q(free_list->template get<block_pointer>(0, 1));
			free_list = *q;
			p->template erase<block_pointer>(0, 1);
			++blocks_allocated;
			return p;
		}

		macro_block_pointer mp(p.get());
		if (!mp)
			return block_pointer();

		++macro_blocks_provisioned;
		std::size_t pos(1);

		auto p(this->template wrap_pointer<block_pointer>(
			mp->template subblock_ptr<BlockSize>(0)
		));

		if (used_list && (
			current_used_offset < used_list_node::item_count
		)) {
			auto r(used_list->template get<used_list_node>(0, 1));
			r->items[current_used_offset++] = mp;
		} else {
			auto q(this->template wrap_pointer<block_pointer>(
				mp->template subblock_ptr<BlockSize>(1)
			));
			++pos;
			auto r(q->template construct<used_list_node>(0, 1));
			r->next = used_list;
			r->items[0] = mp;
			current_used_offset = 1;
			used_list = q;
		}

		for (std::size_t c(pos); c < blocks_per_macro_block; ++c) {
			auto q(this->template wrap_pointer<block_pointer>(
				mp->template subblock_ptr<BlockSize>(c)
			);
			q->template construct<block_pointer>(0, 1, free_list);
		}

		++blocks_allocated;
		return p;
	}

	void deallocate(block_pointer p)
	{
		std::unique_lock<mutex_type> lg(block_lock);
		p->template construct<block_pointer>(0, 1, free_list);
		free_list = p;
		--blocks_allocated;
	}

private:
	struct used_list_node {
		constexpr static std::size_t item_count
		= (block_type::storage_size - sizeof(block_pointer))
		  / sizeof(macro_block_pointer);

		static_assert(item_count > 0, "item_count > 0");

		block_pointer next;
		macro_block_pointer items[item_count];
	};

	mutex_type block_lock;
	block_pointer free_list;
	block_pointer used_list;
	std::size_t macro_blocks_provisioned = 0;
	std::size_t blocks_allocated = 0;
	std::size_t current_used_offset = 0;
};

}
}}}}
#endif
