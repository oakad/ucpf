/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_YESOD_DETAIL_STRING_MAP_BASE_JAN_06_2014_1145)
#define UCPF_YESOD_DETAIL_STRING_MAP_BASE_JAN_06_2014_1145

#include <yesod/sparse_vector.hpp>

namespace ucpf { namespace yesod {

template <typename CharType, typename ValueType, typename Policy>
struct string_map {
	typedef typename Policy::char_traits_type::char_type char_type;
	typedef typename Policy::index_char_type index_char_type;
	typedef ValueType value_type;

	typedef typename std::allocator_traits<
		typename Policy::allocator_type
	>::template rebind_alloc<value_type> allocator_type;

	typedef typename std::allocator_traits<
		typename Policy::allocator_type
	>::template rebind_traits<value_type> allocator_traits;

	typedef typename allocator_type::reference reference;
	typedef typename allocator_type::const_reference const_reference;
	typedef typename allocator_traits::size_type size_type;
	typedef typename allocator_traits::pointer pointer;
	typedef typename allocator_traits::const_pointer const_pointer;


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
	);

	template <typename Iterator>
	pointer find(Iterator first, Iterator last)
	{
		uintptr_t l_pos(1);
		auto n_pos(log_offset(trie_root.first, deref_char(first)));
		++first;
		for (; first != last; ++first) {
			auto p(trie.ptr_at(vec_offset(n_pos)));
			if (!p || (p->second != l_pos))
				break;

			if (!(p->first & 1)) {
				auto v(reinterpret_cast<value_pair *>(
					p->first
				));
				if (v->match(first, last))
					return &v->value;
			}
		}

		return nullptr;
	}

	template <typename Iterator>
	const_pointer find(Iterator first, Iterator last) const
	{
		uintptr_t l_pos(1);
		auto n_pos(log_offset(trie_root.first, deref_char(first)));
		++first;
		for (; first != last; ++first) {
			auto p(trie.ptr_at(vec_offset(n_pos)));
			if (!p || (p->second != l_pos))
				break;

			if (!(p->first & 1)) {
				auto v(reinterpret_cast<value_pair const *>(
					p->first
				));
				if (v->match(first, last))
					return &v->value;
			}
		}

		return nullptr;
	}

	std::basic_ostream<
		char_type, typename Policy::char_traits_type
	> &dump(
		std::basic_ostream<
			char_type, typename Policy::char_traits_type
		> &os
	);

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

	struct alignas(uintptr_t) value_pair {
		typedef typename std::allocator_traits<
			typename Policy::allocator_type
		>::template rebind_alloc<value_pair> pair_allocator_type;
		typedef typename std::allocator_traits<
			typename Policy::allocator_type
		>::template rebind_alloc<char_type> char_allocator_type;

		typedef typename std::allocator_traits<
			typename Policy::allocator_type
		>::template rebind_traits<value_pair> pair_allocator_traits;
		typedef typename std::allocator_traits<
			typename Policy::allocator_type
		>::template rebind_traits<char_type> char_allocator_traits;

		template <typename Alloc, typename Iterator, typename... Args>
		static value_pair *construct(
			Alloc const &a_, Iterator first, Iterator last,
			Args&&... args
		);

		template <typename Alloc>
		static void destroy(Alloc const &a, value_pair *p);

		char_type *suffix()
		{
			if (suffix_length <= Policy::short_suffix_length)
				return short_suffix;
			else
				return long_suffix.data + offset;
		}

		char_type const *suffix() const
		{
			if (suffix_length <= Policy::short_suffix_length)
				return short_suffix;
			else
				return long_suffix.data + offset;
		}

		template <typename Iterator>
		bool match(Iterator first, Iterator last) const
		{
			if ((last - first) != suffix_length)
				return false;

			if (suffix_length <= Policy::short_suffix_length)
				return std::equal(first, last, short_suffix);
			else
				return std::equal(
					first, last,
					long_suffix.data + long_suffix.offset
				);
		}

		size_type suffix_length;
		value_type value;

		union {
			struct {
				char_type *data;
				size_type offset;
			} long_suffix;

			char_type short_suffix[Policy::short_suffix_length];
		};

	private:
		template <typename... Args>
		value_pair(Args&&... args)
		: suffix_length(0), value(std::forward<Args>(args)...),
		  long_suffix{nullptr, 0}
		{}

		~value_pair()
		{}
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
