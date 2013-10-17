/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * Based on implementation from boost::lockfree library (http://www.boost.org/):
 * 
 *      Copyright (c) 2008-2013 Tim Blechmann
 * 
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_YESOD_STACK_OCT_16_2013_1130)
#define UCPF_YESOD_STACK_OCT_16_2013_1130

#include <atomic>

#include <boost/lockfree/detail/tagged_ptr.hpp>

#include <boost/intrusive/detail/utilities.hpp>
#include <boost/intrusive/member_value_traits.hpp>

namespace ucpf { namespace yesod {

struct stack_head {
	template <
		typename ValueType, typename HeadType,
		HeadType ValueType::* HeadPtr
	> friend struct stack;

	typedef stack_head       node;
	typedef stack_head       *node_ptr;
	typedef stack_head const *const_node_ptr;

	stack_head() : next(nullptr)
	{}

private:
	stack_head *next;
};

template <typename ValueType, typename HeadType, HeadType ValueType::* HeadPtr>
struct stack {
	typedef ValueType       value_type;
	typedef ValueType       &reference;
	typedef ValueType const &const_reference;
	typedef ValueType       *pointer;
        typedef ValueType const *const_pointer;

	typedef boost::intrusive::member_value_traits<
		value_type, HeadType, HeadPtr
	> member_value_traits;

	typedef typename boost::lockfree::detail::tagged_ptr<
		stack_head
	> tagged_head_ptr;

	stack() : head(tagged_head_ptr(nullptr, 0))
	{}

	void splice(stack &other)
	{
		auto other_head(other.head.load());
		stack_head *h(nullptr);

		while (true) {
			h = other_head.get_ptr();
			if (!h)
				return;

			tagged_head_ptr new_head(
				nullptr, /*old_head.get_next_tag()*/
				other_head.get_tag() + 1
			);

			if (other.head.compare_exchange_weak(
				other_head, new_head
			))
				break;
		}

		auto t(h);
		while (t->next)
			t = t->next;

		link_nodes_atomic(h, t);
	}

	bool empty() const
	{
		return head.load().get_ptr() == nullptr;
	}

	bool push(reference v)
	{
		stack_head *h(member_value_traits::to_node_ptr(v));

		link_nodes_atomic(h, h);
		return h->next == nullptr;
	}

	pointer pop()
	{
		auto old_head(head.load());
		while (true) {
			auto h(old_head.get_ptr());
			if (!h)
				return nullptr;
			tagged_head_ptr new_head(
				h->next, /*old_head.get_next_tag()*/
				old_head.get_tag() + 1
			);

			if (head.compare_exchange_weak(old_head, new_head))
				return member_value_traits::to_value_ptr(h);
		}
	}

private:
	void link_nodes_atomic(stack_head *top_node, stack_head *end_node)
	{
		auto old_head(head.load());
		while (true) {
			tagged_head_ptr new_head(top_node, old_head.get_tag());
			end_node->next = old_head.get_ptr();
			if (head.compare_exchange_weak(old_head, new_head))
				break;
		}
	}

	std::atomic<tagged_head_ptr> head;
};

}}

#endif
