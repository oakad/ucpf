/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_YESOD_STREAM_COLLECTOR_20141123T2300)
#define UCPF_YESOD_STREAM_COLLECTOR_20141123T2300

#include <yesod/iterator/facade.hpp>
#include <yesod/allocator/array_helper.hpp>

namespace ucpf { namespace yesod {

template <
	typename T, std::size_t BlockCount, typename Producer, typename Alloc
> struct stream_collector {
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

	
private:
	typedef allocator::array_helper<
		value_type, Alloc
	> allocator_helper_type;
	typedef allocator::aligned_storage_t<value_type> storage_type;

	struct node_base {
		node_base *next;
		unsigned long ref_count;
	};

	struct node {
		static node *make(allocator_type const &a)
		{
			typedef allocator::array_helper<node, Alloc> a_h;

			auto deleter = [a](node *p) -> void {
				a_h::destroy(a, p, 1, true);
			};

			std::unique_ptr<node, decltype(deleter)> rv(
				a_h::alloc(a), deleter
			);

			allocator_helper_type::make_n(
				a, rv->items, BlockCount
			);
			return rv.release();
		}

		static void destroy(allocator_type const &a, node *p)
		{
			typedef allocator::array_helper<node, Alloc> a_h;

			allocator_helper_type::destroy(
				a, p->items, BlockCount, false
			);

			a_h::destroy(a, p, 1, true);
		}

		node_base base;
		storage_type items[BlockCount];
	};

	size_type refill(node *last_node)
	{
		auto last_pos(std::get<0>(tup_pos_prod_alloc));
		auto &prod(std::get<1>(tup_pos_prod_alloc));
		auto &a(std::get<2>(tup_pos_prod_alloc));

		auto a_count(prod.available());
		if (!a_count)
			return 0;

		auto c_node(last_node);
		auto c_count(BlockCount - last_pos - 1);
		size_type x_count(0);

		auto deleter = [a](node *p) -> void {
			node::destroy(a, p);
		};

		std::unique_ptr<node, decltype(deleter)> new_node(
			nullptr, deleter
		);

		if (!c_count) {
			new_node.reset(node::make(a));
			c_node = new_node.get();
			last_pos = 0;
			c_count = BlockCount;
		} else
			++last_pos;

		c_count = std::min(c_count, a_count);
		a_count = prod.read(
			reinterpret_cast<value_type *>(
				c_node->items + last_pos
			), c_count
		);

		if (a_count) {
			std::get<0>(
				tup_pos_prod_alloc
			) = last_pos + a_count - 1;

			if (new_node) {
				new_node->base.next = &head.base;
				new_node->base.ref_count = 1;
				last_node->next = &new_node->base;
				new_node.release();
			}
		}

		c_node = head.base.next;

		while (c_node != last_node) {
			if (c_node->base.ref_count > 1)
				break;

			head.base.next = c_node->base.next;
			node::destroy(a, c_node);
			c_node = head.base.next;
		}

		return a_count;
	}

	struct head_type {
		node_base base;
		stream_collector *self;
	} head;

	std::tuple<
		size_type, Producer, allocator_type
	> tup_pos_prod_alloc;

public:
	struct iterator : iterator::facade<
		iterator, value_type, std::forward_iterator_tag, reference
        > {
		iterator()
		: item_ptr(nullptr), base_ptr(nullptr)
		{}

	private:
		bool equal(iterator const &other) const
		{
			return item_ptr == other.item_ptr;
		}

		void increment()
		{
			auto node_ptr(reinterpret_cast<node *>(base_ptr));
			auto next_base(base_ptr->next);

			auto pos(((
				item_ptr - node_ptr.items
			) / sizeof(storage_type)) + 1);

			if (!next_base->ref_cnt) {
				auto sc(reinterpret_cast<head_type *>(
					next_base
				)->self);

				auto last_pos(
					std::get<0>(sc->tup_pos_prod_alloc)
				);

				if ((pos > last_pos) && !sc->refill(node_ptr)) {
					--node_ptr->ref_count;
					item_ptr = nullptr;
					base_ptr = next_base;
					return;
				}
			}

			++item_ptr;

			if (pos == BlockCount) {
				next_base = base_ptr->next;
				--node_ptr->ref_count;
				node_ptr = reinterpret_cast<node *>(next_base);
				++node_ptr->ref_count;
				item_ptr = node_ptr->items;
				base_ptr = next_base;
			}
		}

		typename iterator_base::reference dereference() const
		{
			return reinterpret_cast<
				typename iterator_base::reference
			>(*item_ptr);
		}

		iterator(storage_type *item_ptr_, node_base *base_ptr_)
		: item_ptr(item_ptr_), base_ptr(base_ptr_)
		{}

		storage_type *item_ptr;
		node_base *base_ptr;
	};
};

}}

#endif
