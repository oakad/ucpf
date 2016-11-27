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

	struct locus {
		locus()
		: offset(~uintptr_t(0)), leaf_pos(0)
		{}

		explicit operator bool() const
		{
			return offset < ~uintptr_t(0);
		}

	private:
		friend struct string_map;

		locus(uintptr_t offset_, uintptr_t leaf_pos_)
		: offset(offset_), leaf_pos(leaf_pos_)
		{}

		uintptr_t offset;
		uintptr_t leaf_pos;
	};

	string_map()
	: string_map(typename Policy::allocator_type())
	{}

	template <typename Alloc>
	string_map(Alloc const &a = Alloc())
	: items(a), tup_breadth_map(0, std::move(encoding_map_type(a)))
	{
		init();
	}

	~string_map()
	{
		clear_impl();
	}

	void clear()
	{
		clear_impl();
		init();
	}

	template <typename ValueClearFunc>
	void clear(ValueClearFunc &&func)
	{
		clear_impl(std::forward<ValueClearFunc>(func));
		init();
	}

	locus search_root() const
	{
		return locus(0, 0);
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
	const_pointer find(Iterator first, Iterator last) const
	{
		return find_rel(search_root(), first, last);
	}

	template <typename Iterator>
	pointer find(Iterator first, Iterator last)
	{
		return find_rel(search_root(), first, last);
	}

	template <typename StringType>
	pointer find_rel(locus base, StringType &&s)
	{
		return find_rel(base, std::begin(s), std::end(s));
	}

	template <typename StringType>
	const_pointer find_rel(locus base, StringType &&s) const
	{
		return find_rel(base, std::begin(s), std::end(s));
	}

	template <typename Iterator>
	pointer find_rel(locus base, Iterator first_, Iterator last_)
	{
		auto rv(
			const_cast<string_map const *>(this)->find_rel(
				base, first_, last_
			)
		);

		return const_cast<pointer>(rv);
	}

	template <typename Iterator>
	const_pointer find_rel(
		locus base, Iterator first_, Iterator last_
	) const;

	template <typename StringType>
	locus locate(StringType &&s) const
	{
		return locate(std::begin(s), std::end(s));
	}

	template <typename Iterator>
	locus locate(Iterator first_, Iterator last_) const
	{
		return locate_rel(search_root(), first_, last_);
	}

	template <typename StringType>
	locus locate_rel(locus base, StringType &&s) const
	{
		return locate_rel(base, std::begin(s), std::end(s));
	}

	template <typename Iterator>
	locus locate_rel(locus base, Iterator first_, Iterator last_) const;

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

	template <typename StringType>
	size_type count_terminations(locus base, StringType &&s) const
	{
		return count_terminations(base, std::begin(s), std::end(s));
	}

	template <typename Iterator>
	size_type count_terminations(
		locus base, Iterator first, Iterator last
	) const;

	size_type count_leaves(locus base) const;

	template <typename Pred>
	bool for_each(Pred &&pred)
	{
		key_string_type prefix(items.get_allocator());
		return for_each_impl<reference>(
			c_pair_loc(items.ptr_at(0), 0), prefix,
			std::forward<Pred>(pred)
		);
	}

	template <typename Pred>
	bool for_each(Pred &&pred) const
	{
		key_string_type prefix(items.get_allocator());
		return for_each_impl<const_reference>(
			c_pair_loc(items.ptr_at(0), 0), prefix,
			std::forward<Pred>(pred)
		);
	}

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
	bool for_each(Iterator first_, Iterator last_, Pred &&pred)
	{
		return for_each_prefix<reference>(
			first_, last_, std::forward<Pred>(pred)
		);
	}

	template <typename Iterator, typename Pred>
	bool for_each(Iterator first_, Iterator last_, Pred &&pred) const
	{
		return for_each_prefix<const_reference>(
			first_, last_, std::forward<Pred>(pred)
		);
	}

	std::ostream &dump(std::ostream &os) const;
	std::ostream &dump_internal(std::ostream &os) const
	{
		items.dump(os);
		auto ut(items.utilization([](auto &p) -> bool {
			return p.base || p.check;
		}));
		os << "Storage used: " << ut.first << " bytes, useful: ";
		os << ut.second << ", overhead ";
		os << ((double(ut.first) / ut.second) - 1) * 100 << "%\n";
		return os;
	}

	allocator_type get_allocator() const
	{
		return items.get_allocator();
	}

private:
	struct pair_type;
	struct alignas(uintptr_t) value_pair;
	typedef typename Policy::encoding_map encoding_map_type;
	typedef std::pair<pair_type *, uintptr_t> pair_loc;
	typedef std::pair<pair_type const *, uintptr_t> c_pair_loc;
	typedef std::vector<char_type, allocator_type> key_string_type;
	constexpr static uintptr_t terminator_index = 1;
	constexpr static uintptr_t base_index = 2;

	struct iter_loc {
		iter_loc(c_pair_loc const &loc)
		: index(terminator_index), base(loc.first->base_offset()),
		  pos((loc.second << 1) | 1)
		{}

		iter_loc(pair_loc const &loc)
		: index(terminator_index), base(loc.first->base_offset()),
		  pos((loc.second << 1) | 1)
		{}

		iter_loc(uintptr_t base_offset, uintptr_t pos_)
		: index(terminator_index), base(base_offset),
		  pos((pos_ << 1) | 1)
		{}

		template <typename StorageType>
		auto next_child(StorageType &items, uintptr_t breadth)
		{
			typedef typename std::conditional<
				std::is_const<StorageType>::value,
				c_pair_loc, pair_loc
			>::type result_type;

			while (true) {
				auto n_pos(base + index);
				if (index >= breadth)
					return result_type(nullptr, n_pos);

				auto p(items.ptr_at(n_pos));
				if (p && (p->check == pos))
					return result_type(p, n_pos);

				++index;
			}
		}

		uintptr_t position() const
		{
			return pos >> 1;
		}

		uintptr_t index;
	private:
		uintptr_t base;
		uintptr_t pos;
	};

	struct pair_type {
		friend std::ostream &operator<<(
			std::ostream &os, pair_type const &p
		)
		{
			if (p.is_vacant()) {
				os << "vacant next: " << p.base_offset();
				os << ", prev: " << p.parent();
			} else if (p.is_leaf()) {
				os << "leaf ptr: " << p.leaf_ptr();
				os << ", parent: " << p.parent();
			} else {
				os << "link base: " << p.base_offset();
				os << ", parent: " << p.parent();
			}
			return os;
		}

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

		template <typename StorageType>
		pair_loc child_at(
			StorageType &items, uintptr_t pos,
			uintptr_t char_index
		) const
		{
			pair_loc rv(nullptr, (base >> 1) + char_index);

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

		constexpr static pair_type make_leaf(
			value_pair *leaf, uintptr_t parent
		)
		{
			return pair_type{
				reinterpret_cast<uintptr_t>(leaf),
				(parent << 1) | 1
			};
		}

		constexpr static pair_type make_vacant(
			uintptr_t next, uintptr_t prev
		)
		{
			return pair_type{(next << 1) | 1, prev << 1};
		}

		constexpr static pair_type make(
			uintptr_t base_, uintptr_t parent
		)
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

	template <typename EncTuple>
	struct c_decoding_adapter {
		c_decoding_adapter()
		: tup(nullptr)
		{}

		c_decoding_adapter(EncTuple const &tup_)
		: tup(&tup_)
		{}

		char_type operator()(index_char_type c) const
		{
			return std::get<1>(*tup).value(c);
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
			IndexIterator first, IndexIterator last,
			uintptr_t offset
		) const
		{
			size_type pos(offset);
			auto s_ptr(suffix());

			while ((pos < suffix_length) && (first != last)) {
				if (s_ptr[pos] != *first)
					break;

				++first;
				++pos;
			}
			return pos - offset;
		}

		template <typename IndexIterator>
		bool match(IndexIterator first, IndexIterator last) const
		{
			if (suffix_length != size_type(
				std::distance(first, last)
			))
				return false;
			
			return std::equal(first, last, suffix());
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

	static uintptr_t index_offset(index_char_type c)
	{
		return base_index + c;
	}

	char_type offset_char(uintptr_t index) const
	{
		return std::get<1>(tup_breadth_map).value(index - base_index);
	}

	void init()
	{
		constexpr pair_type empty_map[2] = {
			pair_type::make(0, 1),
			pair_type::make_vacant(1, 1)
		};

		items.clear();
		items.for_each_pos(
			0, 2, [empty_map](auto pos, auto &item) -> void {
				item = empty_map[pos];
			}
		);
	}

	template <typename ValueClearFunc>
	void clear_impl(ValueClearFunc &&func)
	{
		auto a(items.get_allocator());
		auto last(items.ptr_at(0)->parent());
		items.for_each_pos(
			0, last,
			[a, func](size_type pos, pair_type &p) -> bool {
				if (p.is_leaf()) {
					auto leaf_ptr(p.leaf_ptr());
					func(leaf_ptr->value);
					value_pair::destroy(a, leaf_ptr);
				}

				return false;
			}
		);
	}

	void clear_impl()
	{
		auto a(items.get_allocator());
		auto last(items.ptr_at(0)->parent());
		items.for_each_pos(
			0, last, [a](size_type pos, pair_type &p) -> bool {
				if (p.is_leaf()) {
					auto leaf_ptr(p.leaf_ptr());
					value_pair::destroy(a, leaf_ptr);
				}

				return false;
			}
		);
	}

	template <typename IndexIterator>
	c_pair_loc find_impl(
		IndexIterator &first, IndexIterator const &last, uintptr_t base
	) const
	{
		c_pair_loc rv(items.ptr_at(base), base);

		while (first != last) {
			rv = rv.first->child_at(
				items, rv.second, index_offset(*first)
			);

			++first;

			if (!rv.first || rv.first->is_leaf())
				break;
		}

		return rv;
	}

	template <typename IndexIterator>
	pair_loc find_impl(
		IndexIterator &first, IndexIterator const &last, uintptr_t base
	)
	{
		pair_loc rv(items.ptr_at(base), base);

		while (first != last) {
			rv = rv.first->child_at(
				items, rv.second, index_offset(*first)
			);

			++first;

			if (!rv.first || rv.first->is_leaf())
				break;
		}

		return rv;
	}

	template <typename ValueRefType, typename PairType, typename Pred>
	bool for_each_impl(
		PairType first, key_string_type &prefix, Pred &&pred
	) const;

	template <typename ValueRefType, typename Iterator, typename Pred>
	bool for_each_prefix(
		Iterator first_, Iterator last_, Pred &&pred
	) const;

	void grow_storage(uintptr_t pos);

	uintptr_t find_vacant(uintptr_t first);

	uintptr_t find_vacant_set(
		uintptr_t first, uintptr_t const *index_set,
		size_type index_count
	);

	uintptr_t reserve_vacant(
		uintptr_t parent_pos, uintptr_t child_pos, value_pair *v
	);

	void release_pair(uintptr_t taken_pos);

	void release_ancestors(uintptr_t taken_pos);

	void move_pair(
		uintptr_t src_taken_pos, uintptr_t dst_vacant_pos,
		uintptr_t src_vacant_hint
	);

	void relocate(
		pair_loc loc, uintptr_t c_index, value_pair *v
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
