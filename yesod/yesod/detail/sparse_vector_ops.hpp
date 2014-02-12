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
bool sparse_vector<ValueType, Policy>::for_each(
	std::function<bool (size_type, const_reference)> &&f
) const
{
	if (!height)
		return true;

	if (height == 1) {
		auto n(reinterpret_cast<data_node const *>(
			std::get<0>(root_node)
		));
		return n->for_each(0, std::forward<decltype(f)>(f));
	}

	size_type offset(0);

	return for_each_impl(
		reinterpret_cast<ptr_node const *>(std::get<0>(root_node)),
		0, offset, std::forward<decltype(f)>(f)
	);
}

template <typename ValueType, typename Policy>
bool sparse_vector<ValueType, Policy>::for_each_impl(
	ptr_node const *p, size_type h, size_type &offset,
	std::function<bool (size_type, const_reference)> &&f
) const
{
	if ((height - h) == 2) {
		for (auto q: *p) {
			if (q) {
				auto dq(reinterpret_cast<data_node const *>(q));
				if (!dq->for_each(
					offset, std::forward<decltype(f)>(f)
				))
					return false;
			}
		}
		offset += size_type(1) << (
			Policy::data_node_order + Policy::ptr_node_order
		);
		return true;
	}

	for (auto q: *p) {
		if (q) {
			if (!for_each_impl(
				reinterpret_cast<ptr_node const *>(q),
				++h, offset, std::forward<decltype(f)>(f)
			))
				return false;
		} else {
			offset += size_type(1) << (
				Policy::data_node_order
				+ (Policy::ptr_node_order * (height - h - 1))
			);
		}
	}
	return true;
}

template <typename ValueType, typename Policy>
auto sparse_vector<ValueType, Policy>::find_empty_above(
	size_type pos
) const -> size_type
{
	constexpr size_type data_node_mask(
		(size_type(1) << Policy::data_node_order) - 1
	);

	if (!height)
		return 0;

	loc_pair tree_loc[height];

	if (!tree_loc_from_pos(tree_loc, pos)) {
		do {
			if (!tree_loc[height - 1].ptr)
				return tree_loc_to_pos(tree_loc);

			auto node(reinterpret_cast<data_node *>(
				tree_loc[height - 1].ptr
			));
			tree_loc[height - 1].off = node->find_empty_above(
				pos & data_node_mask
			);
			if (tree_loc[height - 1].off < node->size())
				return tree_loc_to_pos(tree_loc);

			--tree_loc[height - 1].off;
		} while (!tree_loc_next(tree_loc));
	}
	pos = data_node_mask + 1;
	pos <<= height * Policy::ptr_node_order;
	return pos;
}

template <typename ValueType, typename Policy>
bool sparse_vector<ValueType, Policy>::tree_loc_from_pos(
	loc_pair *tree_loc, size_type pos
) const
{
	constexpr size_type data_node_mask(
		(size_type(1) << Policy::data_node_order) - 1
	);
	constexpr size_type ptr_node_mask(
		(size_type(1) << Policy::ptr_node_order) - 1
	);

	if (height < 2) {
		if (pos > data_node_mask) {
			tree_loc[0] = loc_pair{nullptr, 0};
			return true;
		} else {
			tree_loc[0] = loc_pair{std::get<0>(root_node), pos};
			return false;
		}
	}

	size_type shift(Policy::data_node_order);
	shift += (height - 2) * Policy::ptr_node_order;

	if ((pos >> shift) > ptr_node_mask) {
		for (size_type h_pos(0); h_pos < height; ++h_pos)
			tree_loc[h_pos] = loc_pair{nullptr, 0};

		return true;
	}

	auto p_node(std::get<0>(root_node));
	for (size_type h_pos(0); h_pos < (height - 1); ++h_pos) {
		tree_loc[h_pos] = loc_pair{
			p_node, (pos >> shift) & ptr_node_mask
		};

		if (p_node)
			p_node = (*reinterpret_cast<ptr_node *>(p_node))[
				(pos >> shift) & ptr_node_mask
			];

		shift -= Policy::ptr_node_order;
	}

	tree_loc[height - 1] = loc_pair{p_node, pos & data_node_mask};

	return false;
}

template <typename ValueType, typename Policy>
bool sparse_vector<ValueType, Policy>::tree_loc_next(loc_pair *tree_loc) const
{
	constexpr size_type data_node_sz(
		size_type(1) << Policy::data_node_order
	);
	constexpr size_type ptr_node_sz(size_type(1) << Policy::ptr_node_order);

	auto pos(height - 1);
	++tree_loc[pos].off;
	if (tree_loc[pos].off < data_node_sz)
		return false;

	tree_loc[pos].off = 0;

	if (height < 2)
		return true;

	for (auto pos(height - 2); pos >= 0; --pos) {
		++tree_loc[pos].off;
		if (tree_loc[pos].off < ptr_node_sz) {
			for (auto rpos(pos + 1); rpos < (height - 1); ++rpos) {
				auto q(reinterpret_cast<ptr_node *>(
					tree_loc[rpos - 1].ptr
				));
				tree_loc[rpos] = loc_pair{
					q ? &q[tree_loc[rpos - 1].off]
					  : nullptr, 0
				};
			}
			return false;
		} else
			tree_loc[pos] = loc_pair{nullptr, 0};
	}
	return true;
}

template <typename ValueType, typename Policy>
auto sparse_vector<ValueType, Policy>::tree_loc_to_pos(
	loc_pair *tree_loc
) const -> size_type
{
	size_type pos(tree_loc[height - 1].off);
	size_type shift(Policy::data_node_order);

	if (height < 2)
		return pos;

	for (auto h_pos(height - 2); h_pos >= 0; --h_pos) {
		pos |= tree_loc[h_pos].off << shift;
		shift += Policy::ptr_node_order;
	}

	return pos;
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
