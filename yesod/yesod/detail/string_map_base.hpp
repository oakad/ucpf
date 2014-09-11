/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 *
 * Based on algorithm from:
 *
 *      An Efficient Implementation of Trie Structures (Jun-Ichi Aoe,
 *      Katsushi Morimoto, Takashi Sato) in Software - Practice and Experience,
 *      Vol. 22(9), 695 - 721 (September 1992)
 *
 */
#if !defined(UCPF_YESOD_DETAIL_STRING_MAP_BASE_JAN_06_2014_1145)
#define UCPF_YESOD_DETAIL_STRING_MAP_BASE_JAN_06_2014_1145

#include <vector>
#include <forward_list>
#include <unordered_map>
#include <scoped_allocator>

#include <yesod/flat_map.hpp>
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
	: trie_root(1)
	{}

	template <typename StringType, typename... Args>
	std::pair<reference, bool> emplace_at(StringType &&s, Args&&... args)
	{
		return emplace_at(
			std::begin(s), std::end(s), std::forward<Args>(args)...
		);
	}

	template <typename Iterator, typename... Args>
	std::pair<reference, bool> emplace_at(
		Iterator first, Iterator last, Args&&... args
	);

	template <typename StringType>
	pointer find(StringType &&s)
	{
		return find(std::begin(s), std::end(s));
	}

	template <typename StringType>
	const_pointer find(StringType &&s) const
	{
		return find(std::begin(s), std::end(s));
	}

	template <typename Iterator>
	const_pointer find(Iterator first, Iterator last) const
	{
		auto rv(find_impl(first, last));
		if (!rv.first)
			return nullptr;

		if (!rv.first->is_leaf()) {
			/* Check for virtual key terminator (will appear if
			 * one key is a substring of another).
			 */
			auto n_pos(char_offset(
				rv.first->base, terminator_char
			));
			auto p(trie.ptr_at(vec_offset(n_pos)));
			if (
				p && p->base && (p->check == rv.second)
				&& p->is_leaf()
			) {
				auto vp(p->leaf_ptr());
				if (vp->match(first, last))
					return &vp->value;
			}
		} else {
			auto vp(rv.first->leaf_ptr());
			if (vp->match(first, last))
				return &vp->value;
		}

		return nullptr;
	}

	template <typename Iterator>
	pointer find(Iterator first, Iterator last)
	{
		auto rv(
			const_cast<string_map const *>(this)->find(first, last)
		);
		return const_cast<pointer>(rv);
	}

	std::basic_ostream<
		CharType, typename Policy::char_traits_type
	> &dump(
		std::basic_ostream<
			CharType, typename Policy::char_traits_type
		> &os
	) const;

