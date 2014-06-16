/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_MINA_MBP_20131113T1445)
#define UCPF_MINA_MBP_20131113T1445

#include <mina/mbp/pack.hpp>
#include <mina/mbp/unpack.hpp>
#include <yesod/iterator/range.hpp>

namespace ucpf { namespace mina { namespace mbp {

template <typename OutputIterator, typename ...Tn>
void pack(OutputIterator &&sink, Tn &&...vn)
{
	detail::pack(
		std::forward<OutputIterator>(sink),
		std::forward<Tn>(vn)...
	);
}

template <typename OutputIterator, typename ...Tn>
void pack_tuple(OutputIterator &&sink, Tn &&...vn)
{
	*sink++ = detail::tuple_start_code;
	detail::pack(
		std::forward<OutputIterator>(sink),
		std::forward<Tn>(vn)...
	);
	*sink++ = detail::tuple_end_code;
}

template <typename ForwardIterator, typename ...Tn>
bool unpack(ForwardIterator &first, ForwardIterator last, Tn &&...vn)
{
	if (!sizeof...(vn))
		return true;

	if (first == last)
		return false;

	if (*first == detail::byte_skip_code) {
		if (!detail::advance_skip(first, last))
			return false;
	}

	return detail::unpack(first, last, std::forward<Tn>(vn)...);
}

template <typename ForwardIterator, typename ...Tn>
bool unpack_tuple(ForwardIterator &first, ForwardIterator last, Tn &&...vn)
{
	if (!sizeof...(vn))
		return true;

	if (first == last)
		return false;

	if (*first == detail::byte_skip_code) {
		if (!detail::advance_skip(first, last))
			return false;
	}

	if (*first != detail::tuple_start_code)
		return false;

	++first;

	if (detail::unpack(first, last, std::forward<Tn>(vn)...)) {
		if (first == last)
			return false;

		if (*first == detail::byte_skip_code) {
			if (!detail::advance_skip(first, last))
				return false;
		}

		if (*first != detail::tuple_end_code)
			return false;

		++first;
		return true;
	} else
		return false;
}

template <typename Range, typename ...Tn>
bool unpack(Range const &r, Tn &&...vn)
{
	auto first(r.begin());
	return unpack(first, r.end(), std::forward<Tn>(vn)...);
}

template <typename Range, typename ...Tn>
bool unpack_tuple(Range const &r, Tn &&...vn)
{
	auto first(r.begin());
	return unpack_tuple(first, r.end(), std::forward<Tn>(vn)...);
}

template <typename CharType>
struct custom<CharType const *> {
	template <typename OutputIterator>
	static void pack(OutputIterator &sink, CharType const *v)
	{
		mbp::pack(
			std::forward<OutputIterator>(sink),
			yesod::iterator::str(v)
		);
	}
};

template <typename CharType, size_t N>
struct custom<CharType const [N]> {
	template <typename OutputIterator>
	static void pack(OutputIterator &sink, CharType const v[N])
	{
		mbp::pack(
			std::forward<OutputIterator>(sink),
			yesod::iterator::make_range(v, N - 1)
		);
	}
};

}}}
#endif
