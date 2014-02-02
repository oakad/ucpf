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
	uintptr_t l_pos(trie_root.first);
	bool inserted(true);

	do {
		auto n_pos(log_offset(l_pos, deref_char(first)));
		auto &p(trie.at(vec_offset(n_pos)));
		printf("yy %p\n", &p);
		++first;
		if (!p.second) {
			rv = value_pair::construct(
				trie.get_allocator(), first, last,
				std::forward<Args>(args)...
			);
			p.first = reinterpret_cast<uintptr_t>(rv);
			p.second = l_pos;
			printf("xx %zd, %zx, %zd, %p\n", n_pos, p.first, p.second, rv);
			break;
		} else if (p.first & 1) {
			if (p.second == l_pos) {
				l_pos = p.first;
				continue;
			} else {
			}
		} else {
			rv = reinterpret_cast<value_pair *>(p.first);
			auto c_pos(rv->common_legth(first, last));
			if (((last - first) == c_pos) && (
				rv->suffix_length == c_pos
			)) {
				inserted = false;
				break;
			}

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
		os << pos << " (" << pos * 2 + 5 << ')';
		if (p.first & 1) {
			os << ": " << p.first << " (";
			os << (p.first ? (p.first - 5) / 2 : 0) << "), ";
			os << p.second << " (";
			os << (p.second ? (p.second - 5) / 2 : 0) << ")\n";
		} else {
			auto vp(reinterpret_cast<value_pair const *>(p.first));
			os << ": (" << vp << ", " << p.second << ')';
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

	os << "r: " << trie_root.first << '\n';
	trie.for_each(std::forward<decltype(f)>(f));
	return os;
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
	printf("construct 1\n");

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
	printf("construct 2\n");
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
	printf("construct 3 %p, %zd\n", p_ptr.get(), suffix_length);
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
		printf("cc 3a\n");
		for (; init_length < suffix_length; ++init_length) {
			char_allocator_traits::construct(
				ca, &ss_ptr[init_length], *first
			);
			++first;
		}
		ss_ptr.release();
	}
	printf("construct 4 %p\n", p_ptr.get());
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
			ca, p->long_suffix.data, p->suffix_length
		);

	pair_allocator_type pa(a_);
	pair_allocator_traits::destroy(pa, p);
	pair_allocator_traits::deallocate(pa, p, 1);
}

}}
#endif
