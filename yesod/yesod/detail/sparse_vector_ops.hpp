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
auto sparse_vector<ValueType, Policy>::alloc_data_node_at(
	sparse_vector<ValueType, Policy>::size_type pos
) -> std::pair<data_node_base *, node_base **>
{
	typedef allocator::array_helper<ptr_node_type, allocator_type> a_hp;
	typedef allocator::array_helper<data_node_type, allocator_type> a_hd;

	auto &h(std::get<0>(tup_height_alloc));
	auto p_h(height_at_pos(pos));
	auto a(std::get<1>(tup_height_alloc));
	std::pair<data_node_base *, node_base **> rv(nullptr, nullptr);

	if (h) {
		while (p_h > h) {
			auto p(a_hp::alloc_n(a, 1));
			p->init(a);
			auto pp(p->reserve_at(0));
			*(pp.first) = root;
			root = p;
			++h;
		}
	} else {
		rv.first = a_hd::alloc_n(a, 1);
		rv.first->init(a);
		root = rv.first;
		rv.second = &root;
		h = 1;

		if (p_h > h) {
			auto d_pos(node_offset(pos, h + 1));
			auto p(a_hp::alloc_n(a, 1));
			p->init(a);
			auto pp(p->reserve_at(d_pos));
			*(pp.first) = rv.first;
			rv.second = pp.first;
			root = p;
			++h;
		}

		while (p_h > h) {
			auto d_pos(node_offset(pos, h + 1));
			auto p(a_hp::alloc_n(a, 1));
			p->init(a);
			auto pp(p->reserve_at(d_pos));
			*(pp.first) = static_cast<ptr_node_base *>(root);
			root = p;
			++h;
		}
		return rv;
	}

	rv.second = &root;
	while (p_h > 1) {
		auto d_pos(node_offset(pos, p_h));
		auto p(static_cast<ptr_node_base *>(*(rv.second)));
		auto pp(p->reserve_at(d_pos));
		if (!pp->first) {
			p = static_cast<ptr_node_base *>(
				p->grow_node(a, rv.second)
			);
			pp = p->reserve_at(d_pos);
		}

		if (pp.second) {

		} else {

		}
		--p_h;
	}
	// add and return data node
}

#if 0

template <typename ValueType, typename Policy>
template <typename Pred>
bool sparse_vector<ValueType, Policy>::for_each_above(
	size_type pos, Pred &&pred
)
{
	loc_pair tree_loc[height];
	if (!height || tree_loc_from_pos(tree_loc, pos))
		return true;

	while (true) {
		if (tree_loc[height - 1].ptr) {
			auto node(reinterpret_cast<data_node *>(
				tree_loc[height - 1].ptr
			));
			if (!node->for_each_above(
				tree_loc[height - 1].off,
				std::forward<Pred>(pred),
				pos - tree_loc[height - 1].off
			))
				return false;
		}
		if (tree_loc_next_leaf(tree_loc))
			return true;
		else
			pos = tree_loc_to_pos(tree_loc);
	}
}

