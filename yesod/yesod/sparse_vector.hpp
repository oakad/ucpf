/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_YESOD_SPARSE_VECTOR_JAN_06_2014_1320)
#define UCPF_YESOD_SPARSE_VECTOR_JAN_06_2014_1320

#include <array>
#include <tuple>
#include <bitset>

namespace ucpf { namespace yesod {

struct default_sparse_vector_policy {
	typedef std::allocator<void> allocator_type;
	constexpr static size_t ptr_node_order = 6;
	constexpr static size_t data_node_order = 6;
};

template <
	typename ValueType, typename Policy = default_sparse_vector_policy
> struct sparse_vector {
	typedef ValueType value_type;

	typedef typename std::allocator_traits<
		typename Policy::allocator_type
	>::template rebind_alloc<value_type> allocator_type;

	typedef typename std::allocator_traits<
		typename Policy::allocator_type
	>::template rebind_traits<value_type> allocator_traits_type;


	typedef typename allocator_type::reference reference;
	typedef typename allocator_type::const_reference const_reference;
	typedef typename allocator_traits_type::size_type size_type;

	sparse_vector()
	: root_node(nullptr, allocator_type()), size(0), height(0)
	{}

	reference operator[](size_type pos)
	{
		auto p(data_node_at(pos));
		return reinterpret_cast<reference>(
			p->items[node_offset(pos, 1)]
		);
	}

	const_reference operator[](size_type pos) const
	{
		auto p(data_node_at(pos));
		return reinterpret_cast<const_reference>(
			p->items[node_offset(pos, 1)]
		);
	}

	void push_back(value_type const &v)
	{
		auto p(data_node_alloc_at(size));
		allocator_traits_type::construct(
			std::get<1>(root_node),
			p->get_raw(
				std::get<1>(root_node),
				node_offset(size, 1)
			), v
		);
		++size;
	}

private:
	typedef typename std::aligned_storage<
		sizeof(ValueType), std::alignment_of<ValueType>::value
	>::type value_storage_type;

	std::tuple<void *, allocator_type> root_node;
	size_type size;
	size_type height;

	struct data_node_triv {
		value_type *get_raw(allocator_type &a, size_type pos)
		{
			return reinterpret_cast<value_type *>(&items[pos]);
		}

		std::array<
			value_storage_type, (1UL << Policy::data_node_order)
		> items;
	};

	struct data_node_obj : data_node_triv {
		value_type *get_raw(allocator_type &a, size_type pos)
		{
			auto rv(reinterpret_cast<value_type *>(
				&data_node_triv::items[pos]
			));

			if (item_init.test(pos)) {
				allocator_traits_type::destroy(a, rv);
				item_init.reset(pos);
			}
			return rv;
		}

		std::bitset<(1UL << Policy::data_node_order)> item_init;
	};

	typedef typename std::conditional<
		std::is_trivial<value_type>::value,
		data_node_triv, data_node_obj
	>::type data_node;

	data_node *make_data_node()
	{
		typename allocator_traits_type::template rebind_alloc<
			data_node
		> node_alloc(
			std::get<1>(root_node)
		);
		typedef typename allocator_traits_type::template rebind_traits<
			data_node
		> alloc_traits;

		auto rv(alloc_traits::allocate(node_alloc, 1));
		alloc_traits::construct(node_alloc, rv);
		return rv;
	}

	struct ptr_node {
		std::array<void *, (1UL << Policy::ptr_node_order)> items;
	};

	ptr_node *make_ptr_node()
	{
		typename allocator_traits_type::template rebind_alloc<
			ptr_node
		> node_alloc(std::get<1>(root_node));

		typedef typename allocator_traits_type::template rebind_traits<
			ptr_node
		> alloc_traits;

		auto rv(alloc_traits::allocate(node_alloc, 1));
		alloc_traits::construct(node_alloc, rv);
		std::fill(rv->items.begin(), rv->items.end(), nullptr);
		return rv;
	}

	static size_type node_offset(size_type pos, size_type h)
	{
		auto l_pos(pos & ((1UL << Policy::data_node_order) - 1));
		if (h == 1)
			return l_pos;

		pos >>= Policy::data_node_order;
		return (pos >> (Policy::ptr_node_order * (h - 2)))
		       & ((1UL << Policy::ptr_node_order) - 2);
	}

	data_node *data_node_at(size_type pos)
	{
		auto h(height);
		void *p(std::get<0>(root_node));

		while (h > 1) {
			auto q(reinterpret_cast<ptr_node *>(p));
			p = q->items[node_offset(pos, h)];
			if (!p)
				return nullptr;
			--h;
		}
		return reinterpret_cast<data_node *>(p);
	}

	data_node const *data_node_at(size_type pos) const
	{
		auto h(height);
		void const *p(std::get<0>(root_node));

		while (h > 1) {
			auto q(reinterpret_cast<ptr_node const *>(p));
			p = q->items[node_offset(pos, h)];
			if (!p)
				return nullptr;
			--h;
		}
		return reinterpret_cast<data_node const *>(p);
	}

	data_node *data_node_alloc_at(size_type pos)
	{
		size_type p_height(1);
		for (
			auto p_pos(pos >> Policy::data_node_order);
			p_pos; p_pos >>= Policy::ptr_node_order
		)
			++p_height;

		void **rr(&std::get<0>(root_node));
		if (!height) {
			while (p_height > 1) {
				auto p(make_ptr_node());
				*rr = p;
				rr = &p->items[node_offset(pos, p_height)];
				--p_height;
				++height;
			}

			auto p(make_data_node());
			*rr = p;
			height = 1;
			return p;
		}

		while (p_height > height) {
			auto p(make_ptr_node());
			p->items[0] = std::get<0>(root_node);
			std::get<0>(root_node) = p;
			++height;
		}

		while (p_height > 1) {
			auto p(reinterpret_cast<ptr_node *>(*rr));
			*rr = &p->items[node_offset(pos, p_height)];
			if (!*rr)
				*rr = make_ptr_node();

			--p_height;
		}

		if (!*rr)
			*rr = make_data_node();

		return reinterpret_cast<data_node *>(*rr);
	}
};

}}
#endif
