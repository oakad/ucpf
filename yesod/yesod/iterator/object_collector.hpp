/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_YESOD_ITERATOR_OBJECT_COLLECTOR_20140616T1930)
#define UCPF_YESOD_ITERATOR_OBJECT_COLLECTOR_20140616T1930

#include <yesod/counted_ptr.hpp>
#include <yesod/allocator/array_helper.hpp>
#include <yesod/detail/placement_array.hpp>

namespace ucpf { namespace yesod { namespace iterator {

template <
	typename ValueType, size_t BlockSize,
	typename Alloc = std::allocator<void>
> struct object_collector : std::iterator<
	std::output_iterator_tag, ValueType, void, void, void
> {
	static_assert(BlockSize > 1, "BlockSize > 1");

	object_collector()
	{}

	object_collector(bool create, Alloc const &a = Alloc())
	: base_ptr(
		create ? allocate_counted<base_type>(a, a)
		       : counted_ptr<base_type>()
	)
	{}

	object_collector(object_collector const &other)
	: base_ptr(other.base_ptr)
	{}

	object_collector(object_collector &&other)
	: base_ptr(std::move(other.base_ptr))
	{}

	object_collector &operator=(object_collector const &other)
	{
		base_ptr = other.base_ptr;
		return *this;
	}

	object_collector &operator=(object_collector &&other)
	{
		base_ptr = std::move(other.base_ptr);
		return *this;
	}

	object_collector &operator=(ValueType const &value)
	{
		auto n_pos(get_next_pos());
		n_pos.cmd = alloc_cmd::COPY_VALUE;
		n_pos.value_ptr = const_cast<ValueType *>(&value);
		base_ptr.access_allocator(&n_pos);
		return *this;
	}

	object_collector &operator=(ValueType &&value)
	{
		auto n_pos(get_next_pos());
		n_pos.cmd = alloc_cmd::MOVE_VALUE;
		n_pos.value_ptr = &value;
		base_ptr.access_allocator(&n_pos);
		return *this;
	}

	object_collector &operator*()
	{
		return *this;
	}

	object_collector &operator++()
	{
		return *this;
	}

	object_collector &operator++(int)
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
		rv += std::min(l_pos, BlockSize);

		return rv;
	}

	template <typename Iterator>
	size_t copy(Iterator &&out, size_t n, size_t skip) const
	{
		if (!base_ptr)
			return 0;

		auto last(base_ptr->node_ptr.load());
		auto last_cnt(std::min(last->pos.load(), BlockSize));
		node_type *p(&base_ptr->first_node);

		for (; skip >= BlockSize; skip -= BlockSize) {
			if (p == last)
				return 0;
			p = p->next;
		}

		size_t rv(0);
		while (n && (p != last)) {
			auto cnt(std::min(BlockSize - skip, n));
			for (size_t c(0); c < cnt; ++c)
				*out++ = p->items[c + skip];

			rv += cnt;
			n -= cnt;
			skip = 0;
			p = p->next;
		}

		if (n && (p == last)) {
			if (skip >= last_cnt)
				return 0;
			auto cnt(std::min(last_cnt - skip, n));
			for (auto c(0); c < cnt; ++c)
				*out++ = p->items[c + skip];

			rv += cnt;
		}

		return rv;
	}

private:
	struct all_valid_pred {
		static bool test(ValueType const &v)
		{
			return true;
		}
	};

	struct node_type {
		template <typename Alloc1>
		node_type(Alloc1 const &a)
		: next(nullptr), pos(0), items(a)
		{}

		node_type *next;
		std::atomic<size_t> pos;
		yesod::detail::placement_array<
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
		template <typename Alloc1>
		base_type(Alloc1 const &a)
		: first_node(a), node_ptr(&first_node)
		{}

		template <typename Alloc1>
		static void destroy(Alloc1 &a, base_type *self)
		{
			auto p(self->first_node.next);

			while (p) {
				auto q(p);
				p = p->next;
				yesod::allocator::array_helper<
					node_type, Alloc1
				>::destroy(a, q, 1, true);
			}

			allocator::array_helper<
				base_type, Alloc1
			>::destroy(a, self, 1, false);
		}

		template <typename Alloc1>
		static void access_allocator(
			Alloc1 &a, base_type const *self, void *data
		)
		{
			auto cmd(reinterpret_cast<alloc_cmd *>(data));
			switch (cmd->cmd) {
			case alloc_cmd::ALLOC_NODE:
				cmd->node_ptr = allocator::array_helper<
					node_type, Alloc1
				>::alloc_n(a, 1, a);
				cmd->node_ptr->pos = 1;
				cmd->node_pos = 0;
				self->node_ptr.load()->next = cmd->node_ptr;
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

		mutable node_type first_node;
		mutable std::atomic<node_type *> node_ptr;
	};

	alloc_cmd get_next_pos()
	{
		alloc_cmd rv{
			.cmd = alloc_cmd::NOP,
			.node_ptr = nullptr,
			.node_pos = 0,
			.value_ptr = nullptr
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
