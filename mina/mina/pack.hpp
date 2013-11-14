/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_MINA_PACK_NOV_13_2013_1445)
#define UCPF_MINA_PACK_NOV_13_2013_1445

#include <mina/detail/pack.hpp>

namespace ucpf { namespace mina {

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

}}
#endif
