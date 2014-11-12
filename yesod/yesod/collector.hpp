/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_YESOD_COLLECTOR_20141107T1800)
#define UCPF_YESOD_COLLECTOR_20141107T1800

#include <yesod/iterator/facade.hpp>
#include <yesod/allocator/array_helper.hpp>

namespace ucpf { namespace yesod {

template <
	typename T, std::size_t BlockCount, bool EmbedFirst = false,
	typename Alloc = std::allocator<void>
> struct collector {
	typedef T value_type;

	typedef typename std::allocator_traits<
		Alloc
	>::template rebind_alloc<value_type> allocator_type;

	typedef typename std::allocator_traits<
		Alloc
	>::template rebind_traits<value_type> allocator_traits;

	typedef typename allocator_type::reference reference;
	typedef typename allocator_type::const_reference const_reference;
	typedef typename allocator_traits::pointer pointer;
	typedef typename allocator_traits::const_pointer const_pointer;
	typedef typename allocator_traits::size_type size_type;

	collector(Alloc const &a = Alloc())
	: tup_head_pos_alloc_emb(nullptr, 0, a, node_base{})
	{
		node *head(nullptr);

		if (EmbedFirst)
			head = static_cast<node *>(&std::get<3>(
				tup_head_pos_alloc_emb
			));
		else
			head = node::make(std::get<2>(tup_head_pos_alloc_emb));

		std::get<0>(tup_head_pos_alloc_emb) = head;
	}

	~collector()
	{
		clear();
		if (!EmbedFirst)
			node::destroy(
				std::get<2>(tup_head_pos_alloc_emb),
				std::get<0>(tup_head_pos_alloc_emb)
			);
	}

	void clear()
	{
		auto &a(std::get<2>(tup_head_pos_alloc_emb));
		auto &head(std::get<0>(tup_head_pos_alloc_emb));
		auto cnt(std::get<1>(tup_head_pos_alloc_emb));
		std::get<1>(tup_head_pos_alloc_emb) = 0;

		while (true) {
			allocator_helper_type::destroy(
				a, head->items, cnt, false
			);
			cnt = BlockCount;

			if (head->next == head)
				return;

			head->prev->next = head->next;
			head->next->prev = head->prev;
			head = head->prev;
		}
	}

	template <typename... Args>
	void emplace_back(Args&&... args)
	{
		auto &head(std::get<0>(tup_head_pos_alloc_emb));
		auto &node_pos(std::get<1>(tup_head_pos_alloc_emb));
		auto &a(std::get<2>(tup_head_pos_alloc_emb));

		if (node_pos == (BlockCount - 1)) {
			auto p(node::make(a));
			p->prev = head;
			p->next = head->next;
			head->next = p;
			p->next->prev = p;
			allocator_helper_type::make(
				a, &head->items[node_pos],
				std::forward<Args>(args)...
			);

			head = p;
			node_pos = 0;
		} else
			allocator_helper_type::make(
				a, &head->items[node_pos++],
				std::forward<Args>(args)...
			);
	}

	void push_back(value_type const &v)
	{
		emplace_back(v);
	}

	void push_back(value_type &&v)
	{
		emplace_back(std::forward<value_type>(v));
	}

	reference back()
	{
		auto head(std::get<0>(tup_head_pos_alloc_emb));
		auto node_pos(std::get<1>(tup_head_pos_alloc_emb));

		if (node_pos)
			return reinterpret_cast<reference>(
				head->items[node_pos - 1]
			);
		else
			return reinterpret_cast<reference>(
				head->prev->items[BlockCount - 1]
			);
	}

	const_reference back() const
	{
		auto head(std::get<0>(tup_head_pos_alloc_emb));
		auto node_pos(std::get<1>(tup_head_pos_alloc_emb));

		if (node_pos)
			return reinterpret_cast<reference>(
				head->items[node_pos - 1]
			);
		else
			return reinterpret_cast<reference>(
				head->prev->items[BlockCount - 1]
			);
	}

	void pop_back()
	{
		auto &head(std::get<0>(tup_head_pos_alloc_emb));
		auto &node_pos(std::get<1>(tup_head_pos_alloc_emb));
		auto &a(std::get<2>(tup_head_pos_alloc_emb));

		if (node_pos) {
			--node_pos;
			allocator_helper_type::destroy(
				a, &head->items[node_pos], 1, false
			);
		} else if (head != head->prev) {
			auto p(head);
			head = head->prev;
			head->next = p->next;
			p->next->prev = head;
			node_pos = BlockCount - 1;
			allocator_helper_type::destroy(
				a, &head->items[node_pos], 1, false
			);
			node::destroy(a, p);
		}
	}

	size_type size() const
	{
		auto head(std::get<0>(tup_head_pos_alloc_emb));
		auto rv(std::get<1>(tup_head_pos_alloc_emb));
		for (auto p(head->prev); p != head; p = p->prev)
			rv += BlockCount;

		return rv;
	}

	bool empty() const
	{
		auto head(std::get<0>(tup_head_pos_alloc_emb));
		auto node_pos(std::get<1>(tup_head_pos_alloc_emb));
		return !node_pos && (head == head->prev);
	}

private:
	typedef allocator::array_helper<
		value_type, Alloc
	> allocator_helper_type;

	struct node_base {
	};

	struct node : node_base {
		static node *make(allocator_type const &a)
		{
			typedef allocator::array_helper<node, Alloc> a_h;

			return a_h::alloc(a);
		}

		static void destroy(allocator_type const &a, node *p)
		{
			typedef allocator::array_helper<node, Alloc> a_h;

			return a_h::destroy(a, p, 1, true);
		}

		node(node_base dummy = node_base{})
		: next(this), prev(this)
		{}

		node *next;
		node *prev;
		allocator::aligned_storage_t<value_type> items[BlockCount];
	};

	std::tuple<
		node *, size_type, allocator_type,
		typename std::conditional<
			EmbedFirst, node, node_base
		>::type
	> tup_head_pos_alloc_emb;

public:
	template <bool MakeConst>
	struct iterator_base : iterator::facade<
		iterator_base<MakeConst>,
		typename std::conditional<
			MakeConst, value_type const, value_type
		>::type,
		std::bidirectional_iterator_tag,
		typename std::conditional<
			MakeConst, const_reference, reference
		>::type
	> {

		iterator_base()
		: n(nullptr), pos(0)
		{}

	private:
		friend struct iterator::core_access;
		friend struct collector;

		template <bool OtherConst>
		bool equal(iterator_base<OtherConst> const &other) const
		{
			return (n == other.n) && (pos == other.pos);
		}

		void increment()
		{
			++pos;
			if (pos >= BlockCount) {
				n = n->next;
				pos %= BlockCount;
			}
		}

		void decrement()
		{
			if (pos)
				--pos;
			else {
				n = n->prev;
				pos = BlockCount - 1;
			}
		}

		void advance(typename iterator_base::difference_type cnt)
		{
			if (n < 0)
				advance_back(-cnt);

			decltype(cnt) rem(BlockCount - pos);

			while (true) {
				if (rem > cnt) {
					pos += cnt;
					return;
				}

				cnt -= rem;
				pos = 0;
				rem = BlockCount;
				n = n->next;
			}
		}

		void advance_back(typename iterator_base::difference_type cnt)
		{
			decltype(cnt) rem(pos);

			while (true) {
				if (rem > cnt) {
					pos -= cnt;
					return;
				}

				cnt -= rem;
				pos = BlockCount - 1;
				rem = pos;
				n = n->prev;
			}
		}

		typename iterator_base::reference dereference() const
		{
			return reinterpret_cast<
				typename iterator_base::reference
			>(n->items[pos]);
		}

		iterator_base(node *n_, size_type pos_)
		: n(n_), pos(pos_)
		{}

		node *n;
		size_type pos;
	};

	typedef iterator_base<false> iterator;
	typedef iterator_base<true> const_iterator;

	iterator begin()
	{
		auto head(std::get<0>(tup_head_pos_alloc_emb));
		return iterator(head->next, 0);
	}

	const_iterator begin() const
	{
		auto head(std::get<0>(tup_head_pos_alloc_emb));
		return const_iterator(head->next, 0);
	}

	iterator end()
	{
		auto head(std::get<0>(tup_head_pos_alloc_emb));
		auto node_pos(std::get<1>(tup_head_pos_alloc_emb));
		return iterator(head, node_pos);
	}

	const_iterator end() const
	{
		auto head(std::get<0>(tup_head_pos_alloc_emb));
		auto node_pos(std::get<1>(tup_head_pos_alloc_emb));
		return const_iterator(head, node_pos);
	}
};

}}
#endif
