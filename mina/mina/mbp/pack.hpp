/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_MINA_MBP_PACK_20131113T1445)
#define UCPF_MINA_MBP_PACK_20131113T1445

#include <mina/mbp/detail/pack.hpp>
#include <boost/range/iterator_range.hpp>

namespace ucpf { namespace mina { namespace mbp {

template <typename OutputIterator, typename ...Tn>
void pack(OutputIterator &&sink, Tn &&...vn)
{
	if (sizeof...(vn) > 1) {
		*sink++ = detail::tuple_start_code;
		detail::pack(
			std::forward<OutputIterator>(sink),
			std::forward<Tn>(vn)...
		);
		*sink++ = detail::tuple_end_code;
	} else
		detail::pack(
			std::forward<OutputIterator>(sink),
			std::forward<Tn>(vn)...
		);
}

template <typename OutputIterator>
struct custom<OutputIterator, char const *&> {
	static void pack(OutputIterator &&sink, char const *&v)
	{
		boost::iterator_range<char const *> r(
			v, v + std::strlen(v)
		);
		mbp::pack(std::forward<OutputIterator>(sink), r);
	}
};

template <typename OutputIterator, size_t N>
struct custom<OutputIterator, char const (&)[N]> {
	static void pack(OutputIterator &&sink, char const (&v)[N])
	{
		boost::iterator_range<char const *> r(
			v, v + N - 1
		);
		mbp::pack(std::forward<OutputIterator>(sink), r);
	}
};

}}}
#endif
