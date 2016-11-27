/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_YESOD_ITERATOR_COLLECTOR_20140617T1645)
#define UCPF_YESOD_ITERATOR_COLLECTOR_20140617T1645

#include <yesod/allocator/array_helper.hpp>

namespace ucpf { namespace yesod { namespace iterator {

template <
	typename ValueType, size_t BlockSize,
	typename Alloc = std::allocator<void>
> struct collector : std::iterator<
	std::output_iterator_tag, ValueType, void, void, void
> {
	static_assert(BlockSize > 1, "BlockSize > 1");
	static_assert(
		std::is_pod<ValueType>::value,
		"std::is_pod<ValueType>::value"
	);

	collector()
	: base_ptr(nullptr, Alloc()), owner(false)
	{}

	collector(bool create, Alloc const &a = Alloc())
	: base_ptr(
		create ? std::make_tuple(base_type::make(a), a)
		       : std::make_tuple(nullptr, Alloc())
	), owner(create)
	{}

	collector(collector const &other)
	: base_ptr(other.base_ptr), owner(false)
	{}

	collector(collector &&other)
	: base_ptr(std::move(other.base_ptr)), owner(other.owner)
	{
		other.owner = false;
	}

	collector &operator=(collector const &other)
	{
		base_ptr = other.base_ptr;
		owner = false;
		return *this;
	}

	collector &operator=(collector &&other)
	{
		base_ptr = std::move(other.base_ptr);
		owner = other.owner;
		other.owner = false;
		return *this;
	}

	~collector()
	{
		if (owner) {
			reset();

			allocator::array_helper<base_type, Alloc>::destroy(
				std::get<1>(base_ptr), std::get<0>(base_ptr),
				1, true
			);
		}
	}

	collector &operator=(ValueType const &value)
	{
		auto n_pos(get_next_pos());
		n_pos.first->items[n_pos.second] = value;
		return *this;
	}

	collector &operator=(ValueType &&value)
	{
		auto n_pos(get_next_pos());
		n_pos.first->items[n_pos.second] = std::move(value);
		return *this;
	}

	collector &operator*()
	{
		return *this;
	}

	collector &operator++()
	{
		return *this;
	}

	collector &operator++(int)
	{
		return *this;
	}

	size_t size() const
	{
		size_t rv(0);
		if (!std::get<0>(base_ptr))
			return rv;

		auto last(std::get<0>(base_ptr)->node_ptr);
		for (
			auto p(&std::get<0>(base_ptr)->first_node);
			p != last; p = p->next
		)
			rv += BlockSize;

		auto l_pos(last->pos);
		rv += std::min(l_pos, BlockSize);

		return rv;
	}

	template <typename Iterator>
	size_t copy(Iterator &&out, size_t n, size_t skip) const
	{
		if (!std::get<0>(base_ptr))
			return 0;

		auto last(std::get<0>(base_ptr)->node_ptr);
		auto last_cnt(std::min(last->pos, BlockSize));
		node_type *p(&std::get<0>(base_ptr)->first_node);

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

	void reset()
	{
		auto b(std::get<0>(base_ptr));
		if (!b)
			return;

		auto p(b->first_node.next);

		while (p) {
			auto q(p);
			p = p->next;
			yesod::allocator::array_helper<
				node_type, Alloc
			>::destroy(std::get<1>(base_ptr), q, 1, true);
		}
		b->first_node.pos = 0;
		b->first_node.next = nullptr;
		b->node_ptr = &b->first_node;
	}

private:
	struct node_type {
		node_type()
		: next(nullptr), pos(0)
		{}

		node_type *next;
		size_t pos;
		ValueType items[BlockSize];
	};

	struct base_type {
		static base_type *make(Alloc const &a)
		{
			return allocator::array_helper<
				base_type, Alloc
			>::alloc_n(a, 1);
		}

		base_type()
		: node_ptr(&first_node)
		{}

		node_type first_node;
		node_type *node_ptr;
	};

	std::pair<node_type *, size_t> get_next_pos()
	{
		std::pair<node_type *, size_t> rv(nullptr, 0);
		auto b(std::get<0>(base_ptr));
		if (!b)
			return rv;

		rv.first = b->node_ptr;
		rv.second = rv.first->pos++;
		if (rv.second == BlockSize) {
			node_type *p(allocator::array_helper<
				node_type, Alloc
			>::alloc_n(std::get<1>(base_ptr), 1));
			b->node_ptr->next = p;
			b->node_ptr = p;
			p->pos = 1;
			rv.first = p;
			rv.second = 0;
		}
		return rv;
	}

	std::tuple<base_type *, Alloc> base_ptr;
	bool owner;
};

}}}
#endif

