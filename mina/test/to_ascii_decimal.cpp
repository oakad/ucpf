/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#define BOOST_TEST_MODULE mina
#include <boost/test/included/unit_test.hpp>

#include <string>
#include <mina/to_ascii_decimal.hpp>

namespace ucpf { namespace mina {

BOOST_AUTO_TEST_CASE(to_ascii_decimal_0)
{
	std::array<uint8_t, 9> n0{
		0, 7, 24, 91, 113, 139, 200, 243, 255
	};
	std::string s0("0, 7, 24, 91, 113, 139, 200, 243, 255");
	std::string ref;
	auto iter0(n0.begin());
	auto sink(std::back_inserter(ref));

	while (true) {
		to_ascii_decimal(sink, *iter0);
		++iter0;
		if (iter0 == n0.end())
			break;
		*sink++ = ',';
		*sink++ = ' ';
	};

	BOOST_CHECK_EQUAL(ref, s0);
	ref.clear();

	std::array<int8_t, 9> n1{
		-128, -101, -53, -5, 0, 9, 61, 111, 127
	};
	std::string s1("-128, -101, -53, -5, +0, +9, +61, +111, +127");
	auto iter1(n1.begin());

	while (true) {
		to_ascii_decimal(sink, *iter1);
		++iter1;
		if (iter1 == n1.end())
			break;
		*sink++ = ',';
		*sink++ = ' ';
	};
	BOOST_CHECK_EQUAL(ref, s1);
}

}}
