/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_YESOD_STRING_MAP_JAN_06_2014_1145)
#define UCPF_YESOD_STRING_MAP_JAN_06_2014_1145

#include <yesod/sparse_vector.hpp>

namespace ucpf { namespace yesod {

template <typename CharType>
struct default_string_map_policy {
	typedef std::allocator<void> allocator_type;
	typedef std::char_traits<CharType> char_traits_type;

	constexpr static size_t short_suffix_length = 16;
	constexpr static size_t ptr_node_order = 4;
	constexpr static size_t trie_node_order = 8;
	constexpr static size_t value_node_order = 6;
};

template <
	typename CharType, typename ValueType,
	typename Policy = default_string_map_policy<CharType>
> struct string_map {
	typedef typename Policy::char_traits_type::char_type char_type;
	typedef ValueType value_type;

	typedef typename std::allocator_traits<
		typename Policy::allocator_type
	>::template rebind_alloc<value_type> allocator_type;

	typedef typename std::allocator_traits<
		typename Policy::allocator_type
	>::template rebind_traits<value_type> allocator_traits_type;

	typedef typename allocator_type::reference reference;
	typedef typename allocator_type::const_reference const_reference;
	typedef typename allocator_traits_type::size_type size_type;

	string_map()
	: trie_root(-1, 0)
	{}

	template <typename Iterator, typename... Args>
	reference emplace_at(
		Iterator first, Iterator last, Args&&... args
	)
	{
		for (; first != last; ++first) {
			
		}
	}

private:
	struct value_pair {
		union {
			struct {
				value_type *data;
				size_type offset;
			} long_suffix;

			std::array<
				char_type, Policy::short_suffix_length
			> short_suffix;
		};
		size_type key_length;
		value_type value;
	};

	struct trie_vector_policy {
		typedef typename Policy::allocator_type allocator_type;

		constexpr static size_t
		ptr_node_order = Policy::ptr_node_order;

		constexpr static size_t
		data_node_order = Policy::trie_node_order;
	};

	struct value_vector_policy {
		typedef typename Policy::allocator_type allocator_type;

		constexpr static size_t
		ptr_node_order = Policy::ptr_node_order;

		constexpr static size_t
		data_node_order = Policy::value_node_order;
	};

	std::pair<long, long> trie_root;
	sparse_vector<std::pair<long, long>, trie_vector_policy> trie;
	sparse_vector<value_pair, value_vector_policy> values;
};

}}
#endif

