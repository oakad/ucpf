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
> auto flat_map_impl<
	KeyOfValue, CompareF, Alloc, AllocPolicy
>::adjust_reserve(size_type cnt) -> difference_type
{
	if (cnt <= alloc_size())
		return 0;

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
		return n_begin;
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

	difference_type r_off(n_begin - begin_pos);
	destroy_all();
	value_alloc::free_s(alloc, data, alloc_size());
	data = n_data.release();
	bit_index.swap(n_index);
	begin_pos = n_begin;
	end_pos = n_end;
	std::get<0>(aux) = sz;
	return r_off;
}

template <
	typename KeyOfValue, typename CompareF, typename Alloc,
	typename AllocPolicy
> auto flat_map_impl<
	KeyOfValue, CompareF, Alloc, AllocPolicy
>::lower_bound(key_type const &key) const -> const_iterator
{
	if (end_pos <= begin_pos)
		return cend();

	auto c_begin(begin_pos), c_end(end_pos - 1);

	do {
		auto c_pos((c_begin + c_end) >> 1);

		if (!bit_index.test(c_pos)) {
			auto x_pos(bit_index.template find_above<true>(
				c_pos, end_pos
			));
			auto p(ptr_at(x_pos));

			if (p) {
				if (compare_keys(key_ref(p), key)) {
					c_begin = x_pos;
					continue;
				}

				c_end = x_pos;
				x_pos = bit_index.template find_below<true>(
					c_pos, begin_pos
				);
				auto q(ptr_at(x_pos));
				if (!q || compare_keys(key_ref(q), key))
					return const_iterator(
						this, c_end
					);

				c_end = x_pos;
			} else {
				x_pos = bit_index.template find_below<true>(
					c_pos, begin_pos
				);
				p = ptr_at(x_pos);
				if (!p || compare_keys(key_ref(p), key))
					return cend();

				c_end = x_pos;
			}
		} else {
			auto p(ptr_at(c_pos));
			if (compare_keys(key_ref(p), key))
				c_begin = c_pos;
			else
				c_end = c_pos;
		}
	} while (c_begin != c_end);

	auto p(ptr_at(c_begin));
	return p ? const_iterator(this, c_begin) : cend();
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
			adjust_reserve(4);

		printf("aa %zd\n", alloc_size());
		begin_pos = alloc_size() / 2;
		end_pos = begin_pos;
		emplace_at(begin_pos, std::forward<Args>(args)...);
		++end_pos;
		return std::make_pair(iterator(this, begin_pos), true);
	}

	printf("eu1 %zd, %zd, %zd\n", hint.pos, begin_pos, end_pos);

	auto l_pos(bit_index.template find_below<false>(hint.pos + 1));
	printf("eu_l %zd\n", l_pos);
	if (bit_index.valid(l_pos) && ((hint.pos - l_pos) <= 1)) {
		if (l_pos < begin_pos)
			begin_pos = l_pos;

		emplace_at(l_pos, std::forward<Args>(args)...);
		return restore_order_unique(l_pos);
	}

	auto h_pos(bit_index.template find_above<false>(hint.pos + 1));
	printf("eu_h %zd\n", h_pos);
	if (h_pos > l_pos) {
		if (l_pos < begin_pos)
			begin_pos = l_pos;

		emplace_at(l_pos, std::forward<Args>(args)...);
		return restore_order_unique(l_pos);
	}

	if (h_pos > alloc_size()) {
		hint = const_iterator(
			this, hint.pos + adjust_reserve(alloc_size() + 1)
		);
		return emplace_unique(
			hint, std::forward<Args>(args)...
		);
	} else if (h_pos >= end_pos)
		end_pos = h_pos + 1;

	emplace_at(h_pos, std::forward<Args>(args)...);
	return restore_order_unique(h_pos);
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
