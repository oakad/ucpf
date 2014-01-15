/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_YESOD_DETAIL_SPARSE_VECTOR_OPS_JAN_06_2014_1320)
#define UCPF_YESOD_DETAIL_SPARSE_VECTOR_OPS_JAN_06_2014_1320

namespace ucpf { namespace yesod {

template <typename ValueType, typename Policy>
void sparse_vector<ValueType, Policy>::destroy_node_r(
	node_pointer p_, size_type h
)
{
	if (!p_)
		return;

	if (h == 1) {
		data_node::destroy(
			std::get<1>(root_node),
			reinterpret_cast<data_node *>(p_)
		);
		return;
	}

	auto p(reinterpret_cast<ptr_node *>(p_));
	for (auto &q: *p)
		destroy_node_r(reinterpret_cast<node_pointer>(q), h - 1);

	ptr_node::destroy(std::get<1>(root_node), p);
}

template <typename ValueType, typename Policy>
auto sparse_vector<
	ValueType, Policy
>::data_node_at(size_type pos) -> data_node *
{
	auto h(height);
	auto p(std::get<0>(root_node));

	while (h > 1) {
		auto &q(*reinterpret_cast<ptr_node *>(p));
		p = q[node_offset(pos, h)];
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
	auto p(std::get<0>(root_node));

	while (h > 1) {
		auto &q(*reinterpret_cast<ptr_node const *>(p));
		p = q[node_offset(pos, h)];
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

	node_pointer *rr(&std::get<0>(root_node));
	if (!height) {
		while (p_height > 1) {
			auto p(ptr_node::construct(std::get<1>(root_node)));
			*rr = p;
			rr = &(*p)[node_offset(pos, p_height)];
			--p_height;
			++height;
		}

		auto p(data_node::construct(std::get<1>(root_node)));
		*rr = p;
		height = 1;
		return p;
	}

	while (p_height > height) {
		auto p(ptr_node::construct(std::get<1>(root_node)));
		(*p)[0] = std::get<0>(root_node);
		std::get<0>(root_node) = p;
		++height;
	}

	while (p_height > 1) {
		auto p(reinterpret_cast<ptr_node *>(*rr));
		*rr = &(*p)[node_offset(pos, p_height)];
		if (!*rr)
			*rr = ptr_node::construct(std::get<1>(root_node));

		--p_height;
	}

	if (!*rr)
		*rr = data_node::construct(std::get<1>(root_node));

	return reinterpret_cast<data_node *>(*rr);
}

}}
#endif
