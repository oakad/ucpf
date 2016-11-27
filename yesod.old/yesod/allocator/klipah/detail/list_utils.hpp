/*
 * Copyright (c) 2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_96004F0BF4DAAFD15394AD4012D0326F)
#define HPP_96004F0BF4DAAFD15394AD4012D0326F

namespace ucpf { namespace yesod { namespace allocator { namespace klipah {
namespace detail {

template <typename Consumer, typename NodePtr, typename Traits>
void list_consume(Consumer &&cons, NodePtr head, Traits const &t)
{
	auto p(head);
	auto list_end(Traits::is_circular ? head : NodePtr{});

	do {
		auto q(t.next(p));
		cons(p);
		p = q;
	} while (p != list_end);
}

template <typename NodePtr, typename Traits>
std::size_t list_size(NodePtr head, Traits const &t)
{
	auto p(head);
	auto list_end(Traits::is_circular ? head : NodePtr{});
	std::size_t rv(0);

	do {
		++rv;
		p = t.next(p);
	} while (p != list_end);

	return rv;
}

template <typename NodePtr, typename Traits>
bool list_is_sorted(NodePtr head, Traits const &t)
{
	auto p(head);
	auto q(t.next(p));
	auto list_end(Traits::is_circular ? head : NodePtr{});

	while (q != list_end) {
		if (t.precedes(q, p))
			return false;

		p = t.next(p);
		q = t.next(q);
	}

	return true;
}

template <typename NodePtr, typename Traits>
NodePtr list_insertion_sort(NodePtr head, Traits const &t = Traits())
{
	auto list_end(Traits::is_circular ? head : NodePtr{});
	auto p(t.next(head));
	auto rv_begin(t.list_head(head));
	auto rv_end(first);
	auto rv_last(first);
	
	while (p != list_end) {
		auto q(t.next(p));

		if (t.precedes(rv_end, p)) {
			rv_last = rv_end;
			rv_end = t.link_after(rv_end, p);
		} else if (t.precedes(p, rv_begin)) {
			p = t.list_head(p);
			rv_last = t.link_after(p, rv_begin);
			rv_begin = p;
		} else {
			auto r(t.precedes(rv_last, p) ? rv_last : rv_begin);
			auto rr(t.next(r));

			while (t.precedes(rr, p)) {
				r = rr;
				rr = t.next(r);
			}

			rv_last = t.link_after(r, p);
		}

		
		p = q;
	}

	return rv_begin;
}

template <typename NodePtr, typename Traits>
NodePtr list_merge_sort(NodePtr head, Traits const &t = Traits())
{
	for (std::size_t m_size(1); true; m_size <<= 1) {
		std::size_t p_size(0);
		std::size_t q_size(m_size);
		auto p(head);
		auto q(head);
		NodePtr tail{};
		auto list_end(Traits::is_circular ? head : tail);

		do {
			++p_size;
			q = t.next(q);
			if (q == list_end)
				return head;
		} while (p_size < m_size);

		if (t.precedes(q, p)) {
			tail = q;
			q = t.next(q);
			--q_size;
		} else {
			tail = p;
			p = t.next(p);
			--p_size;
		}

		head = t.list_head(tail);

		while (true) {
			while (p_size && q_size && (q != list_end)) {
				if (t.precedes(q, p)) {
					auto qq(t.next(q));
					tail = t.link_after(tail, q);
					q = qq;
					--q_size;
				} else {
					auto pp(t.next(p));
					tail = t.link_after(tail, p);
					p = pp;
					--p_size;
				}
			}

			while (p_size) {
				auto pp(t.next(p));
				tail = t.link_after(tail, p);
				p = pp;
				--p_size;
			}

			while (q_size && (q != list_end)) {
				auto qq(t.next(q));
				tail = t.link_after(tail, q);
				q = qq;
				--q_size;
			}

			p = q;
			if (p == list_end)
				break;

			p_size = 0;
			do {
				++p_size;
				q = t.next(q);
			} while ((p_size != m_size) && (q != list_end));

			q_size = m_size;
		}
	}
}

}
}}}}

#endif
