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
	typename KeyType, typename ValueType, typename KeyOfValue,
	typename CompareF, typename Alloc
> void flat_map_impl<
	KeyType, ValueType, KeyOfValue, CompareF, Alloc
>::reserve(size_type cnt) const
{
	auto sz(AllocPolicy::best_size(word_sz + 1));
}

template <
	typename KeyType, typename ValueType, typename KeyOfValue,
	typename CompareF, typename Alloc
> auto flat_map_impl<
	KeyType, ValueType, KeyOfValue, CompareF, Alloc
>::lower_bound(key_type const &key) const -> const_iterator
{
	if (end_pos <= begin_pos)
		return cend();

	auto c_begin(begin_pos), c_end(end_pos - 1);

	do {
		auto c_pos((c_begin + c_end) >> 1);

		if (!bit_index.test(c_pos)) {
			auto x_pos(bit_index.find_above(pos, end_pos));
			auto p(ptr_at(x_pos));

			if (p) {
				if (key_compare(key_ref(p), key)) {
					c_begin = x_pos;
					continue;
				}

				c_end = x_pos;
				x_pos = bit_index.find_below(
					pos, begin_pos
				);
				auto q(ptr_at(x_pos));
				if (!q || key_compare(key_ref(q), key))
					return const_iterator(
						this, c_end
					);

				c_end = x_pos;
			} else {
				x_pos = bit_index.find_below(
					pos, begin_pos
				);
				p = ptr_at(x_pos);
				if (!p || key_compare(key_ref(p), key))
					return cend();

				c_end = x_pos;
			}
		} else {
			auto p(ptr_at(x_pos));
			if (key_compare(key_ref(p), key))
				c_begin = x_pos;
			else
				c_end = x_pos;
		}
	} while (c_begin != c_end);

	auto p(ptr_at(c_begin));
	return p ? const_iterator(this, c_begin) : cend();
}

template <
	typename KeyType, typename ValueType, typename KeyOfValue,
	typename CompareF, typename Alloc
>
template <typename... Args>
auto flat_map_impl<
	KeyType, ValueType, KeyOfValue, CompareF, Alloc
>::emplace_unique(Args&&... args) -> std::pair<iterator, bool>
{
	reserve(size() + 1);

}

}}}
#endif
