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
#if !defined(UCPF_YESOD_DETAIL_STRING_MAP_OPS_JAN_06_2014_1145)
#define UCPF_YESOD_DETAIL_STRING_MAP_OPS_JAN_06_2014_1145

namespace ucpf { namespace yesod {

template <typename CharType, typename ValueType, typename Policy>
template <typename Iterator, typename... Args>
auto string_map<CharType, ValueType, Policy>::emplace_at(
	Iterator first_, Iterator last_, Args&&... args
) -> std::pair<reference, bool>
{
	encoding_adapter<decltype(tup_breadth_map)> map(tup_breadth_map);
	auto first(iterator::make_transform(first_, map));
	auto last(iterator::make_transform(last_, map));
	auto a(items.get_allocator());
	auto value_deleter([a](value_pair *p) -> void {
		value_pair::destroy(a, p);
	});

	std::unique_ptr<
		value_pair, decltype(value_deleter)
	> vp(nullptr, value_deleter);

	pair_loc p(items.ptr_at(0), 0);

	while (first != last) {
		auto n_index(index_offset(first));
		auto q(p.first->pair_at(items, n_index));

		if (!q.first || q.first->is_vacant()) {
			vp.reset(value_pair::construct(
				a, ++first, last, std::forward<Args>(args)...
			));
			reserve_vacant(p.second, q, vp.get());
			return std::pair<reference, bool>(
				vp.release()->value, true
			);
		} else if (q.first->parent() != p.second) {
			vp.reset(value_pair::construct(
				a, ++first, last, std::forward<Args>(args)...
			));
			detangle(p, q, n_index, vp.get());
			return std::pair<reference, bool>(
				vp.release()->value, true
			);
		} else if (q.first->is_leaf()) {
			auto l_ptr(q.first->leaf_ptr());
			auto c_len(l_ptr->common_length(first, last));

			if (std::distance(first, last) == c_len) {
				if (l_ptr->suffix_length == c_len)
					return std::pair<reference, bool>(
						l_ptr->value, false
					);

				first = last;
				n_index = terminator_index;
			} else {
				std::advance(first, c_len);
				n_index = index_offset(first);
				++first;
			}

			vp.reset(value_pair::construct(
				a, first, last, std::forward<Args>(args)...
			));
			unroll_suffix(q, c_len, n_index, vp.get());
			return std::pair<reference, bool>(
				vp.release()->value, true
			);
		} else {
			++first;
			p = q;
		}
	}
}

template <typename CharType, typename ValueType, typename Policy>
std::basic_ostream<
	CharType, typename Policy::char_traits_type
> &string_map<CharType, ValueType, Policy>::dump(
	std::basic_ostream<
		CharType, typename Policy::char_traits_type
	> &os
) const
{
	{
		auto p(items.ptr_at(0));
		os << "r: " << p->base_offset() << ", ";
		os << "freelist: " << p->parent() << '\n';
	}

	items.for_each(
		1, [&os, this](size_type pos, pair_type const &p) -> bool {
			os << pos << ": ";
			if (p.is_vacant()) {
				os << "vacant (next: " << p.base_offset();
				os << ", prev: " << p.parent() << ")\n";
			} else if (p.is_leaf()) {
				auto l_ptr(p.leaf_ptr());

				os << "(parent: " << p.parent();
				os << ", leaf: " << l_ptr << ") ";

				auto q(items.ptr_at(p.parent()));
				auto n_off(pos - q->base_offset);
				if (n_off < base_index)
					os << "'<eok>' [";
				else {
					auto n_char(offset_char(n_off));
					os << '\'' <<  n_char << "' (";
					os << static_cast<uintptr_t>(n_char);
					os << ") [";
				}

				os << '"';
				auto suffix(l_ptr->suffix());
				for (
					size_type c(0);
					c < l_ptr->suffix_length;
					++c
				)
					os << std::get<1>(
						tup_breadth_map
					).value(suffix[c]);

				os << "\"] -> " << l_ptr->value << '\n';
			} else {
				os << "(parent: " << p.parent();
				os << ", offset: " << p.base_offset() << ") ";

				auto q(items.ptr_at(p.parent()));
				auto n_off(pos - q->base_offset);
				if (n_off < base_index)
					os << "'<eok>'\n";
				else {
					auto n_char(offset_char(n_off));
					os << '\'' <<  n_char << "' (";
					os << static_cast<uintptr_t>(n_char);
					os << ")\n";
				}
			}
			return false;
		}
	);

	return os;
}

template <typename CharType, typename ValueType, typename Policy>
void string_map<CharType, ValueType, Policy>::reserve_vacant(
	uintptr_t parent_pos, pair_loc child_loc, value_pair *v
)
{
	if (!child_loc.first) {
	}

	
}

template <typename CharType, typename ValueType, typename Policy>
void string_map<CharType, ValueType, Policy>::detangle(
	pair_loc parent_loc, pair_loc child_loc, uintptr_t c_index,
	value_pair *v
)
{
}

template <typename CharType, typename ValueType, typename Policy>
void string_map<CharType, ValueType, Policy>::unroll_suffix(
	pair_loc loc, size_type count, uintptr_t other_index, value_pair *v
)
{
}

template <typename CharType, typename ValueType, typename Policy>
template <typename Alloc, typename IndexIterator, typename... Args>
auto string_map<CharType, ValueType, Policy>::value_pair::construct(
	Alloc const &a, IndexIterator first, IndexIterator last, Args&&... args
) -> value_pair *
{
	typedef allocator::array_helper<index_char_type, Alloc> ah_c;
	typedef allocator::array_helper<value_pair, Alloc> ah_p;

	auto suffix_length(std::distance(first, last));

	auto s_deleter(
		[&a, suffix_length](index_char_type *p) -> void {
			ah_c::destroy(a, p, suffix_length, true);
		}
	);

	std::unique_ptr<index_char_type[], decltype(s_deleter)> s_ptr(
		nullptr, s_deleter
	);

	if (suffix_length > Policy::short_suffix_length)
		s_ptr.reset(ah_c::alloc(a, first, last));

	auto p_deleter([&a](value_pair *p) -> void {
		ah_p::destroy(a, p, 1, true);
	});

	std::unique_ptr<value_pair, decltype(p_deleter)> p_ptr(
		ah_p::alloc_n(a, 1, std::forward<Args>(args)...), p_deleter
	);

	if (s_ptr)
		p_ptr->long_suffix.data = s_ptr.get();
	else
		ah_c::make(a, p_ptr->short_suffix, first, last);

	p_ptr->suffix_length = suffix_length;
	s_ptr.release();
	return p_ptr.release();
}

template <typename CharType, typename ValueType, typename Policy>
template <typename Alloc>
void string_map<CharType, ValueType, Policy>::value_pair::destroy(
	Alloc const &a, value_pair *p
)
{
	typedef allocator::array_helper<index_char_type, Alloc> ah_c;
	typedef allocator::array_helper<value_pair, Alloc> ah_p;

	auto s(p->suffix());

	ah_c::destroy(a, s, p->suffix_length, false);

	if (p->suffix_length > Policy::short_suffix_length)
		ah_c::free(
			a, p->long_suffix.data,
			p->suffix_length + p->long_suffix.offset
		);

	ah_p::destroy(a, p, 1, true);
}

template <typename CharType, typename ValueType, typename Policy>
template <typename Alloc>
void string_map<CharType, ValueType, Policy>::value_pair::shrink_suffix(
	Alloc const &a, size_type count
)
{
	typedef allocator::array_helper<index_char_type, Alloc> ah;

	int mode(suffix_length > Policy::short_suffix_length ? 2 : 0);

	if (count >= suffix_length) {
		if (mode) {
			ah::destroy(
				a, long_suffix.data + long_suffix.offset,
				suffix_length, false
			);
			ah::free(
				a, long_suffix.data,
				long_suffix.offset + suffix_length
			);
		}
		suffix_length = 0;
		return;
	}
	auto next_length(suffix_length - count);
	mode |= next_length > Policy::short_suffix_length ? 1 : 0;

	switch (mode) {
	case 0: { // short to short
		std::move(
			&short_suffix[count], &short_suffix[suffix_length],
			&short_suffix[0]
		);
		ah::destroy(
			a, short_suffix + suffix_length - count,
			suffix_length - count, false
		);
		suffix_length = next_length;
		break;
	}
	case 2: { // long to short
		auto data(long_suffix.data);
		auto offset(long_suffix.offset);

		ah::make(
			a, short_suffix, data + offset + count,
			data + offset + count + next_length
		);
		ah::destroy(a, data + offset, suffix_length, false);
		ah::free(a, data, offset + suffix_length);
		suffix_length = next_length;
		break;
	}
	case 3: { // long to long
		ah::destroy(
			a, long_suffix.data + long_suffix.offset, count, false
		);
		long_suffix.offset += count;
		suffix_length = next_length;

		if (long_suffix.offset > suffix_length) {
			auto s_ptr(ah::alloc(
				a, long_suffix.data + long_suffix.offset,
				long_suffix.data + long_suffix.offset
				+ suffix_length
			));
			ah::destroy(
				a, long_suffix.data + long_suffix.offset,
				suffix_length, false
			);
			std::swap(s_ptr, long_suffix.data);
			ah::free(
				a, s_ptr, long_suffix.offset + suffix_length
			);
			long_suffix.offset = 0;
		}

		break;
	}
	}
}



}}
#endif
