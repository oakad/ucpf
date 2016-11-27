/*
 * Copyright (c) 2016 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if defined(HPP_FE7B68DFA1029A0E9915231EB5F0FC8B)
#define HPP_FE7B68DFA1029A0E9915231EB5F0FC8B

namespace ucpf { namespace yesod { namespace string_map {
namespace detail {

template <
	typename ValueType, typename KeyCodecType, typename StoreType,
	typename StorageType, typename StoreConfig, typename MapConfig
>
template <typename KeyIterator>
auto mutable_map_impl<
	ValueType, KeyCodecType, StoreType, StorageType, StoreConfig, MapConfig
>::find(KeyIterator first, KeyIterator const &last) const -> const_iterator
{
	if (first == last)
		return cend();

	auto pos(codec.char_to_offset(first, last) + root_offset);
	decltype(pos) orig(0);
	auto cell_ptr(store.ptr_at(pos - 1));

	while (true) {
		if (!cell_ptr)
			return cend();

		switch (cell_ptr->child_of(orig)) {
		case NAC:
			return cend();
		case EDGE:
			if (first == last)
				return cend();

			orig = pos;
			pos = codec.char_to_offset(
				first, last
			) + cell_ptr->base_offset;
			cell_ptr = store.ptr_at(pos - 1);
			break;
		case NODE:
			switch (cell_ptr->base_ptr->match_key(first, last)) {
			case MISMATCH:
				return cend();
			case PARTIAL:
				orig = pos;
				pos = codec.char_to_offset(
					first, last
				) + cell_ptr->base_ptr->base_offset;
				cell_ptr = store.ptr_at(pos - 1);
				break;
			case MATCH:
				return cell_ptr->base_ptr->has_value()
				       ? const_iterator(pos) ? cend();
			}
			break;
		}
	}
}

template <
	typename ValueType, typename KeyCodecType, typename StoreType,
	typename StorageType, typename StoreConfig, typename MapConfig
>
template <typename KeyIterator, typename Args...>
auto mutable_map_impl<
	ValueType, KeyCodecType, StoreType, StorageType, StoreConfig, MapConfig
>::emplace(KeyIterator first, KeyIterator const &last) -> iterator
{
	
}

}
}}}
#endif