template <typename ValueType, typename Policy>
template <typename Pred>
bool sparse_vector<ValueType, Policy>::for_each_above(
	size_type pos, Pred &&pred
) const
{
	loc_pair tree_loc[height];
	if (!height || tree_loc_from_pos(tree_loc, pos))
		return true;

	while (true) {
		if (tree_loc[height - 1].ptr) {
			auto node(reinterpret_cast<data_node const *>(
				tree_loc[height - 1].ptr
			));
			if (!node->for_each_above(
				tree_loc[height - 1].off,
				std::forward<Pred>(pred),
				pos - tree_loc[height - 1].off
			))
				return false;
		}
		if (tree_loc_next_leaf(tree_loc))
			return true;
		else
			pos = tree_loc_to_pos(tree_loc);
	}
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

	printf("--1- pos %zd h %zd\n", pos, height);

	if (!tree_loc_from_pos(tree_loc, pos)) {
		for (size_type h(0); h < height; ++h)
			printf("--1-- h %zd ptr %p off %zd\n", h, tree_loc[h].ptr, tree_loc[h].off);

		do {
			if (!tree_loc[height - 1].ptr)
				return tree_loc_to_pos(tree_loc);

			auto node(reinterpret_cast<data_node *>(
				tree_loc[height - 1].ptr
			));
			printf("-x1- pos %zd node %p n_off %zd\n", pos, node, pos & data_node_mask);
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
template <typename CharType, typename Traits>
auto sparse_vector<ValueType, Policy>::dump(
	std::basic_ostream<CharType, Traits> &os
) const -> std::basic_ostream<CharType, Traits> &
{
	auto r(std::get<0>(root_node));
	os << os.widen('<') << r << os.widen('>') << std::endl;

	auto fill([&os](int ident) -> void {
		for (auto c(0); c < ident; ++c)
			os << os.widen(' ');
	});

	auto print_data([&os](node_pointer p_, int ident) -> void {
		auto p(reinterpret_cast<data_node *>(p_));
		for (size_type c(0); c < data_node_size; ++c) {
			auto q(p->ptr_at(c));
			if (q) {
				for (auto d(0); d < ident; ++d)
					os << os.widen(' ');

				os << c << os.widen(':') << os.widen(' ')
				   << *q << std::endl;
			}
		}
	});

	if (!height)
		return os;

	if (height == 1) {
		print_data(r, 2);
		return os;
	}

	std::pair<loc_pair, int> tree_loc[height];
	tree_loc[0] = std::make_pair(loc_pair{std::get<0>(root_node), 0}, 2);
	size_type h(0);
	while (true) {
restart:
		if (h == (height - 1)) {
			print_data(tree_loc[h].first.ptr, tree_loc[h].second);
			--h;
			continue;
		}

		auto p(reinterpret_cast<ptr_node *>(tree_loc[h].first.ptr));
		while (tree_loc[h].first.off < ptr_node_size) {
			tree_loc[h + 1] = std::make_pair(loc_pair{
				(*p)[tree_loc[h].first.off], 0
			}, tree_loc[h].second + 2);
			
			if (tree_loc[h + 1].first.ptr) {
				fill(tree_loc[h].second);
				os << tree_loc[h].first.off;
				os << os.widen(' ') << os.widen('<');
				os << tree_loc[h + 1].first.ptr;
				os << os.widen('>') << std::endl;
				++tree_loc[h].first.off;
				++h;
				goto restart;
			} else
				++tree_loc[h].first.off;
		}

		if (!h)
			break;
		else
			--h;
	}

	return os;
}

template <typename ValueType, typename Policy>
bool sparse_vector<ValueType, Policy>::tree_loc_from_pos(
	loc_pair *tree_loc, size_type pos
) const
{
	constexpr size_type data_node_mask(data_node_size - 1);
	constexpr size_type ptr_node_mask(ptr_node_size - 1);

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
	auto pos(height - 1);
	++tree_loc[pos].off;
	if (tree_loc[pos].off < data_node_size)
		return false;

	tree_loc[pos].off = 0;

	if (height < 2)
		return true;

	for (auto pos(height - 2); pos >= 0; --pos) {
		++tree_loc[pos].off;
		if (tree_loc[pos].off < ptr_node_size) {
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
bool sparse_vector<ValueType, Policy>::tree_loc_next_leaf(
	loc_pair *tree_loc
) const
{
	tree_loc[height - 1] = loc_pair{nullptr, 0};

	if (height < 2)
		return true;

	auto h_pos(height - 2);

	while (true) {
		if (!tree_loc[h_pos].ptr) {
			tree_loc[h_pos].off = 0;
			if (h_pos) {
				--h_pos;
				continue;
			} else
				return true;
		}

		auto node(reinterpret_cast<ptr_node *>(tree_loc[h_pos].ptr));
		++tree_loc[h_pos].off;
		auto n_pos(node->size());
		if (node->for_each_above(
			tree_loc[h_pos].off,
			[&n_pos](size_type pos, node_pointer p) {
				if (p) {
					n_pos = pos;
					return false;
				} else
					return true;
			}
		)) {
			if (h_pos) {
				--h_pos;
				continue;
			} else
				return true;
		}

		++h_pos;
		tree_loc[h_pos] = loc_pair{&node[n_pos], 0};
		if ((h_pos + 1) == height)
			return false;
	}
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
	auto p_height(std::max(height, pos_height(pos)));

	if (p_height == 1) {
		if (!height) {
			std::get<0>(root_node) = data_node::construct(
				std::get<1>(root_node)
			);
			height = 1;
		}
		return reinterpret_cast<data_node *>(std::get<0>(root_node));
	}

	loc_pair tree_loc[p_height];
	tree_off_from_pos(tree_loc, pos, p_height);
	ptr_node *p(nullptr);
	size_type h(0);

	if (!height) {
		std::get<0>(root_node) = ptr_node::construct(
			std::get<1>(root_node)
		);
		tree_loc[0].ptr = std::get<0>(root_node);
		++height;

		for (h = 1; h < (p_height - 1); ++h) {
			p = reinterpret_cast<ptr_node *>(tree_loc[h - 1].ptr);
			(*p)[tree_loc[h - 1].off] = ptr_node::construct(
				std::get<1>(root_node)
			);
			++height;
		}

		p = reinterpret_cast<ptr_node *>(tree_loc[h - 1].ptr);
		auto q(data_node::construct(std::get<1>(root_node)));
		(*p)[tree_loc[h - 1].off] = q;
		++height;
		return q;
	}

	while (p_height > height) {
		p = ptr_node::construct(std::get<1>(root_node));
		(*p)[0] = std::get<0>(root_node);
		std::get<0>(root_node) = p;
		++height;
	}

	tree_loc[0].ptr = std::get<0>(root_node);

	for (h = 1; h < (height - 1); ++h) {
		p = reinterpret_cast<ptr_node *>(tree_loc[h - 1].ptr);
		tree_loc[h].ptr = p->ptr_at(tree_loc[h - 1].off);
		if (!tree_loc[h].ptr) {
			(*p)[tree_loc[h - 1].off] = ptr_node::construct(
				std::get<1>(root_node)
			);
			tree_loc[h].ptr = (*p)[tree_loc[h - 1].off];
		}
	}

	p = reinterpret_cast<ptr_node *>(tree_loc[h - 1].ptr);
	tree_loc[h].ptr = p->ptr_at(tree_loc[h - 1].off);
	if (!tree_loc[h].ptr) {
		(*p)[tree_loc[h - 1].off] = data_node::construct(
			std::get<1>(root_node)
		);
		tree_loc[h].ptr = (*p)[tree_loc[h - 1].off];
	}

	return reinterpret_cast<data_node *>(tree_loc[h].ptr);
}
#endif
}}
#endif
