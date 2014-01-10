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
> struct string_map {
	typedef typename Policy::char_traits_type::char_type char_type;
	typedef typename Policy::index_char_type index_char_type;
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
	: trie_root(3, 0)
	{}

/* logical:  1 2 3 4 5...
 * physical: r 0 1 2 3...
 * encoded:  3 5 7 9 11...
 */
	template <typename Iterator, typename... Args>
	reference emplace_at(
		Iterator first, Iterator last, Args&&... args
	)
	{
		auto l_pos(1);
		auto n_pos(log_offset(trie_root.first, deref_char(first)));
		++first;
		for (; first != last; ++first) {
			auto &p(trie.at(vec_offset(n_pos)));
			if (!p.second) {
				p.second = l_pos;
				return emplace_value(
					p, first, last,
					std::forward<Args>(args)...
				);
			}
			
		}
	}

private:
	typedef std::pair<uintptr_t, uintptr_t> pair_type;

	template <typename Iterator>
	static index_char_type deref_char(Iterator const &iter)
	{
		return static_cast<index_char_type>(*iter);
	}

	static uintptr_t log_offset(uintptr_t v, index_char_type c)
	{
		return v + ((uintptr_t(c) + 1) << 1);
	}

	static uintptr_t vec_offset(uintptr_t v)
	{
		return (v >> 1) - 2;
	}

	template <typename Iterator, typename... Args>
	reference emplace_value(
		pair_type &p, Iterator first, Iterator last, Args&&... args
	)
	{
		
		if ((last - first) > Policy::short_suffix_length) {
			try {
			} catch(...)
		}
	}

	struct alignas(uintptr_t) value_pair {

		template <typename... Args>
		value_pair(Args&&... args)
		: value(std::forward<Args>(args...))
		{}

		size_type key_length;
		value_type value;

		union {
			struct {
				char_type *data;
				size_type offset;
			} long_suffix;

			std::array<
				char_type, Policy::short_suffix_length
			> short_suffix;
		};
	};

	struct trie_vector_policy {
		typedef typename Policy::allocator_type allocator_type;

		constexpr static size_t
		ptr_node_order = Policy::ptr_node_order;

		constexpr static size_t
		data_node_order = Policy::trie_node_order;
	};

	pair_type trie_root;
	sparse_vector<pair_type, trie_vector_policy> trie;
};

}}
#endif

