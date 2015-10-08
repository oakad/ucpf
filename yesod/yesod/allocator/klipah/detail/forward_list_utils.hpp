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

template <typename NodePtr, typename Traits>
NodePtr forward_list_sort(NodePtr head, Traits const &t = Traits())
{
	for (std::size_t m_size(1); true; m_size <<= 1) {
		std::size_t p_size;
		std::size_t q_size(m_size);
		auto p(head);
		auto q(head);
		NodePtr tail;

		for (p_size = 0; p_size < m_size; ++p_size) {
			q = t.next(q);
			if (!q)
				return head;
		}

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
			while (p_size && q_size && q) {
				if (t.precedes(q, p)) {
					tail = t.link_after(tail, q);
					q = t.next(q);
					--q_size;
				} else {
					tail = t.link_after(tail, p);
					p = t.next(p);
					--p_size;
				}
			}

			while (p_size) {
				tail = t.link_after(tail, p);
				p = t.next(p);
				--p_size;
			}

			while (q_size && q) {
				tail = t.link_after(tail, q);
				q = t.next(q);
				--q_size;
			}

			p = q;
			if (!p)
				break;

			for (p_size = 0; p_size < m_size; ++p_size) {
				q = t.next(q);
				if (!q)
					break;
			}
			q_size = m_size;
		}
	}
}

}
}}}}

#endif
