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
void sparse_vector::for_each(
	std::function<bool (size_type, const_reference)> f
) const
{
	if (!height)
		return;

	if (height == 1) {
		auto n(reinterpret_cast<data_node const *>(
			std::get<0>(root_node)
		));
		n->for_each(0, f);
		return;
	}

	struct vector_pos {
		node_pointer p;
		size_type g_offset;
		size_type l_offset;
		size_type skip;

		vector_pos() = default;

		vector_pos(node_pointer p_, size_type h)
		: p(p_), g_offset(0), l_offset(0),
		  skip(
			  Policy::data_node_order
			  + (height - 2) * Policy::ptr_node_order
		  )
		{
		}

		ptr_node const *get_pptr() const
		{
			return reinterpret_cast<ptr_node const *>(p);
		}

		ptr_node const *get_dptr() const
		{
			return reinterpret_cast<data_node const *>(p);
		}
	};

	vector_pos h_ref[height];
	h_ref[0] = vector_pos({
		std::get<0>(root_node), 0, 0,
		
	});
	auto h_pos(0);

	while (true) {
		if ((h_pos + 1) == height) {
			auto n(reinterpret_cast<data_node const *>(
				std::get<0>(h_ref[h_pos])
			));
			if (n)
				n->for_each(std::get<1>(h_ref[h_pos]), f);

			--h_pos;
			std::get<0
		}
	}
}

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
