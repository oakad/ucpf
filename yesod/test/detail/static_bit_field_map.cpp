/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#define BOOST_TEST_MODULE yesod
#include <boost/test/included/unit_test.hpp>

#include <yesod/detail/static_bit_field_map.hpp>

namespace std {

template <std::size_t N>
ostream &operator<<(
	ostream &os,
	std::array<std::pair<std::size_t, std::size_t>, N> const &v
)
{
	os << '[';
	bool not_first(false);
	for (auto &p: v) {
		if (not_first)
			os << ", ";
		os << '(' << p.first << ", " << p.second << ')';
		not_first = true;
	}
	os << ']';
        return os;
}
}

namespace ucpf { namespace yesod { namespace detail { namespace test {

BOOST_AUTO_TEST_CASE(static_bit_field_map_0)
{
	typedef static_bit_field_map<32, 10, 12, 5, 10>::value_type v0;
	std::array<std::pair<std::size_t, std::size_t>, 4> ref0 = {{
		{10, 0}, {12, 10}, {5, 22}, {5, 27}
	}};
	BOOST_CHECK_EQUAL(v0::value, ref0);

	typedef static_bit_field_map<32, 8, 7>::value_type v1a;
	std::array<std::pair<std::size_t, std::size_t>, 2> ref1a = {{
		{8, 0}, {7, 8}
	}};
	BOOST_CHECK_EQUAL(v1a::value, ref1a);

	typedef static_bit_field_map<32, 8, 7>::repeat_last::value_type v1b;
	std::array<std::pair<std::size_t, std::size_t>, 5> ref1b = {{
		{8, 0}, {7, 8}, {7, 15}, {7, 22}, {3, 29}
	}};
	BOOST_CHECK_EQUAL(v1b::value, ref1b);
}

}}}}
