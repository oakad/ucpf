/*
 * Copyright (C) 2012 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(_PARDES_INTERNAL_BASIC_BTREE_OPS_HPP)
#define _PARDES_INTERNAL_BASIC_BTREE_OPS_HPP

namespace pardes
{

template <typename key_type_, typename value_type_, typename traits_type>
template <typename iter_type>
void basic_btree<key_type_, value_type_, traits_type>::iterator_base<iter_type>
::increment()
{
	if (pos != n->cend()) {
		++pos;
		if (pos != n->cend())
			return;
	}

	size_type h;
	auto b(n->right_stem(h));

	if (b) {
		b = (b->parent_pos + 1)->b;
		for (; h > 1; --h)
			b = b->c.front().b;

		n = b->c.front().l;
		pos = n->c.cbegin();
	}
}

template <typename key_type_, typename value_type_, typename traits_type>
template <typename iter_type>
void basic_btree<key_type_, value_type_, traits_type>::iterator_base<iter_type>
::decrement()
{
	if (pos != n->cbegin()) {
		--pos;
		return;
	}

	size_type h;
	auto b(n->left_stem(h));

	if (b) {
		b = (b->parent_pos - 1)->b;
		for (; h > 1; --h)
			b = b->c.back().b;

		n = b->c.back().l;
		pos = n->c.cend() - 1;
	}
}

template <typename key_type_, typename value_type_, typename traits_type>
auto basic_btree<key_type_, value_type_, traits_type>::begin() -> iterator
{
	auto l(root.l);

	if (height > 1) {
		auto p(root.b);
		for (auto h(height); h; --h)
			p = p->c.front().b;

		l = p->c.front().l;
	}

	return l ? iterator(l, l->c.begin()) : iterator();

}

template <typename key_type_, typename value_type_, typename traits_type>
auto basic_btree<key_type_, value_type_, traits_type>::end() -> iterator
{
	auto l(root.l);

	if (height > 1) {
		auto p(root.b);
		for (auto h(height); h; --h)
			p = p->c.back().b;

		l = p->c.back().l;
	}

	return l ? iterator(l, l->c.end()) : iterator();
}

template <typename key_type_, typename value_type_, typename traits_type>
auto basic_btree<key_type_, value_type_, traits_type>::cbegin() const
-> const_iterator
{
	auto l(root.l);

	if (height > 1) {
		auto p(root.b);
		for (auto h(height); h; --h)
			p = p->c.front().b;

		l = p->c.front().l;
	}

	return l ? const_iterator(l, l->c.cbegin()) : const_iterator();
}

template <typename key_type_, typename value_type_, typename traits_type>
auto basic_btree<key_type_, value_type_, traits_type>::cend() const
-> const_iterator
{
	auto l(root.l);

	if (height > 1) {
		auto p(root.b);
		for (auto h(height); h; --h)
			p = p->back().b;

		l = p->back().l;
	}

	return l ? const_iterator(l, l->c.cend()) : const_iterator();
}

template <typename key_type_, typename value_type_, typename traits_type>
auto basic_btree<key_type_, value_type_, traits_type>::insert_equal(
	value_type_ const &v
) -> iterator
{
	key_type const &k(boost::fusion::at_key<key_of_val_t>(sup)(v));

	auto l(select_leaf(k));
	auto pos(l->lower_bound(k, sup));
	auto n_pos(l->c.insert(pos, v));
	++val_count;
	return iterator(l, n_pos);
}

template <typename key_type_, typename value_type_, typename traits_type>
auto basic_btree<key_type_, value_type_, traits_type>::insert_unique(
	value_type_ const &v
) -> std::pair<iterator, bool>
{
	using boost::fusion::at_key;
	key_type const &k(at_key<key_of_val_t>(sup)(v));

	auto l(select_leaf(k));
	auto pos(l->lower_bound(k, sup));

	if (pos != l->c.cend()) {
		if (k == at_key<key_of_val_t>(sup)(*pos))
			return std::make_pair(
				iterator(
					l, l->c.begin() + (pos - l->c.cbegin())
				), false
			);
	}

	auto n_pos(l->c.insert(pos, v));
	++val_count;
	return std::make_pair(iterator(l, n_pos), true);
}

template <typename key_type_, typename value_type_, typename traits_type>
auto basic_btree<key_type_, value_type_, traits_type>::lower_bound(
	key_type_ const &k
) const -> const_iterator
{
	auto l(root.l);

	if (height > 1) {
		auto p(root.b);
		typename branch_node::const_iterator iter;

		for (auto h(height); h > 1; --h, p = iter->b)
			iter = p->lower_bound(k, sup);

		l = iter->l;
	}

	return l ? const_iterator(l, l->lower_bound(k, sup)) : const_iterator();
}

template <typename key_type_, typename value_type_, typename traits_type>
void basic_btree<key_type_, value_type_, traits_type>::clear()
{
	if (height > 1)
		clear_r(root.b, height);
	else if (height)
		destroy_node(root.l);

	height = 0;
	val_count = 0;
	root.l = nullptr;
}

template <typename key_type_, typename value_type_, typename traits_type>
template <typename char_type, typename char_traits_t>
std::basic_ostream<char_type, char_traits_t> &
basic_btree<key_type_, value_type_, traits_type>::dump(
	basic_btree t, std::basic_ostream<char_type, char_traits_t> &os,
	std::function<decltype(os) & (key_type const &, decltype(os) &)>
	key_dumper,
	std::function<decltype(os) & (key_type const &, decltype(os) &)>
	value_dumper
)
{
	return os;
}

template <typename key_type_, typename value_type_, typename traits_type>
auto basic_btree<key_type_, value_type_, traits_type>::branch_node::lower_bound(
	key_type_ const &key, typename basic_btree::support_type const &sup
) const -> typename branch_node::const_iterator
{
	using boost::fusion::at_key;

	auto k_iter(std::lower_bound(
		k.cbegin(), k.cend(), key, at_key<key_compare_t>(sup)
	));

	return c.cbegin() + (k_iter - k.cbegin());
}

template <typename key_type_, typename value_type_, typename traits_type>
auto basic_btree<key_type_, value_type_, traits_type>::branch_node
::left_stem(size_type &h) const -> branch_node *
{
	auto rv(this);
	auto p(rv->parent);
	auto p_pos(rv->parent_pos);
	h = 0;

	while (p && p_pos == p->c.cbegin()) {
		rv = p;
		p = rv->parent;
		p_pos = rv->parent_pos;
		++h;
	}
	return rv;
}

template <typename key_type_, typename value_type_, typename traits_type>
auto basic_btree<key_type_, value_type_, traits_type>::branch_node
::right_stem(size_type &h) const -> branch_node *
{
	auto rv(this);
	auto p(rv->parent);
	auto p_pos(rv->parent_pos);
	h = 0;

	while (p && (p_pos + 1) == p->c.cend()) {
		rv = p;
		p = rv->parent;
		p_pos = rv->parent_pos;
		++h;
	}
	return rv;
}

template <typename key_type_, typename value_type_, typename traits_type>
auto basic_btree<key_type_, value_type_, traits_type>::leaf_node::lower_bound(
	key_type_ const &key, typename basic_btree::support_type const &sup
) const -> typename leaf_node::const_iterator
{
	using boost::fusion::at_key;

	return std::lower_bound(
		c.cbegin(), c.cend(), key,
		[sup](
			node_value_type const &v, key_type const &key
		) -> bool {
			return at_key<key_compare_t>(sup)(
				at_key<key_of_val_t>(sup)(v), key
			);
		}
	);
}

template <typename key_type_, typename value_type_, typename traits_type>
auto basic_btree<key_type_, value_type_, traits_type>::split_1(
	branch_node *n, key_type_ const &key
) -> branch_node *
{
	using boost::fusion::at_key;

	auto other(create_node<branch_node>());
	auto p(const_cast<branch_node *>(n->parent));
	auto k_pos(p->k.cbegin() + (n->parent_pos - p->c.cbegin()));
	auto c_count(n->c.size() / 2);

	other->c.move_append(n->c.end() - c_count, n->c.end());
	other->k.move_append(n->k.end() - c_count + 1, n->k.end());
	n->c.erase(n->c.end() - c_count, n->c.end());
	n->k.erase(n->k.end() - c_count + 1, n->k.end());

	auto o_pos(p->c.insert(n->parent_pos + 1, other));
	for (auto iter(o_pos); iter != p->c.cend(); ++iter) {
		iter->b->parent = p;
		iter->b->parent_pos = iter;
	}

	for (auto iter(other->c.cbegin()); iter != other->c.cend(); ++iter) {
		iter->b->parent = other;
		iter->b->parent_pos = iter;
	}

	auto iter(p->k.insert(k_pos, std::move(n->k.back())));
	n->k.pop_back();

	if (at_key<key_compare_t>(sup)(*iter, key))
		return other;
	else
		return n;
}

template <typename key_type_, typename value_type_, typename traits_type>
auto basic_btree<key_type_, value_type_, traits_type>::split_1(
	leaf_node *n, key_type_ const &key
) -> leaf_node *
{
	using boost::fusion::at_key;

	auto other(create_node<leaf_node>());
	auto p(const_cast<branch_node *>(n->parent));
	auto k_pos(p->k.cbegin() + (n->parent_pos - p->c.cbegin()));

	other->c.move_append(n->c.end() - (n->c.size() / 2), n->c.end());
	n->c.erase(n->c.cend() - (n->c.size() / 2), n->c.cend());

	auto o_pos(p->c.insert(n->parent_pos + 1, other));
	for (auto iter(o_pos); iter != p->c.cend(); ++iter) {
		iter->l->parent = p;
		iter->l->parent_pos = iter;
	}

	auto iter(p->k.insert(k_pos, at_key<key_of_val_t>(sup)(n->c.back())));

	if (at_key<key_compare_t>(sup)(*iter, key))
		return other;
	else
		return n;
}

template <typename key_type_, typename value_type_, typename traits_type>
auto basic_btree<key_type_, value_type_, traits_type>::select_leaf(
	key_type_ const &key
) -> leaf_node *
{
	using boost::fusion::at_key;

	if (!height) {
		root.l = create_node<leaf_node>();
		++height;
		return root.l;
	} else  if (height == 1) {
		if (!root.l->c.full())
			return root.l;

		auto b(create_node<branch_node>());
		root.l->parent = b;
		root.l->parent_pos = b->c.append(root);
		root.b = b;
		++height;
		return split_1(b->c.back().l, key);
	}

	auto b(root.b);
	auto h(height);

	if (b->c.full()) {
		b = create_node<branch_node>();
		root.b->parent = b;
		root.b->parent_pos = b->c.append(root);
		root.b = b;
		++height;

		b = split_1(b->c.back().b, key);
	}

	auto p_pos(b->lower_bound(key, sup));
	for (--h; h > 2; --h) {
		if (!p_pos->b->c.full())
			b = p_pos->b;
		else
			b = split_1(p_pos->b, key);

		p_pos = b->lower_bound(key, sup);
	}

	if (!p_pos->l->c.full())
		return p_pos->l;
	else
		return split_1(p_pos->l, key);
}

template <typename key_type_, typename value_type_, typename traits_type>
void basic_btree<key_type_, value_type_, traits_type>::clear_r(
	branch_node *b, size_type h
)
{
	if (h > 2) {
		for (auto n : b->c)
			clear_r(n.b, h - 1);
	} else {
		for (auto n : b->c) {
			auto c(n.l->c.size());
			destroy_node(n.l);
			val_count -=c;
		}
	}

	destroy_node(b);
}
/*

template <typename key_type_, typename value_type_, typename traits_type>
auto basic_btree<key_type_, value_type_, traits_type>::expand(
	branch_node *c, key_type_ const &k
) -> branch_node *
{
	using std::get;

	auto lt(c->left()), rt(c->right());
	branch_node *left(get<0>(lt)), *right(get<0>(rt));
	size_type borrow_l(0), borrow_r(0);

	if (left)
		borrow_l = (left->c.max_size() - left->c.size()) / 2;

	if (right)
		borrow_r = (right->c.max_size() - right->c.size()) / 2;

	if (borrow_l) {
		if (borrow_l > borrow_r)
			return left->adjust(k, c, get<1>(lt), get<2>(lt));
		else
			return c->adjust(k, get<0>(rt), get<1>(rt), get<2>(rt));
	} else if (borrow_r)
		return c->adjust(k, get<0>(rt), get<1>(rt), get<2>(rt));


	return nullptr;
}

template <typename key_type_, typename value_type_, typename traits_type>
auto basic_btree<key_type_, value_type_, traits_type>::expand(
	leaf_node *c, key_type_ const &k
) -> leaf_node *
{
	return nullptr;
}
*/
}

#endif
