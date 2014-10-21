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

#include <yesod/iterator/transform.hpp>

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
	: string_map(typename Policy::allocator_type())
	{}

	template <typename Alloc>
	string_map(Alloc const &a = Alloc())
	: items(a), tup_breadth_map(0, encoding_map_type(a))
	{
		init();
	}

	~string_map()
	{
		clear();
	}

	void clear()
	{
		auto a(items.get_allocator());
		items.for_each(0, [a](size_type pos, pair_type &p) -> bool {
				if (p.is_leaf())
					value_pair::destroy(a, p.leaf_ptr());

				return false;
		});
		init();
	}

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
	const_pointer find(Iterator first_, Iterator last_) const
	{
		c_encoding_adapter<
			decltype(tup_breadth_map)
		> map(tup_breadth_map);

		auto first(iterator::make_transform(first_, map));
		auto last(iterator::make_transform(last_, map));

		auto rv(find_impl(first, last));
		if (!rv.first)
			return nullptr;

		/* Check for virtual key terminator (will appear if
		 * one key is a substring of another).
		 */
		if (!rv.first->is_leaf())
			rv = rv.first->child_at(
				items, rv.second, terminator_index
			);

		if (rv.first->is_leaf()) {
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

	template <typename StringType, typename... Args>
	std::pair<reference, bool> emplace(StringType &&s, Args&&... args)
	{
		return emplace(
			std::begin(s), std::end(s), std::forward<Args>(args)...
		);
	}

	template <typename Iterator, typename... Args>
	std::pair<reference, bool> emplace(
		Iterator first_, Iterator last_, Args&&... args
	);

	template <typename StringType>
	size_type erase(StringType &&s)
	{
		return erase(std::begin(s), std::end(s));
	}

	template <typename Iterator>
	size_type erase(Iterator first_, Iterator last_);

	template <typename StringType>
	size_type erase_prefix(StringType &&s)
	{
		return erase_prefix(std::begin(s), std::end(s));
	}

	template <typename Iterator>
	size_type erase_prefix(Iterator first_, Iterator last_);

	template <typename Pred>
	bool for_each(Pred &&pred);

	template <typename Pred>
	bool for_each(Pred &&pred) const;

	template <typename StringType, typename Pred>
	bool for_each(StringType &&s, Pred &&pred)
	{
		return for_each(
			std::begin(s), std::end(s), std::forward<Pred>(pred)
		);
	}

	template <typename StringType, typename Pred>
	bool for_each(StringType &&s, Pred &&pred) const
	{
		return for_each(
			std::begin(s), std::end(s), std::forward<Pred>(pred)
		);
	}

	template <typename Iterator, typename Pred>
	bool for_each(Iterator first_, Iterator last_, Pred &&pred);

	template <typename Iterator, typename Pred>
	bool for_each(Iterator first_, Iterator last_, Pred &&pred) const;

	std::basic_ostream<
		CharType, typename Policy::char_traits_type
	> &dump(
		std::basic_ostream<
			CharType, typename Policy::char_traits_type
		> &os
	) const;

private:
	struct pair_type;
	struct alignas(uintptr_t) value_pair;
	typedef typename Policy::encoding_map encoding_map_type;
	typedef std::pair<pair_type *, uintptr_t> pair_loc;
	typedef std::pair<pair_type const *, uintptr_t> c_pair_loc;
	constexpr static uintptr_t terminator_index = 1;
	constexpr static uintptr_t base_index = 2;

	struct pair_type {
		bool is_leaf() const
		{
			return !(base & 1);
		}

		bool is_vacant() const
		{
			return !(check & 1);
		}

		template <typename StorageType>
		pair_loc pair_at(
			StorageType &items, uintptr_t char_index
		)
		{
			auto pos = (base >> 1) + char_index;
			return pair_loc(items.ptr_at(pos), pos);
		}

		template <typename StorageType>
		c_pair_loc child_at(
			StorageType const &items, uintptr_t pos,
			uintptr_t char_index
		) const
		{
			c_pair_loc rv(nullptr, (base >> 1) + char_index);

			auto p(items.ptr_at(rv.second));
			if (p && ((p->check >> 1) == pos))
				rv.first = p;

			return rv;
		}

		uintptr_t base_offset() const
		{
			return base >> 1;
		}

		uintptr_t parent() const
		{
			return check >> 1;
		}

		value_pair const *leaf_ptr() const
		{
			return reinterpret_cast<value_pair const *>(base);
		}

		value_pair *leaf_ptr()
		{
			return reinterpret_cast<value_pair *>(base);
		}

		void set_base_offset(uintptr_t base_)
		{
			base = (base_ << 1) | 1;
		}

		void set_parent(uintptr_t parent, bool occupied)
		{
			check = (parent << 1) | (occupied ? 1 : 0);
		}

		static pair_type make_leaf(value_pair *leaf, uintptr_t parent)
		{
			return pair_type{
				reinterpret_cast<uintptr_t>(leaf),
				(parent << 1) | 1
			};
		}
		static pair_type make_vacant(uintptr_t next, uintptr_t prev)
		{
			return pair_type{(next << 1) | 1, prev << 1};
		}

		static pair_type make(uintptr_t base_, uintptr_t parent)
		{
			return pair_type{
				(base_ << 1) | 1, (parent << 1) | 1
			};
		}

		uintptr_t base;
		uintptr_t check;
	};

	template <typename EncTuple>
	struct encoding_adapter {
		encoding_adapter()
		: tup(nullptr)
		{}

		encoding_adapter(EncTuple &tup_)
		: tup(&tup_)
		{}

		index_char_type operator()(char_type c) const
		{
			auto id(std::get<1>(*tup).index(c));
			auto next_id(std::get<0>(*tup));
			if (id < next_id)
				return id;
			else {
				std::get<1>(*tup).set(c, next_id);
				++std::get<0>(*tup);
				return next_id;
			}
		}

		EncTuple *tup;
	};

	template <typename EncTuple>
	struct c_encoding_adapter {
		c_encoding_adapter()
		: tup(nullptr)
		{}

		c_encoding_adapter(EncTuple const &tup_)
		: tup(&tup_)
		{}

		index_char_type operator()(char_type c) const
		{
			return std::get<1>(*tup).index(c);
		}

		EncTuple const *tup;
	};

	struct alignas(uintptr_t) value_pair {
		template <
			typename Alloc, typename IndexIterator,
			typename... Args
		> static value_pair *construct(
			Alloc const &a, IndexIterator first,
			IndexIterator last, Args&&... args
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

		template <typename IndexIterator>
		size_type common_length(
			IndexIterator first, IndexIterator last
		) const
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

		template <typename IndexIterator>
		bool prefix_match(
			IndexIterator first, IndexIterator last
		) const
		{
			return std::distance(first, last) == common_key_length(
				first, last
			);
		}

		template <typename IndexIterator>
		bool match(IndexIterator first, IndexIterator last) const
		{
			return (
				suffix_length == std::distance(first, last)
			) && (suffix_length == common_length(first, last));
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

	template <typename IndexIterator>
	static uintptr_t index_offset(IndexIterator iter)
	{
		return base_index + *iter;
	}

	char_type offset_char(uintptr_t index) const
	{
		return std::get<1>(tup_breadth_map).value(index - base_index);
	}

	void init()
	{
		items.clear();
		items.emplace(1, pair_type::make_vacant(1, 1));
		items.emplace(0, pair_type::make(0, 1));
	}

	template <typename IndexIterator>
	c_pair_loc find_impl(
		IndexIterator &first, IndexIterator const &last
	) const
	{
		c_pair_loc rv(items.ptr_at(0), 0);

		while (first != last) {
			rv = rv.first->child_at(
				items, rv.second, index_offset(first)
			);

			if (!rv.first || rv.first->is_leaf())
				break;

			++first;
		}

		return rv;
	}

	uintptr_t reserve_vacant(
		uintptr_t parent_pos, uintptr_t child_pos, value_pair *v
	);

	void detangle(
		pair_loc parent_loc, pair_loc child_loc, uintptr_t c_index,
		value_pair *v
	);

	void unroll_suffix(
		pair_loc loc, size_type count, uintptr_t other_index,
		value_pair *v
	);

	typename Policy::storage_type::template rebind<
		pair_type, typename Policy::storage_policy
	>::other items;
	std::tuple<uintptr_t, encoding_map_type> tup_breadth_map;
};

}}
#endif
