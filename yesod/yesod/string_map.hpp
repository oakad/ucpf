/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_YESOD_STRING_MAP_JAN_06_2014_1145)
#define UCPF_YESOD_STRING_MAP_JAN_06_2014_1145

#include <yesod/detail/string_map_base.hpp>
#include <yesod/detail/string_map_ops.hpp>

namespace ucpf { namespace yesod {

template <typename CharType>
struct default_string_map_policy {
	typedef std::allocator<void> allocator_type;
	typedef std::char_traits<CharType> char_traits_type;
	typedef typename std::make_unsigned<
		typename char_traits_type::char_type
	>::type index_char_type;

	constexpr static size_t short_suffix_length = 16;
	constexpr static size_t ptr_node_order = 4;
	constexpr static size_t trie_node_order = 8;
};

template <
	typename CharType, typename ValueType,
	typename Policy = default_string_map_policy<CharType>
> struct string_map;

}}
#endif

