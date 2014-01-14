/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_YESOD_SPARSE_VECTOR_BASE_JAN_06_2014_1320)
#define UCPF_YESOD_SPARSE_VECTOR_BASE_JAN_06_2014_1320

#include <array>
#include <tuple>
#include <bitset>

namespace ucpf { namespace yesod {
namespace detail {

template <typename ValueType, size_t Order>
struct node_base {
};

template <typename ValueType, size_t Order>
struct cp_node_base : node_base<ValueType, Order> {
	std::bitset<size_t(1) << Order> stored;
	size_t count;
};

template <typename ValueType, size_t Order, size_t CompOrd>
struct cp_node : cp_node_base<ValueType, Order> {
	static node_base *construct();
	static void destroy(node_base *p);

	ValueType items[size_t(1) << CompOrd];
};



}

template <
	typename ValueType, typename Policy
> struct sparse_vector {
	typedef ValueType value_type;

	typedef typename std::allocator_traits<
		typename Policy::allocator_type
	>::template rebind_alloc<value_type> allocator_type;

	typedef typename std::allocator_traits<
		typename Policy::allocator_type
	>::template rebind_traits<value_type> allocator_traits;


	typedef typename allocator_type::reference reference;
	typedef typename allocator_type::const_reference const_reference;
	typedef typename allocator_traits::pointer pointer;
	typedef typename allocator_traits::const_pointer const_pointer;
	typedef typename allocator_traits::void_pointer void_pointer;
	typedef typename allocator_traits::size_type size_type;

	sparse_vector()
	: root_node(nullptr, allocator_type()), height(0)
	{}

	~sparse_vector()
	{
		clear();
	}

	void swap(sparse_vector &other)
	{
		std::swap(height, other.height);

		if (allocator_traits::propagate_on_container_swap::value)
			std::swap(root_node, other.root_node);
		else
			std::swap(
				std::get<0>(root_node),
				std::get<0>(other.root_node)
			);
	}

	void clear()
	{
		destroy_ptr_node(std::get<0>(root_node), height);
		std::get<0>(root_node) = nullptr;
		height = 0;
	}

	bool empty() const
	{
		return !height;
	}

	template <typename Pred>
	void remove_if(Pred pred);

	reference operator[](size_type pos)
	{
		auto &node(*data_node_at(pos));
		return node[node_offset(pos, 1)];
	}

	const_reference operator[](size_type pos) const
	{
		auto &node(*data_node_at(pos));
		return node[node_offset(pos, 1)];
	}

	reference at(size_type pos)
	{
		auto node(data_node_alloc_at(pos));
		return node->at(
			node_offset(pos, 1), std::get<1>(root_node)
		);
	}

	template <typename... Args>
	reference emplace_at(size_type pos, Args&&... args)
	{
		auto node(data_node_alloc_at(pos));
		return node->emplace_at(
			node_offset(pos, 1), std::get<1>(root_node),
			std::forward<Args>(args)...
		);
	}

	allocator_type get_allocator() const
	{
		return std::get<1>(root_node);
	}

private:
	struct data_node_pod;
	struct data_node_obj;

	typedef typename std::aligned_storage<
		sizeof(ValueType), std::alignment_of<ValueType>::value
	>::type value_storage_type;

	typedef typename std::conditional<
		std::is_pod<ValueType>::value, data_node_pod, data_node_obj
	>::type data_node;

	std::tuple<void_pointer, allocator_type> root_node;
	size_type height;

	struct data_node_pod {
		typedef typename sparse_vector::allocator_traits
		::template rebind_alloc<data_node_pod> node_allocator_type;

		typedef typename sparse_vector::allocator_traits
		::template rebind_traits<data_node_pod> node_allocator_traits;

		data_node_pod()
		{}

		data_node_pod(allocator_type &a)
		{
			for (auto &p: items)
				allocator_traits::construct(a, &p);
		}

		reference operator[](size_type pos)
		{
			return reinterpret_cast<reference>(items[pos]);
		}

		const_reference operator[](size_type pos) const
		{
			return reinterpret_cast<const_reference>(items[pos]);
		}

		reference at(size_type pos, allocator_type &a)
		{
			return reinterpret_cast<reference>(items[pos]);
		}

		template <typename... Args>
		reference emplace_at(
			size_type pos, allocator_type &a, Args&&... args
		)
		{
			auto p(&(*this)[pos]);
			allocator_traits::construct(
				a, p, std::forward<Args>(args)...
			);
			return *p;
		}

		void destroy(allocator_type &a)
		{}

		std::array<
			value_storage_type, (1UL << Policy::data_node_order)
		> items;
	};

	struct data_node_obj : data_node_pod {
		typedef typename sparse_vector::allocator_traits
		::template rebind_alloc<data_node_obj> node_allocator_type;

		typedef typename sparse_vector::allocator_traits
		::template rebind_traits<data_node_obj> node_allocator_traits;

		data_node_obj(allocator_type &a)
		: data_node_pod()
		{
		}

		reference at(size_type pos, allocator_type &a)
		{
			if (!value_set.test(pos)) {
				allocator_traits::construct(
					a, reinterpret_cast<pointer>(
						&data_node_pod::items[pos]
					)
				);
				value_set.set(pos);
			}

			return reinterpret_cast<reference>(
				data_node_pod::items[pos]
			);
		}

		template <typename... Args>
		reference emplace_at(
			size_type pos, allocator_type &a, Args&&... args
		);
		void destroy(allocator_type &a);

		std::bitset<1UL << Policy::data_node_order> value_set;
	};

	struct ptr_node {
		typedef typename sparse_vector::allocator_traits
		::template rebind_alloc<ptr_node> node_allocator_type;

		typedef typename sparse_vector::allocator_traits
		::template rebind_traits<ptr_node> node_allocator_traits;

		ptr_node()
		{
			std::fill(items.begin(), items.end(), nullptr);
		}

		std::array<void_pointer, (1UL << Policy::ptr_node_order)> items;
	};



	static size_type node_offset(size_type pos, size_type h)
	{
		auto l_pos(pos & ((1UL << Policy::data_node_order) - 1));
		if (h == 1)
			return l_pos;

		pos >>= Policy::data_node_order;
		return (pos >> (Policy::ptr_node_order * (h - 2)))
		       & ((1UL << Policy::ptr_node_order) - 2);
	}

	data_node *make_data_node();
	void destroy_data_node(void_pointer raw_node);
	ptr_node *make_ptr_node();
	void destroy_ptr_node(void_pointer raw_node, size_type height);
	data_node *data_node_at(size_type pos);
	data_node const *data_node_at(size_type pos) const;
	data_node *data_node_alloc_at(size_type pos);
};

}}
#endif
