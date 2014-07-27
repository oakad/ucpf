/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_MINA_DETAIL_FROM_ASCII_DECIMAL_U_20140727T2300)
#define UCPF_MINA_DETAIL_FROM_ASCII_DECIMAL_U_20140727T2300

namespace ucpf { namespace mina { namespace detail {

template <typename T>
struct from_ascii_decimal_u_cnt {

	template <typename InputIterator>
	from_ascii_decimal_u(
		InputIterator &first, InputIterator last,
		T initial, size_t max_digits, bool consume_all
	)
	: converted(0), consumed(0), value(initial)
	{
		while (true) {
			if ((first == last) || !std::isdigit(*first))
				return;

			if (converted >= max_digits)
				break;

			value *= T(10);
			value += *first - '0';
			++converted;
			++consumed;
			++first;
		}

		if (!consume_all)
			return;

		if ((first != last) && std::isdigit(*first)) {
			if (*first >= '5')
				++value;

			++first;
		}

		while ((first != last) && std::isdigit(*first)) {
			++consumed;
			++first;
		}
	}

	size_t converted;
	size_t consumed;
	T value;
};

}}}
#endif

