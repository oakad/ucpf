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
	tree_loc[0] = loc_pair(&root, 0);
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

		tree_loc[++h] = loc_pair(*pp.first, 0);
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
	tree_loc[0] = loc_pair(&root, 0);
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
template <typename Pred>
void sparse_vector<ValueType, Policy>::for_each_pos(
	size_type first, size_type last, Pred &&pred
)
{
	alloc_data_node_at(last - 1);
	auto const height(std::get<0>(tup_height_alloc));
	auto a(std::get<1>(tup_height_alloc));

	loc_pair tree_loc[height];
	tree_loc[0] = loc_pair(&root, 0);
	for (size_type h(1); h < height; ++h)
		tree_loc[h].pos = node_offset(first, height - h + 1);

	auto count(last - first);
	size_type m_height(height - height_at_pos(count));
	size_type h(0);
	size_type d_pos(node_offset(first, 1));

	auto release_p = [](loc_pair *p) -> void {
		p->release();
	};

	std::unique_ptr<
		loc_pair, decltype(release_p)
	> loc_guard(nullptr, release_p);

	auto release_d = [&d_pos](max_data_node_type *p) -> void {
		p->release_at(d_pos);
	};

	std::unique_ptr<
		max_data_node_type, decltype(release_d)
	> loc_guard_d(nullptr, release_d);

	while (true) {
		auto pp(tree_loc[h].reserve());
		if (!pp.first) {
			tree_loc[h].ptr = static_cast<ptr_node_base *>(
				tree_loc[h].ptr->grow_node(
					a, tree_loc[h - 1], h > m_height
				)
			);
			pp = tree_loc[h].reserve();
		}

		if ((h + 1) == height) {
			if (!pp.second) {
				loc_guard.reset(&tree_loc[h]);
				*pp.first = data_node_base::make(a, true);
				loc_guard.release();
			} else
				*pp.first = (*pp.first)->grow_node(
					a, tree_loc[h], true
				);

			auto qq(static_cast<max_data_node_type *>(*pp.first));
			for (; d_pos < node_size(1); ++d_pos) {
				auto dp(qq->reserve_at(d_pos));
				if (!dp.second) {
					loc_guard_d.reset(qq);
					qq->items.init_at(a, d_pos);
					loc_guard_d.release();
				}

				pred(last - count, *dp.first);
				--count;
				if (!count)
					return;
			}

			d_pos = 0;
			++tree_loc[h].pos;
		} else {
			if (!pp.second) {
				loc_guard.reset(&tree_loc[h]);
				*pp.first = ptr_node_base::make(
					a, h > m_height
				);
				loc_guard.release();
			}

			tree_loc[++h].ptr = static_cast<ptr_node_base *>(
				*pp.first
			);
			continue;
		}

		while (tree_loc[h].pos == node_size(height - h + 1)) {
			tree_loc[h] = loc_pair(nullptr, 0);
			--h;
			if (!h)
				return;

			++tree_loc[h].pos;
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
		auto p(tree_loc[h - 1].ptr->ptr_at(tree_loc[h - 1].pos));
		if (!p)
			return first;

		tree_loc[h] = c_loc_pair{
			static_cast<ptr_node_base const *>(*p),
			node_offset(first, height - h + 1)
		};
	}

	size_type h(height - 1);
	size_type n_pos(first >> node_size_shift(1));
	size_type d_pos;

	{
		auto p(tree_loc[h].ptr->ptr_at(tree_loc[h].pos));
		if (!p)
			return first;

		auto q(static_cast<data_node_base const *>(*p));
		d_pos = q->find_vacant(node_offset(first, 1));
		if (d_pos < node_size(1))
			return (n_pos << node_size_shift(1)) + d_pos;
	}

	++tree_loc[h].pos;
	++n_pos;

	while (true) {
		if (tree_loc[h].pos == node_size(height - h + 1)) {
			n_pos >>= node_size_shift(height - h + 1);
			if (h) {
				++tree_loc[--h].pos;
				continue;
			} else
				break;
		}

		while ((h + 1) < height) {
			auto p(tree_loc[h].ptr->ptr_at(tree_loc[h].pos));
			if (!p)
				goto out;

			tree_loc[++h] = c_loc_pair{
				static_cast<ptr_node_base const *>(*p), 0
			};
			n_pos <<= node_size_shift(height - h + 1);
		}

		auto p(tree_loc[h].ptr->ptr_at(tree_loc[h].pos));
		d_pos = 0;
		if (!p || (
			node_size(1) > (
				d_pos = static_cast<
					data_node_base const *
				>(*p)->find_vacant(0)
			)
		))
			return (n_pos << node_size_shift(1)) + d_pos;
		else {
			++tree_loc[h].pos;
			++n_pos;
		}
	}
out:
	for (; h < height; ++h)
		n_pos <<= node_size_shift(height - h + 1);

	return n_pos << node_size_shift(1);
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
void sparse_vector<ValueType, Policy>::shrink_to_fit()
{
	auto a(std::get<1>(tup_height_alloc));
	auto const height(std::get<0>(tup_height_alloc));

	if (!height)
		return;

	loc_pair tree_loc[height];
	tree_loc[0] = loc_pair(&root, 0);
	size_type h(0);

	while (true) {
		auto pp(tree_loc[h].ptr->find_occupied(tree_loc[h].pos));
		if (!pp.first) {
			if (!h)
				break;

			tree_loc[h].ptr->shrink_node(a, tree_loc[h - 1]);
			tree_loc[h].pos = 0;
			++tree_loc[--h].pos;
			continue;
		}

		tree_loc[h].pos = pp.second;

		if ((h + 1) == height) {
			auto q(static_cast<data_node_base *>(*pp.first));
			q->shrink_node(a, tree_loc[h]);
			++tree_loc[h].pos;
		} else
			tree_loc[++h] = loc_pair(*pp.first, 0);
	}

	auto p(root.ptr_at(0));
	if (!p) {
		std::get<0>(tup_height_alloc) = 0;
		return;
	}

	auto q(static_cast<ptr_node_base *>(*p));
	h = height;
	while ((h > 1) && (q->find_occupied(0).second == 0)) {
		if (q->find_occupied(1).first)
			break;

		*p = *(q->ptr_at(0));
		q->destroy(a);
		q = static_cast<ptr_node_base *>(*p);
		--std::get<0>(tup_height_alloc);
		--h;
	}
}

template <typename ValueType, typename Policy>
auto sparse_vector<ValueType, Policy>::utilization() const
-> std::pair<size_type, size_type>
{
		auto a(std::get<1>(tup_height_alloc));
	auto const height(std::get<0>(tup_height_alloc));

	std::pair<size_type, size_type> rv(0, 0);
	if (!height)
		return rv;

	c_loc_pair tree_loc[height];
	tree_loc[0] = c_loc_pair{&root, 0};
	size_type h(0);

	while (true) {
		auto pp(tree_loc[h].ptr->find_occupied(tree_loc[h].pos));

		if (!pp.first) {
			if (!h)
				break;

			rv.first += tree_loc[h].ptr->utilization().first;
			tree_loc[h].pos = 0;
			++tree_loc[--h].pos;
			continue;
		}

		tree_loc[h].pos = pp.second;

		if ((h + 1) == height) {
			auto q(static_cast<data_node_base *>(*pp.first));
			auto qp(q->utilization());
			rv.first += qp.first;
			rv.second += qp.second;
			++tree_loc[h].pos;
		} else
			tree_loc[++h] = c_loc_pair{
				static_cast<ptr_node_base *>(*pp.first), 0
			};
	}

	return rv;
}

template <typename ValueType, typename Policy>
auto sparse_vector<ValueType, Policy>::ptr_node_base::make(
	allocator_type const &a, bool maximize
) -> ptr_node_base *
{
	if (maximize) {
		typedef allocator::array_helper<
			max_ptr_node_type, allocator_type
		> a_h;

		return a_h::alloc_n(a, 1, a);
	} else {
		typedef allocator::array_helper<
			min_ptr_node_type, allocator_type
		> a_h;

		return a_h::alloc_n(a, 1, a);
	}
}

template <typename ValueType, typename Policy>
auto sparse_vector<ValueType, Policy>::data_node_base::make(
	allocator_type const &a, bool maximize
) -> data_node_base *
{
	if (maximize) {
		typedef allocator::array_helper<
			max_data_node_type, allocator_type
		> a_h;

		return a_h::alloc_n(a, 1, a);
	} else {
		typedef allocator::array_helper<
			min_data_node_type, allocator_type
		> a_h;

		return a_h::alloc_n(a, 1, a);
	}
}

template <typename ValueType, typename Policy>
auto sparse_vector<ValueType, Policy>::alloc_data_node_at(
	sparse_vector<ValueType, Policy>::size_type pos
) -> loc_pair
{
	auto a(std::get<1>(tup_height_alloc));
	auto &h(std::get<0>(tup_height_alloc));
	auto p_h(height_at_pos(pos));

	auto release = [](loc_pair *p) -> void {
		p->release();
	};

	std::unique_ptr<loc_pair, decltype(release)> loc_guard(
		nullptr, release
	);

	loc_pair rv(&root, 0);
	if (!h) {
		auto rr(rv.reserve());
		loc_guard.reset(&rv);
		*rr.first = data_node_base::make(a, false);
		loc_guard.release();
		h = 1;

		if (p_h == h)
			return rv;

		{
			rv = loc_pair(
				ptr_node_base::make(a, false),
				node_offset(pos, h + 1)
			);
			auto pp(rv.reserve());
			*pp.first = *rr.first;
			*rr.first = rv.ptr;
			++h;
		}

		while (p_h > h) {
			auto p(ptr_node_base::make(a, false));
			auto pp(p->reserve_at(node_offset(pos, h + 1)));
			*pp.first = *rr.first;
			*rr.first = p;
			++h;
		}

		return rv;
	}

	auto qq(rv.reserve());
	while (p_h > h) {
		rv.ptr = ptr_node_base::make(a, false);
		auto pp(rv.reserve());
		*pp.first = *qq.first;
		*qq.first = rv.ptr;
		++h;
	}

	if (h > p_h)
		p_h = h;

	if (p_h == 1)
		return rv;

	loc_pair parent(&root, 0);
	rv = loc_pair(*qq.first, node_offset(pos, p_h));
	qq = rv.reserve();

	while (true) {
		if (qq.first) {
			if (p_h == 2)
				break;

			if (!qq.second) {
				loc_guard.reset(&rv);
				*qq.first = ptr_node_base::make(a, false);
				loc_guard.release();
			}

			parent = rv;
			rv = loc_pair(*qq.first, node_offset(pos, --p_h));
		} else
			rv.ptr = static_cast<ptr_node_base *>(
				rv.ptr->grow_node(a, parent, false)
			);

		qq = rv.reserve();
	}

	if (!qq.second) {
		loc_guard.reset(&rv);
		*qq.first = data_node_base::make(a, false);
		loc_guard.release();
	}
	return rv;
}

}}
#endif
