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

namespace ucpf { namespace yesod {

template <typename ValueType>
struct default_sparse_vector_policy {
	typedef typename std::aligned_storage<
		sizeof(ValueType), std::alignment_of<ValueType>::value
	>::type value_storage_type;

	typedef std::allocator<ValueType> allocator_type;

	constexpr static size_t ptr_node_order = 6;
	constexpr static size_t data_node_order = 6;
};

template <
	typename ValueType,
	typename Policy = default_sparse_vector_policy<ValueType>
> struct sparse_vector {
	typedef ValueType value_type;
	typedef typename Policy::allocator_type allocator_type;
	typedef std::allocator_traits<allocator_type> allocator_traits_type;
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
			std::get<allocator_type>(root_node),
			p->items[node_offset(size, 1)],
			v
		);
		++size;
	}

	template <typename CharType, typename TraitsType>
	static std::basic_ostream<CharType, TraitsType> &dump(
		sparse_vector const &r,
		std::basic_ostream<CharType, TraitsType> &os,
		int indent = 0
	)
	{
		std::ostream_iterator<
			CharType, CharType, TraitsType
		> out_iter(os);

		std::fill_n(out_iter, indent, os.fill());

		auto rr(std::get<void *>(r.root_node));

		if (!rr) {
			os << "null\n";
			return os;
		}
		
        }

private:
	std::tuple<void *, allocator_type> root_node;
	size_type size;
	size_type height;

	struct data_node {
		static data_node *make(decltype(root_node) &r)
		{
			typename allocator_traits_type::template rebind_alloc<
				data_node
			>::other node_alloc(
				std::get<allocator_type>(r)
			);
			typedef typename allocator_traits_type
			::template rebind_traits<
				decltype(node_alloc)
			> alloc_traits;

			auto rv(alloc_traits::allocate(node_alloc, 1));
			alloc_traits::construct(node_alloc, rv);
			return rv;
		}

		std::array<
			typename Policy::value_storage_type,
			(1UL << Policy::data_node_order)
		> items;
	};

	struct ptr_node {
		static ptr_node *make(decltype(root_node) &r)
		{
			typename allocator_traits_type::template rebind_alloc<
				ptr_node
			>::other node_alloc(std::get<allocator_type>(r));

			typedef typename allocator_traits_type
			::template rebind_traits<
				decltype(node_alloc)
			> alloc_traits;

			auto rv(alloc_traits::allocate(node_alloc, 1));
			alloc_traits::construct(node_alloc, rv);
			std::fill(rv->items.begin(), rv->items.end(), nullptr);
			return rv;
		}

		std::array<void *, (1UL << Policy::ptr_node_order)> items;
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

	data_node const *data_node_at(size_type pos) const
	{
		auto h(height);
		void const *p(std::get<void *>(root_node));

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

		void **rr(&std::get<void *>(root_node));

		if (!height) {
			while (p_height > 1) {
				auto p(ptr_node::make(root_node));
				*rr = p;
				rr = &p->items[node_offset(pos, p_height)];
				--p_height;
				++height;
			}

			auto p(data_node::make(root_node));
			*rr = p;
			return p;
		}

		while (p_height > height) {
			auto p(ptr_node::make(root_node));
			p->items[0] = std::get<void *>(root_node);
			std::get<void *>(root_node) = p;
			++height;
		}

		while (p_height > 1) {
			auto p(reinterpret_cast<ptr_node *>(*rr));
			*rr = &p->items[node_offset(pos, p_height)];
			if (!*rr)
				*rr = ptr_node::make(root_node);

			--p_height;
		}

		if (!*rr)
			*rr = data_node::make(root_node);

		return reinterpret_cast<data_node *>(*rr);
	}


};

}}
#endif
