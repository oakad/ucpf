/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_YESOD_DETAIL_SPARSE_VECTOR_OPS_JAN_06_2014_1320)
#define UCPF_YESOD_DETAIL_SPARSE_VECTOR_OPS_JAN_06_2014_1320

#include <yesod/detail/tree_print_decorator.hpp>

namespace ucpf { namespace yesod {

template <typename ValueType, typename Policy>
void sparse_vector<ValueType, Policy>::clear()
{
	auto const height(std::get<0>(tup_height_alloc));
	auto a(std::get<1>(tup_height_alloc));

	if (!height)
		return;

	loc_pair tree_loc[height];
	tree_loc[0] = {&root, 0};
	size_type h(0);

	while (true) {
		auto pp(tree_loc[h].ptr->find_occupied(tree_loc[h].pos));
		tree_loc[h].pos = pp.second;

		if (!pp.first) {
			if (h) {
				tree_loc[h].ptr->destroy(a);
				--h;
				++tree_loc[h].pos;
				continue;
			} else {
				if (root.ptr_at(0))
					*root.ptr_at(0) = nullptr;

				std::get<0>(tup_height_alloc) = 0;
				return;
			}
		}

		if ((h + 1) == height) {
			static_cast<data_node_base *>(*pp.first)->destroy(a);
			++tree_loc[h].pos;
			continue;
		}

		tree_loc[++h] = loc_pair{
			static_cast<ptr_node_base *>(*pp.first), 0
		};
	}
}

template <typename ValueType, typename Policy>
template <typename Pred>
bool sparse_vector<ValueType, Policy>::for_each(
	size_type first, Pred &&pred
) const
{
	auto const height(std::get<0>(tup_height_alloc));

	if (!height || (height_at_pos(first) > height))
		return false;

	c_loc_pair tree_loc[height];
	tree_loc_at(tree_loc, first);

	auto base_pos(first - node_offset(first, 1));
	first -= base_pos;

	if (tree_loc[height - 1].ptr) {
		auto p(static_cast<data_node_base const * const *>(
			tree_loc[height - 1].ptr->ptr_at(
				tree_loc[height - 1].pos
			)
		));

		if (p) {
			auto q(*p);
			while (true) {
				first = q->find_occupied(first);
				if (first == node_size(1))
					break;

				if (pred(
					base_pos + first,
					*(q->ptr_at(first))
				))
					return true;
			}
		}
	}

	while (true) {
		if (!tree_loc_next_valid(tree_loc))
			return false;

		base_pos = tree_loc_pos(tree_loc);

		auto p(*static_cast<data_node_base const * const *>(
			tree_loc[height - 1].ptr->ptr_at(
				tree_loc[height - 1].pos
			)
		));

		while (true) {
			first = p->find_occupied(first);
			if (first == node_size(1))
				break;

			if (pred(base_pos + first, *(p->ptr_at(first))))
				return true;
		}

		first = 0;
		tree_loc_next(tree_loc);
	}
}

template <typename ValueType, typename Policy>
template <typename Pred>
bool sparse_vector<ValueType, Policy>::for_each(
	size_type first, Pred &&pred
)
{
	auto const height(std::get<0>(tup_height_alloc));

	if (!height || (height_at_pos(first) > height))
		return false;

	c_loc_pair tree_loc[height];
	tree_loc_at(tree_loc, first);

	auto base_pos(first - node_offset(first, 1));
	first -= base_pos;

	if (tree_loc[height - 1].ptr) {
		auto p(static_cast<data_node_base **>(
			tree_loc[height - 1].ptr->ptr_at(
				tree_loc[height - 1].pos
			)
		));

		if (p) {
			auto q(*p);
			while (true) {
				first = q->find_occupied(first);
				if (first == node_size(1))
					break;

				if (pred(
					base_pos + first,
					*(q->ptr_at(first))
				))
					return true;
			}
		}
	}

	while (true) {
		if (!tree_loc_next_valid(tree_loc))
			return false;

		base_pos = tree_loc_pos(tree_loc);

		auto p(*static_cast<data_node_base **>(
			tree_loc[height - 1].ptr->ptr_at(
				tree_loc[height - 1].pos
			)
		));

		while (true) {
			first = p->find_occupied(first);
			if (first == node_size(1))
				break;

			if (pred(base_pos + first, *(p->ptr_at(first))))
				return true;
		}

		first = 0;
		tree_loc_next(tree_loc);
	}
}

template <typename ValueType, typename Policy>
auto sparse_vector<ValueType, Policy>::find_vacant(
	size_type first
) const -> size_type
{
	auto const height(std::get<0>(tup_height_alloc));

	if (!height || (height_at_pos(first) > height))
		return first;

	c_loc_pair tree_loc[height];
	tree_loc_at(tree_loc, first);

	while (true) {
		auto p(tree_loc[height - 1].ptr);
		if (!p)
			return first;

		auto q(static_cast<data_node_base **>(p->ptr_at(
			node_offset(first, 2)
		)));
		if (!q)
			return first;

		auto pos(*q->find_vacant(node_offset(first, 1)));
		if (pos < node_size(1))
			return first;

		first += pos - node_offset(first, 1);
		tree_loc_next(tree_loc);
	}
}

template <typename ValueType, typename Policy>
std::ostream &sparse_vector<ValueType, Policy>::dump(std::ostream &os) const
{
	struct printer_type {
		printer_type(std::ostream &os_)
		: os(os_)
		{}

		void operator()(char const *str)
		{
			os << str;
		}

		std::ostream &os;
	} printer(os);

	detail::tree_print_decorator<
		printer_type, allocator_type
	> decorator(printer, std::get<1>(tup_height_alloc));

	os << '<' << this << ">\n";
	auto const height(std::get<0>(tup_height_alloc));
	if (!height) {
		return os;
	}

	c_loc_pair tree_loc[height];
	tree_loc[0] = c_loc_pair{&root, 0};
	size_type h(0);

	while (true) {
		auto pos(tree_loc[h].pos);
		auto pp(tree_loc[h].ptr->find_occupied(pos));
		if (!pp.first) {
			if (!h)
				return os;

			--h;
			decorator.pop_level();
			continue;
		}

		auto pq(tree_loc[h].ptr->find_occupied(pp.second + 1));
		tree_loc[h].pos = pq.second;
		if (pq.first)
			decorator.next_child();
		else
			decorator.last_child();

		os << '[' << pp.second << "] <" << *pp.first << ">\n";
		if ((h + 1) == height) {
			auto q(static_cast<data_node_base *>(*pp.first));
			auto qp(q->find_occupied(0));
			if (!qp.first)
				continue;

			auto qq(q->find_occupied(qp.second + 1));
			decorator.push_level();

			while (qq.first) {
				decorator.next_child();
				os << '[' << qp.second << "] " << *qp.first
				   << '\n';
				qp = qq;
				qq = q->find_occupied(qp.second + 1);
			}
			decorator.last_child();
			os << '[' << qp.second << "] " << *qp.first
			   << '\n';
			decorator.pop_level();
		} else {
			decorator.push_level();
			tree_loc[++h] = c_loc_pair{
				static_cast<ptr_node_base *>(*pp.first), 0
			};
		}
	}
}

template <typename ValueType, typename Policy>
auto sparse_vector<ValueType, Policy>::alloc_data_node_at(
	sparse_vector<ValueType, Policy>::size_type pos
) -> std::pair<data_node_base *, node_base **>
{
	typedef allocator::array_helper<ptr_node_type, allocator_type> a_hp;
	typedef allocator::array_helper<data_node_type, allocator_type> a_hd;

	auto a(std::get<1>(tup_height_alloc));
	auto &h(std::get<0>(tup_height_alloc));
	auto p_h(height_at_pos(pos));
	size_type d_pos(0);

	auto release_p([&d_pos](ptr_node_base *p) -> void {
		p->release_at(d_pos);
	});

	std::unique_ptr<ptr_node_base, decltype(release_p)> u_node_ptr(
		nullptr, release_p
	);

	if (!h) {
		std::pair<data_node_base *, node_base **> rv(nullptr, nullptr);
		u_node_ptr.reset(&root);
		auto rr(root.reserve_at(0));
		rv.first = a_hd::alloc_n(a, 1, a);
		*rr.first = rv.first;
		rv.second = rr.first;
		u_node_ptr.release();
		h = 1;

		if (p_h == h)
			return rv;

		auto p(a_hp::alloc_n(a, 1, a));
		auto pp(p->reserve_at(node_offset(pos, h + 1)));
		*pp.first = rv.first;
		*rr.first = p;
		rv.second = pp.first;
		++h;

		while (p_h > h) {
			p = a_hp::alloc_n(a, 1, a);
			pp = p->reserve_at(node_offset(pos, h + 1));
			*pp.first = rv.first;
			*rr.first = p;
			++h;
		}

		return rv;
	}

	auto qq(root.reserve_at(0));
	while (p_h > h) {
		auto p(a_hp::alloc_n(a, 1, a));
		auto pp(p->reserve_at(0));
		*pp.first = *qq.first;
		*qq.first = p;
		++h;
	}

	if (h > p_h)
		p_h = h;

	auto p(qq.first);
	auto q(*p);
	if (p_h == 1)
		return std::make_pair(static_cast<data_node_base *>(q), p);

	while (true) {
		d_pos = node_offset(pos, p_h);
		qq = static_cast<ptr_node_base *>(q)->reserve_at(d_pos);
		printf("xx (%zd) %p, %zd - %p, %d\n", p_h, q, d_pos, qq.first, qq.second);
		if (qq.first) {
			p = qq.first;
			if (p_h == 2)
				break;

			if (!qq.second) {
				u_node_ptr.reset(
					static_cast<ptr_node_base *>(q)
				);
				*p = a_hp::alloc_n(a, 1, a);
				u_node_ptr.release();
			}

			q = *p;
			--p_h;
		} else
			q = static_cast<ptr_node_base *>(q)->grow_node(a, p);
	}

	if (!qq.second) {
		printf("yy %p, %p, %zd\n", p, q, d_pos);
		u_node_ptr.reset(static_cast<ptr_node_base *>(q));
		*p = a_hd::alloc_n(a, 1, a);
		u_node_ptr.release();
	}
	return std::make_pair(static_cast<data_node_base *>(*p), p);
}

template <typename ValueType, typename Policy>
void sparse_vector<ValueType, Policy>::tree_loc_at(
	c_loc_pair *tree_loc, size_type pos
) const
{
	auto const height(std::get<0>(tup_height_alloc));
	tree_loc[0] = c_loc_pair{&root, 0};
	size_type h(1);

	while (h < height) {
		auto p(static_cast<ptr_node_base *>(
			tree_loc[h - 1].ptr
			? tree_loc[h - 1].ptr->ptr_at(tree_loc[h - 1].pos)
			: nullptr
		));
		if (!p) {
			while (h < height) {
				tree_loc[h] = c_loc_pair{
					nullptr, node_offset(pos, h)
				};
				++h;
			}
			return;
		}

		tree_loc[h] = c_loc_pair{*p, node_offset(pos, h)};
		++h;
	}
}

template <typename ValueType, typename Policy>
auto sparse_vector<ValueType, Policy>::tree_loc_pos(
	c_loc_pair *tree_loc
) const -> size_type
{
	auto const height(std::get<0>(tup_height_alloc));
	size_type rv(0);
	for (size_type h(1); h < height; ++h) {
		auto const f(pos_field_map::value[height - h + 1]);
		auto pos(tree_loc[h].pos & ((size_type(1) << f.first) - 1));
		rv |= pos << f.second;
	}
	return rv;
}

template <typename ValueType, typename Policy>
void sparse_vector<ValueType, Policy>::tree_loc_next(
	c_loc_pair *tree_loc
) const
{
	auto const height(std::get<0>(tup_height_alloc));
	auto h(height - 1);
	size_type rv(0);

	while (h > 0) {
		++tree_loc[h].pos;
		if (tree_loc[h].pos >= node_size(height - h + 1)) {
			tree_loc[h--] = c_loc_pair{nullptr, 0};
			continue;
		}

		while (h < (height - 1)) {
			if (!tree_loc[h].ptr)
				return;

			auto p(tree_loc[h].ptr->ptr_at(tree_loc[h].pos));
			if (!p)
				return;

			tree_loc[++h].ptr = p;
		}
	}

	tree_loc[0].ptr = nullptr;
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

#endif
}}
#endif
