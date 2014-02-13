/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_YESOD_DETAIL_STRING_MAP_OPS_JAN_06_2014_1145)
#define UCPF_YESOD_DETAIL_STRING_MAP_OPS_JAN_06_2014_1145

namespace ucpf { namespace yesod {

template <typename CharType, typename ValueType, typename Policy>
template <typename Iterator, typename... Args>
auto string_map<CharType, ValueType, Policy>::emplace_at(
	Iterator first, Iterator last, Args&&... args
) -> std::pair<reference, bool>
{
	value_pair *rv(nullptr);
	uintptr_t l_pos(trie_root);
	bool inserted(true);

	do {
		auto n_pos(char_offset(l_pos, deref_char(first)));
		auto &p(trie.at(vec_offset(n_pos)));
		++first;
		if (!p.second) {
			rv = value_pair::construct(
				trie.get_allocator(), first, last,
				std::forward<Args>(args)...
			);
			p.first = reinterpret_cast<uintptr_t>(rv);
			p.second = l_pos;
			break;
		} else if (p.first & 1) {
			if (p.second == l_pos) {
				l_pos = p.first;
				continue;
			} else {
				// relocate group
			}
		} else {
			rv = reinterpret_cast<value_pair *>(p.first);
			auto c_len(rv->common_length(first, last));
			index_char_type n_char(terminator_char);

			if ((last - first) == c_len) {
				if (rv->suffix_length == c_len) {
					inserted = false;
					break;
				}
			} else
				n_char = *(first + c_len);

			printf("single n_pos %zd, c_len %zd, n_char %d\n", n_pos, c_len, n_char);
			auto loc(unroll_key(&p, n_pos, c_len, n_char));
			rv = value_pair::construct(
				trie.get_allocator(), first + c_len, last,
				std::forward<Args>(args)...
			);
			loc.first->first = reinterpret_cast<uintptr_t>(rv);
			loc.first->second = loc.second;
			break;
		}
	} while (first != last);

	return std::pair<reference, bool>(rv->value, inserted);
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
	std::function<
		bool (size_type, pair_type const &)
	> f = [&os](size_type pos, pair_type const &p) -> bool {
		os << log_offset(pos) << " (" << pos << "): ";
		if (p.first & 1) {
			os << p.first << " (";
			if (p.first > 1)
				os << vec_offset(p.first);
			else
				os << 'r';
			
			os << "), ";
			os << p.second << " (";
			if (p.second > 1)
				os << vec_offset(p.second);
			else
				os << (p.second ? 'r' : 'n');

			os << ")\n";
		} else {
			auto vp(reinterpret_cast<value_pair const *>(p.first));
			os << vp << ", " << p.second << " (";

			if (p.second > 1)
				os << vec_offset(p.second);
			else
				os << (p.second ? 'r' : 'n');

			os << ')';

			if (vp) {
				os << " -> \"";
				auto sp(vp->suffix());
				for (
					size_type c(0); c < vp->suffix_length;
					++c
				) {
					if (sp[c])
						os << sp[c];
					else
						os << "\\0";
				}
				os << "\" <" << vp->value << '>';
				
			}
			os << '\n';
		}
		return true;
	};

	os << "r: " << trie_root << '\n';
	trie.for_each(std::forward<decltype(f)>(f));
	return os;
}

template <typename CharType, typename ValueType, typename Policy>
template <typename Iterator>
auto string_map<CharType, ValueType, Policy>::find_impl(
	Iterator &first, Iterator const &last
) const -> std::tuple<uintptr_t, uintptr_t, uintptr_t>
{
	uintptr_t l_pos(trie_root);
	do {
		auto n_pos(char_offset(l_pos, deref_char(first)));
		auto p(trie.ptr_at(vec_offset(n_pos)));
		++first;

		if (!p || !p->first || p->second != l_pos)
			return std::make_tuple(0, 0, 0);

		if (!(p->first & 1))
			return std::tie(p->first, p->second, n_pos);

		l_pos = p->first;
	} while (first != last);

	/* Check for virtual key terminator (will appear if one key is a
	 * substring of another).
	 */
	auto n_pos(char_offset(l_pos, 1));
	auto p(trie.ptr_at(vec_offset(n_pos)));
	if (p && p->first && p->second == l_pos && !(p->first & 1))
		return std::tie(p->first, p->second, n_pos);

	return std::make_tuple(0, 0, 0);
}

template <typename CharType, typename ValueType, typename Policy>
auto string_map<CharType, ValueType, Policy>::unroll_key(
	pair_type *p, uintptr_t pos, size_type count, index_char_type other
) -> std::pair<pair_type *, uintptr_t>
{
	size_type shrink(0);
	std::unique_ptr<value_pair, std::function<void (value_pair *)>> v_ptr(
		reinterpret_cast<value_pair *>(p->first),
		[&shrink, this](value_pair *v) -> void {
			if (shrink)
				v->shrink_suffix(trie.get_allocator(), shrink);
		}
	);

	auto suffix(v_ptr->suffix());
	auto k_char(terminator_char);
	if ((count + 1) < v_ptr->suffix_length)
		k_char = suffix[count];

	uintptr_t next_pos(0);
	while (count > shrink) {
		next_pos = char_offset(1, suffix[shrink]);
		auto xp(trie.find_empty_above(vec_offset(next_pos)));
		auto q(trie.emplace_at(
			vec_offset(xp),
			reinterpret_cast<uintptr_t>(v_ptr.get()),
			pos
		));
		p->first = log_offset(xp - vec_offset(next_pos));
		pos = log_offset(xp);
		++shrink;
		p = &q;
	}

	auto min_char(std::min(k_char, other));
	auto max_char(std::max(k_char, other));
	uintptr_t adj_pos(1);

	while (true) {
		next_pos = char_offset(adj_pos, min_char);
		auto xp(trie.find_empty_above(vec_offset(next_pos)));

		adj_pos = log_offset(xp - vec_offset(next_pos));
		if (!trie.ptr_at(
			vec_offset(char_offset(adj_pos, max_char))
		))
			break;

		adj_pos += 2;
	}

	trie.emplace_at(
		vec_offset(char_offset(adj_pos, k_char)),
		reinterpret_cast<uintptr_t>(v_ptr.get()), pos
	);
	p->first = adj_pos;
	++shrink;

	return std::make_pair(
		&trie.emplace_at(
			vec_offset(char_offset(adj_pos, other)), 0, 0
		), pos
	);
}

template <typename CharType, typename ValueType, typename Policy>
template <typename Alloc, typename Iterator, typename... Args>
auto string_map<CharType, ValueType, Policy>::value_pair::construct(
	Alloc const &a_, Iterator first, Iterator last, Args&&... args
) -> value_pair *
{
	char_allocator_type ca(a_);
	auto suffix_length(last - first);
	decltype(suffix_length) init_length(0);

	std::unique_ptr<char_type[], std::function<void (char_type *)>> s_ptr(
		nullptr, [
			&ca, &init_length, suffix_length
		](char_type *p) -> void {
			for (; init_length > 0; --init_length)
				char_allocator_traits::destroy(
					ca, &p[init_length - 1]
				);

			char_allocator_traits::deallocate(
				ca, p, suffix_length
			);
		}
	);

	if (suffix_length > Policy::short_suffix_length) {
		s_ptr.reset(char_allocator_traits::allocate(
			ca, suffix_length
		));

		for (; init_length < suffix_length; ++init_length) {
			char_allocator_traits::construct(
				ca, &s_ptr[init_length], *first
			);
			++first;
		}
	}

	pair_allocator_type pa(a_);
	std::unique_ptr<value_pair, std::function<void (value_pair *)>> p_ptr(
		pair_allocator_traits::allocate(pa, 1),
		[&pa](value_pair *p) -> void {
			pair_allocator_traits::deallocate(pa, p, 1);
		}
	);

	pair_allocator_traits::construct(
		pa, p_ptr.get(), std::forward<Args>(args)...
	);
	if (s_ptr)
		p_ptr->long_suffix.data = s_ptr.release();
	else {
		decltype(s_ptr) ss_ptr(
			p_ptr->suffix(), [
				&ca, &init_length
			](char_type *p) -> void {
				for (; init_length > 0; --init_length)
					char_allocator_traits::destroy(
						ca, &p[init_length - 1]
					);
			}
		);

		for (; init_length < suffix_length; ++init_length) {
			char_allocator_traits::construct(
				ca, &ss_ptr[init_length], *first
			);
			++first;
		}
		ss_ptr.release();
	}

	p_ptr->suffix_length = suffix_length;
	return p_ptr.release();
}

template <typename CharType, typename ValueType, typename Policy>
template <typename Alloc>
void string_map<CharType, ValueType, Policy>::value_pair::destroy(
	Alloc const &a_, value_pair *p
)
{
	char_allocator_type ca(a_);
	auto s(p->suffix());

	for (auto cnt(p->suffix_length); cnt > 0; --cnt)
		char_allocator_traits::destroy(ca, &s[cnt - 1]);

	if (p->suffix_length > Policy::short_suffix_length)
		char_allocator_traits::deallocate(
			ca, p->long_suffix.data,
			p->suffix_length + p->long_suffix.offset
		);

	pair_allocator_type pa(a_);
	pair_allocator_traits::destroy(pa, p);
	pair_allocator_traits::deallocate(pa, p, 1);
}

template <typename CharType, typename ValueType, typename Policy>
template <typename Alloc>
void string_map<CharType, ValueType, Policy>::value_pair::shrink_suffix(
	Alloc const &a_, size_type count
)
{
	int mode(suffix_length > Policy::short_suffix_length ? 2 : 0);
	mode |= (suffix_length - count) > Policy::short_suffix_length
		? 1 : 0;
	char_allocator_type ca(a_);

	switch (mode) {
	case 0: { // short to short
		std::move(
			&short_suffix[count], &short_suffix[suffix_length],
			&short_suffix[0]
		);
		for (auto c(suffix_length - count); c < suffix_length; ++c)
			char_allocator_traits::destroy(
				ca, &short_suffix[c]
			);
		break;
	}
	case 2: { // long to short
		auto data(long_suffix.data);
		auto offset(long_suffix.offset);

		for (size_type c(0); c < (suffix_length - count); ++c)
			char_allocator_traits::construct(
				ca, &short_suffix[c],
				std::move(data[offset + count + c])
			);

		while (true) {
			auto c(suffix_length - 1);
			char_allocator_traits::destroy(
				ca, &data[offset + c]
			);
			if (!c)
				break;
			--c;
		}

		char_allocator_traits::deallocate(
			ca, data, offset + suffix_length
		);
		break;
	}
	case 3: { // long to long
		auto next_offset(long_suffix.offset + count);
		for (; long_suffix.offset < next_offset; ++long_suffix.offset)
			char_allocator_traits::destroy(
				ca, long_suffix.data + long_suffix.offset
			);
		break;
	}
	}

	suffix_length -= count;
}

}}
#endif
