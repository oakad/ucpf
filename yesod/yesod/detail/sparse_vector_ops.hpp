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
	if (!height)
		return false;

	c_loc_pair tree_loc[height];
	tree_loc[0] = c_loc_pair{&root, 0};
	for (size_type h(1); h < height; ++h)
		tree_loc[h].pos = node_offset(first, height - h + 1);

	size_type h(0);
	size_type d_pos(node_offset(first, 1));
	size_type n_pos(0);

	while (true) {
		auto pp(tree_loc[h].ptr->find_occupied(tree_loc[h].pos));

		if (!pp.first) {
			if (!h)
				return false;

			tree_loc[h].pos = 0;
			n_pos >>= node_size_shift(height - h + 1);
			++tree_loc[--h].pos;
			continue;
		} else
			n_pos += pp.second - tree_loc[h].pos;

		tree_loc[h].pos = pp.second;

		if ((h + 1) == height) {
			auto q(static_cast<data_node_base const *>(*pp.first));
			auto base_pos(n_pos << node_size_shift(1));
			for (
				auto qq(q->find_occupied(d_pos));
				qq.first;
				qq = q->find_occupied(qq.second + 1)
			) {
				if (pred(base_pos + qq.second, *qq.first))
					return true;
			}

			d_pos = 0;
			++tree_loc[h].pos;
			++n_pos;
		} else {
			tree_loc[++h].ptr = static_cast<
				ptr_node_base const *
			>(*pp.first);
			n_pos <<= node_size_shift(height - h + 1);
			n_pos += tree_loc[h].pos;
		}
	}
}

template <typename ValueType, typename Policy>
template <typename Pred>
bool sparse_vector<ValueType, Policy>::for_each(
	size_type first, Pred &&pred
)
{
	auto const height(std::get<0>(tup_height_alloc));
	if (!height)
		return false;

	loc_pair tree_loc[height];
	tree_loc[0] = loc_pair{&root, 0};
	for (size_type h(1); h < height; ++h)
		tree_loc[h].pos = node_offset(first, height - h + 1);

	size_type h(0);
	size_type d_pos(node_offset(first, 1));
	size_type n_pos(0);

	while (true) {
		auto pp(tree_loc[h].ptr->find_occupied(tree_loc[h].pos));

		if (!pp.first) {
			if (!h)
				return false;

			tree_loc[h].pos = 0;
			n_pos >>= node_size_shift(height - h + 1);
			++tree_loc[--h].pos;
			continue;
		} else
			n_pos += pp.second - tree_loc[h].pos;

		tree_loc[h].pos = pp.second;

		if ((h + 1) == height) {
			auto q(static_cast<data_node_base *>(*pp.first));
			auto base_pos(n_pos << node_size_shift(1));
			for (
				auto qq(q->find_occupied(d_pos));
				qq.first;
				qq = q->find_occupied(qq.second + 1)
			) {
				if (pred(base_pos + qq.second, *qq.first))
					return true;
			}

			d_pos = 0;
			++tree_loc[h].pos;
			++n_pos;
		} else {
			tree_loc[++h].ptr = static_cast<
				ptr_node_base *
			>(*pp.first);
			n_pos <<= node_size_shift(height - h + 1);
			n_pos += tree_loc[h].pos;
		}
	}
}

template <typename ValueType, typename Policy>
auto sparse_vector<ValueType, Policy>::find_vacant(
	size_type first
) const -> size_type
{
	auto const height(std::get<0>(tup_height_alloc));
	if (!height)
		return first;

	c_loc_pair tree_loc[height];
	tree_loc[0] = c_loc_pair{&root, 0};
	for (size_type h(1); h < height; ++h) {
		tree_loc[h] = c_loc_pair{
			tree_loc[h - 1].ptr->ptr_at(tree_loc[h - 1].pos),
			node_offset(first, height - h + 1)
		};
		if (!tree_loc[h].ptr)
			return first;
	}

	size_type h(height - 1);
	size_type d_pos(node_offset(first, 1));
	size_type n_pos(first >> node_size_shift(1));

	while (true) {
		
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
	if (!height)
		return os;

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
			*pp.first = *rr.first;
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
		u_node_ptr.reset(static_cast<ptr_node_base *>(q));
		*p = a_hd::alloc_n(a, 1, a);
		u_node_ptr.release();
	}
	return std::make_pair(static_cast<data_node_base *>(*p), p);
}

}}
#endif
