/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_YESOD_DETAIL_FLAT_MAP_OPS_20140430T1620)
#define UCPF_YESOD_DETAIL_FLAT_MAP_OPS_20140430T1620

namespace ucpf { namespace yesod { namespace detail {

template <
	typename KeyOfValue, typename CompareF, typename Alloc,
	typename AllocPolicy
> void flat_map_impl<
	KeyOfValue, CompareF, Alloc, AllocPolicy
>::reserve(size_type cnt)
{
	if (cnt <= alloc_size())
		return;

	auto sz(AllocPolicy::best_size(cnt));
	auto n_begin((sz - alloc_size()) / 2);
	auto n_end(n_begin + alloc_size());

	decltype(bit_index) n_index(bit_index, n_begin, sz - n_end);

	typename value_alloc::allocator_type alloc(bit_index.get_allocator());

	if (!data) {
		data = value_alloc::alloc_s(alloc, sz);
		bit_index.swap(n_index);
		begin_pos = n_begin;
		end_pos = n_end;
		std::get<0>(aux) = sz;
		return;
	}

	auto init_pos(n_begin);
	auto deleter(
		[&](typename value_alloc::storage_type *p) -> void {
			for (
				init_pos = n_index.template find_below<true>(
					init_pos
				);
				n_index.valid(init_pos)
				&& (init_pos >= n_begin);
				init_pos = n_index.template find_below<true>(
					init_pos
				)
			)
				value_alloc::allocator_traits::destroy(
					alloc, reinterpret_cast<value_type *>(
						&p[init_pos]
					)
				);

			value_alloc::free_s(alloc, p, sz);
		}
	);

	std::unique_ptr<
		typename value_alloc::storage_type[], decltype(deleter)
	> n_data(value_alloc::alloc_s(alloc, sz), deleter);

	for (
		auto pos(begin_pos); pos < end_pos;
		pos = bit_index.template find_above<true>(pos, end_pos)
	) {
		init_pos = pos - begin_pos + n_begin;

		value_alloc::allocator_traits::construct(
			alloc,
			reinterpret_cast<value_type *>(&n_data[init_pos]),
			std::move_if_noexcept(
				reinterpret_cast<value_type &>(data[pos])
			)
		);
	}

	destroy_all();
	value_alloc::free_s(alloc, data, alloc_size());
	data = n_data.release();
	bit_index.swap(n_index);
	begin_pos = n_begin;
	end_pos = n_end;
	std::get<0>(aux) = sz;
	return;
}

template <
	typename KeyOfValue, typename CompareF, typename Alloc,
	typename AllocPolicy
> auto flat_map_impl<
	KeyOfValue, CompareF, Alloc, AllocPolicy
>::lower_bound(key_type const &key) const -> const_iterator
{
	if (end_pos <= begin_pos)
		return const_iterator(this, end_pos);

	auto c_begin(begin_pos), c_end(end_pos);

	while (true) {
		if ((c_begin + 1) == c_end) {
			if (compare_keys(key_ref(data + c_begin), key))
				return cend();
			else
				return const_iterator(this, c_begin);
		}

		auto c_pos(c_begin + c_end);
		if (c_pos & 1)
			c_pos = (c_pos / 2) + 1;
		else
			c_pos = c_pos / 2;

		auto x_end(bit_index.template find_below<true>(
			c_pos, c_begin
		));

		if (!bit_index.valid(x_end)) {
			c_begin = bit_index.template find_above<true>(
				c_pos - 1, c_end
			);

			if (!bit_index.valid(c_begin))
				return cend();

			continue;
		}

		if (compare_keys(key_ref(data + x_end), key)) {
			c_begin = bit_index.template find_above<true>(
				c_pos - 1, c_end
			);

			if (!bit_index.valid(c_begin))
				return cend();
		} else
			c_end = x_end + 1;
	}
}

template <
	typename KeyOfValue, typename CompareF, typename Alloc,
	typename AllocPolicy
> auto flat_map_impl<
	KeyOfValue, CompareF, Alloc, AllocPolicy
>::upper_bound(key_type const &key) const -> const_iterator
{
	if (end_pos <= begin_pos)
		return const_iterator(this, end_pos);

	auto c_begin(begin_pos), c_end(end_pos);

	while (true) {
		if ((c_begin + 1) == c_end) {
			if (compare_keys(key, key_ref(data + c_begin)))
				return const_iterator(this, c_begin);
			else
				return cend();
		}

		auto c_pos(c_begin + c_end);
		if (c_pos & 1)
			c_pos = (c_pos / 2) + 1;
		else
			c_pos = c_pos / 2;

		auto x_end(bit_index.template find_below<true>(
			c_pos, c_begin
		));

		if (!bit_index.valid(x_end)) {
			c_begin = bit_index.template find_above<true>(
				c_pos - 1, c_end
			);

			if (!bit_index.valid(c_begin))
				return cend();

			continue;
		}

		if (compare_keys(key, key_ref(data + x_end)))
			c_end = x_end + 1;
		else {
			
			c_begin = bit_index.template find_above<true>(
				c_pos - 1, c_end
			);

			if (!bit_index.valid(c_begin))
				return cend();
		}
	}
}

template <
	typename KeyOfValue, typename CompareF, typename Alloc,
	typename AllocPolicy
>
template <typename... Args>
auto flat_map_impl<
	KeyOfValue, CompareF, Alloc, AllocPolicy
>::emplace_unique(
	const_iterator hint, Args&&... args
) -> std::pair<iterator, bool>
{
	if (begin_pos != end_pos) {
		if (!ptr_at(hint.pos))
			hint.pos = end_pos;
	} else {
		if (!alloc_size())
			reserve(4);

		begin_pos = alloc_size() / 2;
		end_pos = begin_pos;
		emplace_at(begin_pos, std::forward<Args>(args)...);
		++end_pos;
		return std::make_pair(iterator(this, begin_pos), true);
	}

	auto pos(bit_index.template find_below<false>(hint.pos + 1));
	if (bit_index.valid(pos)) {
		if ((hint.pos - pos) <= 1) {
			if (pos < begin_pos)
				begin_pos = pos;

			emplace_at(pos, std::forward<Args>(args)...);
			return restore_order_unique(pos);
		}

		auto h_pos(bit_index.template find_above<false>(
			hint.pos ? hint.pos - 1 : 0
		));

		if ((h_pos - hint.pos) < (hint.pos - pos))
			pos = h_pos;
	} else
		pos = bit_index.template find_above<false>(
			hint.pos ? hint.pos - 1 : 0
		);

	if (pos >= alloc_size()) {
		reserve(alloc_size() + 1);
		pos = end_pos;
		emplace_at(end_pos, std::forward<Args>(args)...);
		++end_pos;
		return restore_order_unique(pos);
	} else if (pos >= end_pos)
		end_pos = pos + 1;

	emplace_at(pos, std::forward<Args>(args)...);
	return restore_order_unique(pos);
}

template <
	typename KeyOfValue, typename CompareF, typename Alloc,
	typename AllocPolicy
> auto flat_map_impl<
	KeyOfValue, CompareF, Alloc, AllocPolicy
>::restore_order_unique(size_type pos) -> std::pair<iterator, bool>
{
	auto x_pos(bit_index.template find_below<true>(pos, begin_pos));
	bool p_ok(!bit_index.valid(x_pos) || compare_keys(
		key_ref(data + x_pos), key_ref(data + pos)
	));

	if (p_ok) {
		x_pos = bit_index.template find_above<true>(pos, end_pos);
		p_ok = !bit_index.valid(x_pos) || compare_keys(
			key_ref(data + pos), key_ref(data + x_pos)
		);
	}

	if (x_pos < pos) {
		while (!p_ok) {
			if (!compare_keys(
				key_ref(data + pos), key_ref(data + x_pos)
			)) {
				destroy_at(pos);
				return std::make_pair(
					iterator(this, x_pos), false
				);
			}

			swap_pos(x_pos, pos);
			pos = x_pos;
			x_pos = bit_index.template find_below<true>(
				pos, begin_pos
			);
			p_ok = !bit_index.valid(x_pos) || compare_keys(
				key_ref(data + x_pos), key_ref(data + pos)
			);
		}
	} else {
		while (!p_ok) {
			if (!compare_keys(
				key_ref(data + x_pos), key_ref(data + pos)
			)) {
				destroy_at(pos);
				return std::make_pair(
					iterator(this, x_pos), false
				);
			}

			swap_pos(x_pos, pos);
			pos = x_pos;
			x_pos = bit_index.template find_above<true>(
				pos, end_pos
			);
			p_ok = !bit_index.valid(x_pos) || compare_keys(
				key_ref(data + pos), key_ref(data + x_pos)
			);
		}
	}

	return std::make_pair(iterator(this, pos), true);
}

template <
	typename KeyOfValue, typename CompareF, typename Alloc,
	typename AllocPolicy
> auto flat_map_impl<
	KeyOfValue, CompareF, Alloc, AllocPolicy
>::erase(const_iterator first, const_iterator last) -> iterator
{
	auto c_begin(std::max(begin_pos, first.pos));
	auto c_end(std::min(end_pos, last.pos));

	for (
		auto pos(
			bit_index.test(c_begin)
			? c_begin
			: bit_index.template find_above<true>(c_begin, c_end)
		);
		pos < c_end;
		pos = bit_index.template find_above<true>(pos, c_end)
	) {
		destroy_at(pos);
	}

	if (!bit_index.test(begin_pos)) {
		auto pos(bit_index.template find_above<true>(
			begin_pos, end_pos
		));
		if (bit_index.valid(pos))
			begin_pos = pos;
		else {
			begin_pos = alloc_size() / 2;
			end_pos = begin_pos;
			return end();
		}
	}

	end_pos = bit_index.template find_below<true>(end_pos, begin_pos) + 1;

	if (last.pos < end_pos) {
		if (bit_index.test(last.pos))
			return iterator(this, last.pos);
		else {
			auto pos(bit_index.template find_above<true>(
				last.pos, end_pos
			));
			if (bit_index.valid(pos))
				return iterator(this, pos);
		}
	}

	return end();
}

}}}
#endif
