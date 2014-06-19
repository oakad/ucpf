/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_MINA_MBP_EXPAND_20140619T2300)
#define UCPF_MINA_MBP_EXPAND_20140619T2300

#include <mina/mbp/raw_encoding.hpp>

namespace ucpf { namespace mina { namespace mbp { namespace detail {

template <typename ForwardIterator, typename Visitor>
bool expand(ForwardIterator &first, ForwardIterator last, Visitor &&v)
{
	uint8_t xv(*first);
	if (!field_class<>::header_valid(xv))
		return false;

	auto f_class(field_class<>::from_header(xv));

	if (f_class.scalar_r == scalar_rank<>::i5) {
		if (xv < small_uint_code_offset) {
			int8_t val(xv & small_int_mask);
			val -= small_int_code_offset;
			v.value(val);
		} else {
			uint8_t val(xv & small_int_mask);
			v.value(val);
		}
		++first;
		return true;
	};

	if (f_class.list_size_r == list_size_rank::l3) {
		size_t len(xv & small_list_size_mask);
		++first;
		if (!len)
			return expand_s(
				f_class, first, last, std::forward<Visitor>(v)
			);

		if (f_class.scalar_r == scalar_rank<>::i8)
			return expand_b(
				f_class, len, first, last,
				std::forward<Visitor>(v)
			);

		return expand_v(
			f_class, len, first, last, std::forward<Visitor>(v)
		);
	}

#error for all remaining list types

	return true;
}

}}}}

#endif
