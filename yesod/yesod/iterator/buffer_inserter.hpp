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

	buffer_inserter()
	{}

	buffer_inserter(bool create, Alloc const &a = Alloc())
	: base_ptr(create ? allocate_counted(a) : counted_ptr<base_type>())
	{}

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

	typename std::enable_if<
		!std::is_pod<ValueType>::value, buffer_inserter &
	>::type operator=(ValueType const &value)
	{
		auto n_pos(get_next_pos());
		n_pos.cmd = COPY_VALUE;
		n_pos.value_ptr = const_cast<ValueType *>(&value);
		base_ptr.access_allocator(&n_pos);
		return *this;
	}

	typename std::enable_if<
		!std::is_pod<ValueType>::value, buffer_inserter &
	>::type operator=(ValueType &&value)
	{
		auto n_pos(get_next_pos());
		n_pos.cmd = MOVE_VALUE;
		n_pos.value_ptr = &value;
		base_ptr.access_allocator(&n_pos);
		return *this;
	}

	typename std::enable_if<
		std::is_pod<ValueType>::value, buffer_inserter &
	>::type operator=(ValueType const &value)
	{
		auto n_pos(get_next_pos());
		*n_pos.node_ptr->items.ptr_at(n_pos.node_pos) = value;
		return *this;
	}

	typename std::enable_if<
		std::is_pod<ValueType>::value, buffer_inserter &
	>::type operator=(ValueType &&value)
	{
		auto n_pos(get_next_pos());
		*n_pos.node_ptr->items.ptr_at(
			n_pos.node_pos
		) = std::move(value);
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

	size_t size() const
	{
		size_t rv(0);
		if (!base_ptr)
			return rv;

		auto last(base_ptr->node_ptr.load());
		for (auto p(&base_ptr->first_node); p != last; p = p->next)
			rv += BlockSize;

		auto l_pos(last->pos.load());
		if (l_pos)
			rv += l_pos - 1;

		return rv;
	}

	template <typename Iterator>
	void copy(Iterator &first, Iterator last) const
	{
	}

	template <typename Iterator>
	void copy_n(Iterator &first, size_t n) const
	{
	}

private:
	struct all_valid_pred {
		static bool test(ValueType const &v)
		{
			return true;
		}
	};

	struct node_type {
		node_type()
		: next(nullptr), pos(0)
		{}

		node *next;
		std::atomic<size_t> pos;
		placement_array<
			ValueType, BlockSize, Alloc, all_valid_pred
		> items;
	};

	struct alloc_cmd {
		enum {
			NOP = 0,
			ALLOC_NODE,
			COPY_VALUE,
			MOVE_VALUE
		} cmd;
		node_type *node_ptr;
		size_t node_pos;
		ValueType *value_ptr;
	};

	struct base_type {
		base_type()
		: node_ptr(&first_node), tail(&first_node)
		{}

		node_type *alloc_node()
		{
		}

		template <typename Alloc1>
		static node_type *alloc_node(Alloc1 &a)
		{
		}

		template <typename Alloc1>
		static void access_allocator(
			Alloc1 &a, base_type const *self, void *data
		)
		{
			auto cmd(reinterpret_cast<alloc_cmd *>(data));
			switch (cmd->cmd) {
			case alloc_cmd::ALLOC_NODE:
				cmd->node_ptr = alloc_node(a);
				cmd->node_ptr->pos = 1;
				cmd->node_pos = 0;
				self->tail->next = cmd->node_ptr;
				self->tail = cmd->node_ptr;
				self->node_ptr = cmd->node_ptr;
				break;
			case alloc_cmd::COPY_VALUE:
				cmd->node_ptr->items.emplace_at(
					cmd->node_pos, a,
					*const_cast<ValueType const *>(
						cmd->value_ptr
					)
				);
				break;
			case alloc_cmd::MOVE_VALUE:
				cmd->node_ptr->items.emplace_at(
					cmd->node_pos, a,
					std::move(*cmd->value_ptr)
				);
				break;
			default:
				break;
			}
		}

		mutable std::atomic<node_type *> node_ptr;
		mutable node_type *tail;
		mutable node_type first_node;
	};

	alloc_cmd get_next_pos()
	{
		alloc_cmd rv{
			.cmd = alloc_cmd::NOP,
			.node_ptr = nullptr,
			.node_pos = 0
		};

		if (!base_ptr)
			return rv;

		/* It is somewhat unlikely that multiple threads will be
		 * simultaneously writing to the same buffer, but such use
		 * case is supported and the below code may need to be
		 * improved if found inadequate.
		 */
		while (true) {
			rv.node_ptr = base_ptr->node_ptr.load();
			rv.node_pos = rv.node_ptr->pos.fetch_add(1);
			if (rv.node_pos > BlockSize)
				sched_yield();
			else
				break;
		};

		if (rv.node_pos == BlockSize) {
			rv.cmd = alloc_cmd::ALLOC_NODE;
			base_ptr.access_allocator(&rv);
		}

		return rv;
	}

	counted_ptr<base_type> base_ptr;
};

}}}
#endif
