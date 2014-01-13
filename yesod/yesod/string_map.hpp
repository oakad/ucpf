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
	>::template rebind_traits<value_type> allocator_traits;

	typedef typename allocator_type::reference reference;
	typedef typename allocator_type::const_reference const_reference;
	typedef typename allocator_traits::size_type size_type;

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
		typedef typename value_pair::allocator_traits
		vp_allocator_traits;
		typedef typename value_pair::char_allocator_traits
		char_allocator_traits;

		value_pair::allocator_type a(trie.get_allocator());
		value_pair::char_allocator_type ca(a);
		auto suffix_length(last - first);
		decltype(suffix_length) init_length(0);

		std::unique_ptr<
			char_type[], std::function<void (char_type *)>
		> s(
			nullptr, [
				&ca, &init_length, suffix_length
			](char_type *p) -> void {
				for (auto cnt(init_length); cnt; --cnt)
					char_allocator_traits::destroy(
						ca, &p[cnt - 1]
					);

				char_allocator_traits::deallocate(
					ca, p, suffix_length
				);
			}
		);

		if (suffix_length > Policy::short_suffix_length) {
			s.reset(char_allocator_traits::allocate(
				ca, suffix_length
			));
			for (; init_length < suffix_length; ++init_length) {
				char_allocator_traits::construct(
					ca, &s[init_length], *first
				);
				++first;
			}
		}

		auto v(vp_allocator_traits::allocate(a, 1));
		vp_allocator_traits::construct(
			a, v, std::forward<Args>(args)...
		);

		if (s) {
			v.long_suffix.data = s.release();
			v.long_suffix.offset = 0;
		} else {
			std::unique_ptr<
				value_pair, std::function<void (value_pair *)>
			> uv(
				v, [
					&a, &ca, &init_length, suffix_length
				](value_pair *p) -> void {
					for (auto cnt(init_length); cnt; --cnt)
						char_allocator_traits::destroy(
							ca, &p->short_suffix[
								cnt - 1
							]
						);
					vp_allocator_traits::destroy(
						a, p
					);
					vp_allocator_traits::deallocate(
						a, p, 1
					);
				}
			);

			uv.release();
		}

		v->suffix_length = suffix_length;
		p.first = reinterpret_cast<uintptr_t>(v);
		p.second = 1;
		return v->value;
	}

	void destroy_value(value_pair *p)
	{
		value_pair::allocator_type a(trie.get_allocator());
		value_pair::char_allocator_type ca(a);

		if (p->suffix_length > Policy::short_suffix_length) {
			for (
				auto cnt(p->suffix_length);
				cnt > p->long_suffix.offset;
				--cnt
			)
				value_pair::char_allocator_traits::destroy(
					ca, &p->long_suffix.data[cnt - 1]
				);

			value_pair::char_allocator_traits::deallocate(
				ca, p->data, p->suffix_length
			);
		} else {
			for (auto cnt(p->suffix_length); cnt; --cnt)
				value_pair::char_allocator_traits::destroy(
					ca, &p->short_suffix[cnt - 1]
				);
		}
		value_pair::allocator_traits::destroy(a, p);
		value_pair::allocator_traits::deallocate(a, p, 1);
	}

	struct alignas(uintptr_t) value_pair {
		typedef typename std::allocator_traits<
			typename Policy::allocator_type
		>::template rebind_alloc<value_pair> allocator_type;
		typedef typename std::allocator_traits<
			typename Policy::allocator_type
		>::template rebind_alloc<char_type> char_allocator_type;

		typedef typename std::allocator_traits<
			typename Policy::allocator_type
		>::template rebind_traits<value_pair> allocator_traits;
		typedef typename std::allocator_traits<
			typename Policy::allocator_type
		>::template rebind_traits<char_type> char_allocator_traits;

		template <typename... Args>
		value_pair(Args&&... args)
		: suffix_length(0), value(std::forward<Args>(args...))
		{}

		size_type suffix_length;
		value_type value;

		union {
			struct {
				char_type *data;
				size_type offset;
			} long_suffix;

			char_type short_suffix[Policy::short_suffix_length];
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

