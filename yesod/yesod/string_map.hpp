/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_YESOD_STRING_MAP_20140106T1145)
#define UCPF_YESOD_STRING_MAP_20140106T1145

#include <yesod/pod_sparse_vector.hpp>
#include <yesod/detail/dense_encoding_map.hpp>
#include <yesod/detail/string_map_base.hpp>
#include <yesod/detail/string_map_ops.hpp>

namespace ucpf { namespace yesod {

template <typename CharType>
struct string_map_default_policy {
	typedef std::allocator<void> allocator_type;
	typedef std::char_traits<CharType> char_traits_type;
	typedef detail::dense_encoding_map<
		CharType, allocator_type
	> encoding_map;
	typedef typename encoding_map::index_type index_char_type;
	typedef pod_sparse_vector<> storage_type;
	typedef pod_sparse_vector_default_policy storage_policy;
	constexpr static size_t short_suffix_length = 16;
};

template <
	typename CharType, typename ValueType,
	typename Policy = string_map_default_policy<CharType>
> struct string_map;

}}
#endif

