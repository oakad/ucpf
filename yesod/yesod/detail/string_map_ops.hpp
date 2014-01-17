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

	if (suffix_length_ > Policy::short_suffix_length) {
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
				ca, &s_ptr[init_length], *first
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
			ca, p->long_suffix.data, p->suffix_length
		);

	pair_allocator_type pa(a_);
	pair_allocator_traits::destroy(pa, p);
	pair_allocator_traits::deallocate(pa, p, 1);
}

}}
#endif
