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
	typename ValueType, typename KeyOfValue,
	typename CompareF, typename Alloc, typename AllocPolicy
> void flat_map_impl<
	ValueType, KeyOfValue, CompareF, Alloc, AllocPolicy
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
				init_pos = n_index.find_below<true>(init_pos);
				(init_pos != decltype(n_index)::npos)
				&& init_pos >= n_begin;
				init_pos = n_index.find_below<true>(init_pos)
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
		pos = bit_index.find_above<true>(pos, end_pos)
	) {
		init_pos = pos - begin_pos + n_begin;

		value_alloc::allocator_type::construct(
			alloc, &n_data[init_pos],
			std::move_if_noexcept(
				reinterpret_cast<value_type &>(data[pos])
			)
		);
	}

	clear_data();
	value_alloc::free_s(alloc, data, alloc_size());
	data = n_data;
	bit_index.swap(n_index);
	begin_pos = n_begin;
	end_pos = n_end;
	std::get<0>(aux) = sz;
}

template <
	typename ValueType, typename KeyOfValue,
	typename CompareF, typename Alloc, typename AllocPolicy
> auto flat_map_impl<
	ValueType, KeyOfValue, CompareF, Alloc, AllocPolicy
>::lower_bound(key_type const &key) const -> const_iterator
{
	if (end_pos <= begin_pos)
		return cend();

	auto c_begin(begin_pos), c_end(end_pos - 1);

	do {
		auto c_pos((c_begin + c_end) >> 1);

		if (!bit_index.test(c_pos)) {
			auto x_pos(bit_index.find_above(c_pos, end_pos));
			auto p(ptr_at(x_pos));

			if (p) {
				if (key_compare(key_ref(p), key)) {
					c_begin = x_pos;
					continue;
				}

				c_end = x_pos;
				x_pos = bit_index.find_below(
					c_pos, begin_pos
				);
				auto q(ptr_at(x_pos));
				if (!q || key_compare(key_ref(q), key))
					return const_iterator(
						this, c_end
					);

				c_end = x_pos;
			} else {
				x_pos = bit_index.find_below(
					c_pos, begin_pos
				);
				p = ptr_at(x_pos);
				if (!p || key_compare(key_ref(p), key))
					return cend();

				c_end = x_pos;
			}
		} else {
			auto p(ptr_at(c_pos));
			if (key_compare(key_ref(p), key))
				c_begin = c_pos;
			else
				c_end = c_pos;
		}
	} while (c_begin != c_end);

	auto p(ptr_at(c_begin));
	return p ? const_iterator(this, c_begin) : cend();
}

template <
	typename ValueType, typename KeyOfValue,
	typename CompareF, typename Alloc, typename AllocPolicy
>
template <typename... Args>
auto flat_map_impl<
	ValueType, KeyOfValue, CompareF, Alloc, AllocPolicy
>::emplace_unique(Args&&... args) -> std::pair<iterator, bool>
{
	reserve(size() + 1);

}

}}}
#endif
