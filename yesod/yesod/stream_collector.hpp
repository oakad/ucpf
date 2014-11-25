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

	stream_collector(Producer &prod, Alloc const &a)
	: head{{&head.base, 0}, this},
	  tup_eos_pos_prod_alloc(false, 0, prod, a)
	{
	}

	~stream_collector()
	{
		auto &a(std::get<3>(tup_eos_pos_prod_alloc));

		while (head.base.next != &head.base) {
			auto c_base(head.base.next);
			head.base.next = c_base->next;
			node::destroy(a, reinterpret_cast<node *>(c_base));
		}
	}

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

	void cull_head(node *last_node)
	{
		auto &a(std::get<3>(tup_eos_pos_prod_alloc));

		auto c_base(head.base.next);
		while (c_base != &last_node->base) {
			if (c_base->ref_count > 1)
				break;

			head.base.next = c_base->next;
			node::destroy(a, reinterpret_cast<node *>(c_base));
			c_base = head.base.next;
		}
	}

	size_type refill(node *last_node)
	{
		cull_head(last_node);

		if (std::get<0>(tup_eos_pos_prod_alloc))
			return 0;

		auto last_pos(std::get<1>(tup_eos_pos_prod_alloc));
		auto &prod(std::get<2>(tup_eos_pos_prod_alloc));
		auto &a(std::get<3>(tup_eos_pos_prod_alloc));

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

		auto a_count(prod(
			reinterpret_cast<value_type *>(
				c_node->items + last_pos
			), c_count
		));

		if (a_count) {
			std::get<1>(
				tup_eos_pos_prod_alloc
			) = last_pos + a_count - 1;

			if (new_node) {
				new_node->base.next = &head.base;
				new_node->base.ref_count = 1;
				last_node->base.next = &new_node->base;
				new_node.release();
			}
		} else
			std::get<0>(tup_eos_pos_prod_alloc) = true;

		return a_count;
	}

	struct head_type {
		node_base base;
		stream_collector *self;
	} head;

	std::tuple<
		bool, size_type, Producer &, allocator_type
	> tup_eos_pos_prod_alloc;

public:
	struct iterator : yesod::iterator::facade<
		iterator, value_type, std::forward_iterator_tag, reference
        > {
		iterator()
		: item_ptr(nullptr), base_ptr(nullptr)
		{}

		iterator(iterator const &other)
		: item_ptr(other.item_ptr), base_ptr(other.base_ptr)
		{
			if (item_ptr)
				++base_ptr->ref_count;
		}

		iterator(iterator &&other)
		: item_ptr(other.item_ptr), base_ptr(other.base_ptr)
		{
			other.item_ptr = nullptr;
		}

		iterator &operator=(iterator const &other)
		{
			if (item_ptr)
				--base_ptr->ref_count;

			item_ptr = other.item_ptr;
			base_ptr = other.base_ptr;

			if (item_ptr)
				++base_ptr->ref_count;
		}

		iterator &operator=(iterator &&other)
		{
			if (item_ptr)
				--base_ptr->ref_count;

			item_ptr = other.item_ptr;
			base_ptr = other.base_ptr;

			other.item_ptr = nullptr;
		}

		~iterator()
		{
			if (item_ptr)
				--base_ptr->ref_count;
		}

		void swap(iterator &other)
		{
			std::swap(item_ptr, other.item_ptr);
			std::swap(base_ptr, other.base_ptr);
		}

	private:
		friend struct yesod::iterator::core_access;
		friend struct stream_collector;

		bool equal(iterator const &other) const
		{
			return item_ptr == other.item_ptr;
		}

		void increment()
		{
			auto node_ptr(reinterpret_cast<node *>(base_ptr));
			auto next_base(base_ptr->next);

			size_type pos((item_ptr - node_ptr->items) + 1);

			if (!next_base->ref_count) {
				auto sc(reinterpret_cast<head_type *>(
					next_base
				)->self);

				auto last_pos(
					std::get<1>(sc->tup_eos_pos_prod_alloc)
				);

				if ((pos > last_pos) && !sc->refill(node_ptr)) {
					--node_ptr->base.ref_count;
					item_ptr = nullptr;
					base_ptr = next_base;
					return;
				}
			}

			++item_ptr;

			if (pos == BlockCount) {
				next_base = base_ptr->next;
				--node_ptr->base.ref_count;
				node_ptr = reinterpret_cast<node *>(next_base);
				++node_ptr->base.ref_count;
				item_ptr = node_ptr->items;
				base_ptr = next_base;
			}
		}

		typename iterator::reference dereference() const
		{
			return reinterpret_cast<
				typename iterator::reference
			>(*item_ptr);
		}

		iterator(node_base *base_ptr_)
		: item_ptr(nullptr), base_ptr(base_ptr_)
		{
		}

		iterator(size_type pos, node_base *base_ptr_)
		: item_ptr(reinterpret_cast<node *>(base_ptr_)->items + pos),
		  base_ptr(base_ptr_)
		{
		}

		storage_type *item_ptr;
		node_base *base_ptr;
	};

	iterator begin()
	{
		if (head.base.next == &head.base) {
			auto &prod(std::get<2>(tup_eos_pos_prod_alloc));
			auto &a(std::get<3>(tup_eos_pos_prod_alloc));

			auto deleter = [a](node *p) -> void {
				node::destroy(a, p);
			};

			std::unique_ptr<node, decltype(deleter)> new_node(
				node::make(a), deleter
			);

			auto a_count(prod(
				reinterpret_cast<pointer>(new_node->items),
				BlockCount
			));

			if (!a_count) {
				std::get<0>(tup_eos_pos_prod_alloc) = true;
				return iterator(&head.base);
			}

			std::get<1>(tup_eos_pos_prod_alloc) = a_count - 1;
			new_node->base.next = &head.base;
			new_node->base.ref_count = 2;
			head.base.next = &new_node->base;
			return iterator(0, &new_node.release()->base);
		} else {
			++head.base.next->ref_count;
			return iterator(0, head.base.next);
		}
	}

	iterator end()
	{
		return iterator(&head.base);
	}
};

}}

#endif
