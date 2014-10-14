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

	string_map(allocator_type const &a = allocator_type())
	: root{0, 0}, items(a), tup_breadth_map(0, Policy::encoding_map(a))
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
			auto p(items.ptr_at(vec_offset(n_pos)));
			if (
				p && (p->check == rv.second)
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

		value_pair const *leaf_ptr() const
		{
			return reinterpret_cast<value_pair const *>(base);
		}

		value_pair *leaf_ptr()
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

	constexpr static uintptr_t terminator_index = 1;
	constexpr static uintptr_t base_index = 2;

	template <typename Iterator>
	static uintptr_t deref_char(Iterator const &iter)
	{
		return static_cast<uintptr_t>(*iter) + base_index;
	}

	/* logical:  0 1 2 3 4 5...
	 * physical: r 0 1 2 3 4...
	 * encoded:  1 3 5 7 9 11...
	 *
	 * encoded v -> encoded (v + c)
	 * offset 1 is a virtual string terminator
	 */
	static uintptr_t char_offset(uintptr_t v, uintptr_t c)
	{
		return v + (c << 1);
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

	index_char_type char_index(char_type c) const
	{
		return std::min(
			size_type(std::get<1>(tup_breadth_map).index(c)),
			std::get<0>(tup_breadth_map)
		);
	}

	index_char_type assign_char_index(char_type c)
	{
		auto id(std::get<1>(tup_breadth_map).index(c));
		if (id >= std::get<0>(tup_breadth_map)) {
			id = std::get<0>(tup_breadth_map);
			std::get<1>(tup_breadth_map).set(c, id);
			++std::get<0>(tup_breadth_map);
		}
		return id;
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
		template <
			typename Alloc, typename Encoding, typename Iterator,
			typename... Args
		> static value_pair *construct(
			Alloc const &a, Encoding const &e,
			Iterator first, Iterator last, Args&&... args
		);

		template <typename Alloc>
		static void destroy(Alloc const &a, value_pair *p);

		template <typename Alloc>
		void shrink_suffix(Alloc const &a, size_type count);

		index_char_type *suffix()
		{
			if (suffix_length <= Policy::short_suffix_length)
				return short_suffix;
			else
				return long_suffix.data + long_suffix.offset;
		}

		index_char_type const *suffix() const
		{
			if (suffix_length <= Policy::short_suffix_length)
				return short_suffix;
			else
				return long_suffix.data + long_suffix.offset;
		}

		template <typename Encoding, typename Iterator>
		size_type common_length(
			Encoding const &e, Iterator first, Iterator last
		) const
		{
			size_type pos(0);
			auto s_ptr(suffix());

			while ((pos < suffix_length) && (first != last)) {
				if (e.value(s_ptr[pos]) != *first)
					break;

				++first;
				++pos;
			}
			return pos;
		}

		template <typename Encoding, typename Iterator>
		bool prefix_match(
			Encoding const &e, Iterator first, Iterator last
		) const
		{
			return std::distance(first, last) == common_key_length(
				e, first, last
			);
		}

		template <typename Encoding, typename Iterator>
		bool match(
			Encoding const &e, Iterator first, Iterator last
		) const
		{
			return (
				suffix_length == std::distance(first, last)
			) && (suffix_length == common_length(e, first, last));
		}

		size_type suffix_length;
		value_type value;

		union {
			struct {
				index_char_type *data;
				size_type offset;
			} long_suffix;

			index_char_type short_suffix[
				Policy::short_suffix_length
			];
		};

		template <typename... Args>
		value_pair(Args&&... args)
		: suffix_length(0), value(std::forward<Args>(args)...),
		  long_suffix{nullptr, 0}
		{}

		~value_pair()
		{}
	};

	typename Policy::storage_type::template rebind<
		pair_type, typename Policy::storage_policy
	>::other items;
	std::tuple<size_type, Policy::encoding_map> tup_breadth_map;
};

}}
#endif