private:
	struct alignas(uintptr_t) value_pair;

	struct pair_type {
		uintptr_t base;
		uintptr_t check;

		bool is_leaf() const
		{
			return !(base & 1);
		}

		value_pair* leaf_ptr() const
		{
			return reinterpret_cast<value_pair *>(base);
		}

		static pair_type make(uintptr_t base_, uintptr_t check_)
		{
			return pair_type{base_, check_};
		}

		static pair_type make(value_pair *base_, uintptr_t check_)
		{
			return pair_type{
				reinterpret_cast<uintptr_t>(base_), check_
			};
		}
	};

	typedef std::forward_list<
		pair_type *,
		typename std::allocator_traits<
			typename Policy::allocator_type
		>::template rebind_alloc<pair_type *>
	> pair_ptr_list;

	typedef std::tuple<
		pair_type *, uintptr_t, pair_ptr_list
	> index_entry_type;

	typedef std::vector<
		index_entry_type,
		typename std::allocator_traits<
			typename Policy::allocator_type
		>::template rebind_alloc<index_entry_type>
	> index_entry_set;

	constexpr static uintptr_t terminator_char = 1;
	constexpr static uintptr_t null_char = 2;

	template <typename Iterator>
	static index_char_type deref_char(Iterator const &iter)
	{
		return static_cast<index_char_type>(*iter);
	}

	/* logical:  0 1 2 3 4 5...
	 * physical: r 0 1 2 3 4...
	 * encoded:  1 3 5 7 9 11...
	 *
	 * encoded v -> encoded (v + c)
	 * offset 1 is a virtual string terminator
	 */
	static uintptr_t char_offset(uintptr_t v, index_char_type c)
	{
		return v + ((uintptr_t(c) + null_char) << 1);
	}

	/* encoded -> physical */
	static uintptr_t vec_offset(uintptr_t v)
	{
		return (v - 3) >> 1;
	}

	/* physical -> encoded */
	static uintptr_t log_offset(uintptr_t v)
	{
		return (v << 1) + 3;
	}

	/* encoded + physical -> encoded */
	static uintptr_t adjust_encoded(uintptr_t v, uintptr_t adj)
	{
		return v + (adj << 1);
	}

	/* physical - encoded -> char */
	static uintptr_t offset_to_char(uintptr_t v, uintptr_t adj)
	{
		return (log_offset(v) - adj) >> 1;
	}

	template <typename Iterator>
	std::pair<pair_type const *, uintptr_t> find_impl(
		Iterator &first, Iterator const &last
	) const;

	std::pair<pair_type *, uintptr_t> unroll_key(
		pair_type *p, uintptr_t pos, size_type count, uintptr_t other
	);

	std::pair<pair_type *, uintptr_t> split_subtree(
		uintptr_t r_pos, uintptr_t l_pos, uintptr_t k_char
	);

	uintptr_t advance_edges(
		uintptr_t pos, index_entry_set &b_set, uintptr_t k_char
	);

	struct alignas(uintptr_t) value_pair {
		template <typename Alloc, typename Iterator, typename... Args>
		static value_pair *construct(
			Alloc const &a, Iterator first, Iterator last,
			Args&&... args
		);

		template <typename Alloc>
		static void destroy(Alloc const &a, value_pair *p);

		template <typename Alloc>
		void shrink_suffix(Alloc const &a, size_type count);

		char_type *suffix()
		{
			if (suffix_length <= Policy::short_suffix_length)
				return short_suffix;
			else
				return long_suffix.data + long_suffix.offset;
		}

		char_type const *suffix() const
		{
			if (suffix_length <= Policy::short_suffix_length)
				return short_suffix;
			else
				return long_suffix.data + long_suffix.offset;
		}

		template <typename Iterator>
		bool match(Iterator first, Iterator last) const
		{
			return (suffix_length == std::distance(first, last))
			       && (suffix_length == common_length(first, last));
		}

		template <typename Iterator>
		size_type common_length(Iterator first, Iterator last) const
		{
			size_type pos(0);
			auto s_ptr(suffix());

			while ((pos < suffix_length) && (first != last)) {
				if (s_ptr[pos] != *first)
					break;

				++first;
				++pos;
			}
			return pos;
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

		template <typename... Args>
		value_pair(Args&&... args)
		: suffix_length(0), value(std::forward<Args>(args)...),
		  long_suffix{nullptr, 0}
		{}

		~value_pair()
		{}
	};

	struct pair_valid_pred 
	{
		static bool test(pair_type const &p)
		{
			return p.check != 0;
		}
	};

	struct trie_vector_policy {
		typedef typename Policy::allocator_type allocator_type;

		constexpr static size_t
		ptr_node_order = Policy::ptr_node_order;

		constexpr static size_t
		data_node_order = Policy::trie_node_order;

		typedef pair_valid_pred value_valid_pred;
	};

	uintptr_t trie_root;
	sparse_vector<pair_type, trie_vector_policy> trie;

public:
	struct reverse_index {
		typedef std::basic_string<
			char_type, typename Policy::char_traits_type,
			typename std::allocator_traits<
				typename Policy::allocator_type
			>::template rebind_alloc<char_type>
		> prefix_string_type;

		template <typename Pred>
		void for_each(Pred &&pred) const;

		template <typename Iterator, typename Pred>
		void for_each_prefix(
			Iterator first, Iterator last, Pred &&pred
		) const;

		template <typename StringType, typename Pred>
		void for_each_prefix(StringType &&s, Pred &&pred) const
		{
			for_each_prefix(
				std::begin(s), std::end(s),
				std::forward<Pred>(pred)
			);
		}

	private:
		friend string_map;

		struct r_node {
			r_node()
			: pair(nullptr), pos(0), char_id(0)
			{}

			r_node(
				pair_type const *pair_, uintptr_t pos_,
				uintptr_t char_id_
			) : pair(pair_), pos(pos_), char_id(char_id_)
			{}

			pair_type const *pair;
			uintptr_t pos;
			uintptr_t char_id;
		};

		typedef std::vector<r_node, typename std::allocator_traits<
			typename Policy::allocator_type
		>::template rebind_alloc<r_node>> r_node_vec;

		typedef std::scoped_allocator_adaptor<
			typename std::allocator_traits<
				typename Policy::allocator_type
			>::template rebind_alloc<std::pair<
				uintptr_t, r_node_vec
			>>, typename r_node_vec::allocator_type
		> trie_allocator_adaptor;

		typedef std::unordered_map<
			uintptr_t, r_node_vec, std::hash<uintptr_t>,
			std::equal_to<uintptr_t>, trie_allocator_adaptor
		> r_trie_type;

		struct state {
			typedef typename prefix_string_type::allocator_type
			allocator_type;

			state(
				typename r_trie_type::const_iterator r_pos_,
				typename r_node_vec::const_iterator b_pos_,
				allocator_type const &a
			) : r_pos(r_pos_), b_pos(b_pos_), prefix(a)
			{}

			state(allocator_type const &a)
			: prefix(a)
			{}

			state(state const &other, allocator_type const &a)
			: prefix(other.prefix), r_pos(other.r_pos),
			  b_pos(other.b_pos)
			{}

			state(state &&other, allocator_type const &a)
			: prefix(std::move(other.prefix)),
			  r_pos(std::move(other.r_pos)),
			  b_pos(std::move(other.b_pos))
			{}

			prefix_string_type prefix;
			typename r_trie_type::const_iterator r_pos;
			typename r_node_vec::const_iterator b_pos;
		};

		typedef std::scoped_allocator_adaptor<
			typename std::allocator_traits<
				typename Policy::allocator_type
			>::template rebind_alloc<state>,
			typename state::allocator_type
		> state_allocator_adaptor;

		typedef std::vector<
			state, state_allocator_adaptor
		> state_stack_type;

		reverse_index(
			string_map const &parent_,
			typename decltype(trie)::allocator_type const &a
		) : parent(parent_), r_trie(
			10, typename r_trie_type::hasher(),
			typename r_trie_type::key_equal(),
			trie_allocator_adaptor(a, a)
		) {}

		template <typename Pred>
		void for_each_impl(state_stack_type &ss, Pred &&pred) const;

		string_map const &parent;
		r_trie_type r_trie;
	};

	reverse_index make_index() const;
};

}}
#endif
