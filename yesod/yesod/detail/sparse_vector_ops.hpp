/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_YESOD_SPARSE_VECTOR_OPS_JAN_06_2014_1320)
#define UCPF_YESOD_SPARSE_VECTOR_OPS_JAN_06_2014_1320

namespace ucpf { namespace yesod {

template <typename ValueType, typename Policy>
template <typename... Args>
auto sparse_vector<ValueType, Policy>::data_node_obj::emplace_at(
	size_type pos, allocator_type &a, Args&&... args
) -> reference
{
	auto p(&(*this)[pos]);
	int restore_mode(0);

	if (!value_set.test(pos)) {
		allocator_traits::construct(a, p, std::forward<Args>(args)...);
		value_set.set(pos);
	} else {
		std::function<void (value_type *)> deleter(
			[&a, &restore_mode, p](value_type *bp) {
				switch (restore_mode) {
				case 2:
					allocator_traits::construct(
						a, p, std::move(*bp)
					);
				case 1:
					allocator_traits::destroy(a, bp);
				case 0:
					allocator_traits::deallocate(a, bp, 1);
				}
			}
		);

		std::unique_ptr<value_type, decltype(deleter)> bp(
			allocator_traits::allocate(a, 1), deleter
		);

		allocator_traits::construct(a, bp.get(), std::move(*p));
		restore_mode = 1;

		allocator_traits::destroy(a, p);
		restore_mode = 2;

		allocator_traits::construct(a, p, std::forward<Args>(args)...);
		restore_mode = 1;
	}

	return *p;
}

template <typename ValueType, typename Policy>
void sparse_vector<ValueType, Policy>::data_node_obj::destroy(allocator_type &a)
{
	for (size_type pos(0); pos < data_node_pod::items.size(); ++pos) {
		if (value_set.test(pos))
			allocator_traits::destroy(
				a, &data_node_pod::items[pos]
			);
		}
}

template <typename ValueType, typename Policy>
auto sparse_vector<ValueType, Policy>::make_data_node() -> data_node *
{
	typename data_node::node_allocator_type node_alloc(
		std::get<1>(root_node)
	);

	auto node(data_node::node_allocator_traits::allocate(
		node_alloc, 1
	));
	data_node::node_allocator_traits::construct(
		node_alloc, node, std::get<1>(root_node)
	);
	return node;
}

template <typename ValueType, typename Policy>
void sparse_vector<ValueType, Policy>::destroy_data_node(void_pointer raw_node)
{
	if (!raw_node)
		return;

	auto node(reinterpret_cast<data_node *>(raw_node));
	node->destroy(std::get<1>(root_node));

	typename data_node::node_allocator_type node_alloc(
		std::get<1>(root_node)
	);

	data_node::node_allocator_traits::destroy(node_alloc, node);
	data_node::node_allocator_traits::deallocate(node_alloc, node, 1);
}

template <typename ValueType, typename Policy>
auto sparse_vector<ValueType, Policy>::make_ptr_node() -> ptr_node *
{
	typename ptr_node::node_allocator_type node_alloc(
		std::get<1>(root_node)
	);

	auto p(ptr_node::node_allocator_traits::allocate(node_alloc, 1));

	ptr_node::node_allocator_traits::construct(node_alloc, p);
	return p;
}

template <typename ValueType, typename Policy>
void sparse_vector<ValueType, Policy>::destroy_ptr_node(
	void_pointer raw_node, size_type height
)
{
	if (!raw_node)
		return;

	if (height == 1) {
		destroy_data_node(raw_node);
		return;
	}

	auto node(reinterpret_cast<ptr_node *>(raw_node));
	for (auto p: node->items)
		destroy_ptr_node(p, height - 1);

	typename ptr_node::node_allocator_type node_alloc(
		std::get<1>(root_node)
	);

	ptr_node::node_allocator_traits::destroy(node_alloc, node);
	ptr_node::node_allocator_traits::deallocate(
		node_alloc, node, 1
	);
}

template <typename ValueType, typename Policy>
auto sparse_vector<
	ValueType, Policy
>::data_node_at(size_type pos) -> data_node *
{
	auto h(height);
	auto p(std::get<0>(root_node));

	while (h > 1) {
		auto q(reinterpret_cast<ptr_node *>(p));
		p = q->items[node_offset(pos, h)];
		if (!p)
			return nullptr;
		--h;
	}
	return reinterpret_cast<data_node *>(p);
}

template <typename ValueType, typename Policy>
auto sparse_vector<
	ValueType, Policy
>::data_node_at(size_type pos) const -> data_node const *
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

template <typename ValueType, typename Policy>
auto sparse_vector<
	ValueType, Policy
>::data_node_alloc_at(size_type pos) -> data_node *
{
	size_type p_height(1);
	for (
		auto p_pos(pos >> Policy::data_node_order);
		p_pos; p_pos >>= Policy::ptr_node_order
	)
		++p_height;

	void_pointer *rr(&std::get<0>(root_node));
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

}}
#endif
