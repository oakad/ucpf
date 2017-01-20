/*
 * Copyright (c) 2017 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_59A9D97B3D7FB9065888413615DE137D)
#define HPP_59A9D97B3D7FB9065888413615DE137D

#include <yesod/memory.hpp>
#include <yesod/iterator/locator_facade.hpp>
#include <yesod/detail/paged_storage_layout.hpp>

namespace ucpf::yesod {

template <
	typename ValueType,
	size_t ValueNodeOrder = 8,
	size_t PtrNodeOrder = 6
>
struct paged_storage {
	typedef ValueType value_type;
	typedef size_t size_type;

private:
	struct node {};
	typedef detail::paged_storage_layout<
		size_type, ValueNodeOrder, PtrNodeOrder
	> layout;

	typedef std::aligned_storage_t<sizeof(value_type)> storage_type;

	struct value_node : node {
		constexpr static size_t size()
		{
			return size_t(1) << ValueNodeOrder;
		}

		static value_node *create(pmr::memory_resource *mem_alloc)
		{
			auto deleter = [mem_alloc](value_node *ptr) {
				mem_alloc->deallocate(ptr, sizeof(value_node));
			};
			std::unique_ptr<value_node, decltype(deleter)> p(
				static_cast<value_node *>(
					mem_alloc->allocate(sizeof(value_node))
				), deleter
			);
			uninitialized_default_construct_n(
				reinterpret_cast<value_type *>(p->data),
				size()
			);
			return p.release();
		}

		static void destroy(node *p_, pmr::memory_resource *mem_alloc)
		{
			if (!p_)
				return;

			auto p(static_cast<value_node *>(p_));
			destroy_n(
				reinterpret_cast<value_type *>(p->data),
				size()
			);
			mem_alloc->deallocate(p, sizeof(value_node));
		}

		value_type &operator[](size_type pos)
		{
			return *(reinterpret_cast<value_type *>(
				data + pos
			));
		}

		value_type const &operator[](size_type pos) const
		{
			return *(reinterpret_cast<value_type const *>(
				data + pos
			));
		}

		storage_type data[size()];
	};

	static_assert(std::is_pod<value_node>::value);

	struct ptr_node : node {
		constexpr static size_t size()
		{
			return size_t(1) << PtrNodeOrder;
		}

		static ptr_node *create(pmr::memory_resource *mem_alloc)
		{
			auto p(static_cast<ptr_node *>(
				mem_alloc->allocate(sizeof(ptr_node))
			));
			__builtin_memset(p->nodes, 0, sizeof(p->nodes));
			return p;
		}

		static void destroy(
			node *p_, size_t h, pmr::memory_resource *mem_alloc
		)
		{
			if (!p_)
				return;

			auto p(static_cast<ptr_node *>(p_));
			if (h > 1) {
				for (size_t pos(0); pos < size(); ++pos)
					ptr_node::destroy(
						p->nodes[pos], h - 1, mem_alloc
					);
			} else {
				for (size_t pos(0); pos < size(); ++pos)
					value_node::destroy(
						p->nodes[pos], mem_alloc
					);
			}
			mem_alloc->deallocate(p, sizeof(ptr_node));
		}

		node *nodes[size()];
	};
	
	static_assert(std::is_pod<ptr_node>::value);

public:
	struct locator : iterator::locator_facade<
		locator, std::bidirectional_iterator_tag, ptrdiff_t
	> {
	private:
		template <typename, size_t, size_t>
		friend struct paged_storage;
		friend struct iterator::locator_core_access;

		constexpr static size_t depth = 3;

		locator(size_type pos_, paged_storage const &st)
		: pos(pos_)
		{
			st.pos_to_locus(pos, clocus);
		}

		size_type pos;
		union {
			node const *clocus[depth];
			node mutable *locus[depth];
		};
	};

	paged_storage()
	: root(nullptr), height(0), mem_alloc(pmr::new_delete_resource())
	{}

	~paged_storage()
	{
		if (height)
			ptr_node::destroy(root, height, mem_alloc);
		else
			value_node::destroy(root, mem_alloc);
	}

	locator at(size_type pos) const
	{
		return locator(pos, *this);
	}

	value_type const &get(locator const &loc) const
	{
		if (loc.clocus[0])
			return (*static_cast<value_node const *>(
				loc.clocus[0]
			))[layout::offset_at_height(loc.pos, 0)];
		else
			return default_value;
	}

	void put(locator &loc, value_type &&v)
	{
		if (!loc.locus[0])
			init_node(loc.pos, loc.locus);

		(*static_cast<value_node *>(loc.locus[0]))[
			layout::offset_at_height(loc.pos, 0)
		] = std::move(v);
	}

	void put(locator &&loc, value_type &&v)
	{
		if (!loc.locus[0])
			init_node(loc.pos, loc.locus);

		(*static_cast<value_node *>(loc.locus[0]))[
			layout::offset_at_height(loc.pos, 0)
		] = std::move(v);
	}

private:
	template <size_t N>
	void pos_to_locus(size_type pos, node const *(&locus)[N]) const
	{
		auto ph(layout::height_at_offset(pos));
		size_t h;
		if (ph > height) {
			for (h = 0; h < N; ++h)
				locus[h] = nullptr;

			return;
		} else {
			for (h = N - 1; h > height; --h)
				locus[h] = nullptr;
		}

		locus[h] = node_at_height(pos, h);

		for (; h > 0; --h) {
			if (locus[h - 1]) {
				locus[h - 1] = static_cast<ptr_node const *>(
					locus[h]
				)->nodes[layout::offset_at_height(pos, h)];
			} else {
				do {
					--h;
					locus[h] = nullptr;
				} while (h);
				return;
			}
		}
	}

	node const *node_at_height(size_type pos, size_t h) const
	{
		auto p(root);
		for (size_t hh(height); hh > h; --hh) {
			if (!p)
				break;
			p = static_cast<ptr_node const *>(p)->nodes[
				layout::offset_at_height(pos, hh)
			];
		}
		return p;
	}

	template <size_t N>
	void init_node(size_type pos, node *(&locus)[N])
	{
		size_t h;

		for (h = 1; h < N; ++h) {
			if (locus[h]) {
				init_children(pos, h, locus);
				return;
			}
		}

		h = layout::height_at_offset(pos);
		for (; height < h; ++height) {
			auto p(ptr_node::create(this->mem_alloc));
			static_cast<ptr_node *>(p)->nodes[0] = root;
			root = p;
		}

		if (N > height) {
			locus[height] = root;
			init_children(pos, height, locus);
		} else {
			locus[N - 1] = init_parents<N - 1>(pos);
			init_children(pos, N - 1, locus);
		}
	}

	template <size_t N>
	void init_children(
		size_type pos, size_t loc_h, node *(&locus)[N]
	)
	{
		switch (loc_h) {
		case 0:
			root = value_node::create(this->mem_alloc);
			locus[0] = root;
			break;
		case 1:
			locus[0] = value_node::create(this->mem_alloc);
			static_cast<ptr_node *>(locus[1])->nodes[
				layout::offset_at_height(pos, 1)
			] = locus[0];
			break;
		default:
			locus[loc_h - 1] = ptr_node::create(this->mem_alloc);
			static_cast<ptr_node *>(locus[loc_h])->nodes[
				layout::offset_at_height(pos, loc_h)
			] = locus[loc_h - 1];
			init_children(pos, loc_h - 1, locus);
		}
	}

	template <size_t N>
	node *init_parents(size_type pos)
	{
		auto p(static_cast<ptr_node *>(root));
		for (size_t h(height); h > N; --h) {
			auto q(static_cast<ptr_node *>(
				p->nodes[layout::offset_at_height(pos, h)]
			));

			if (!q) {
				q = ptr_node::create(this->mem_alloc);
				p->nodes[layout::offset_at_height(pos, h)] = q;
			}

			p = q;
		}
		return p;
	}

	node *root;
	size_t height;
	pmr::memory_resource *mem_alloc;
	value_type default_value;
};

}
#endif
