/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#define BOOST_TEST_MODULE mina
#include <boost/test/included/unit_test.hpp>

#include <vector>
#include <string>
#include <mina/fixed_string.hpp>

namespace ucpf { namespace mina {

BOOST_AUTO_TEST_CASE(fixed_string_0)
{
	std::allocator<void> a;

	auto s0(fixed_string::make_s(a, "aa"));
	auto s1(std::string("bb"));
	auto s2(std::vector<char>{'c', 'c'});
	auto s3(fixed_string::make_s(a, "dd"));
	auto s4(fixed_string::make(a, s0, s1, s2, s3));

	std::string rs0("aabbccdd");

	BOOST_CHECK_EQUAL_COLLECTIONS(
		rs0.begin(), rs0.end(), s4.begin(), s4.end()
	);
	fixed_string::destroy(a, s4);
	fixed_string::destroy(a, s3);
	fixed_string::destroy(a, s0);
}

BOOST_AUTO_TEST_CASE(fixed_string_1)
{
	std::allocator<void> a;

	auto s0(fixed_string::make_s(a, "aaaa aaaa aaaa aaaa "));
	auto s1(fixed_string::make_s(a, "bbbb bbbb bbbb bbbb "));
	auto s2(std::string("cccc cccc cccc cccc "));
	auto s3(fixed_string::make_s(a, "dddd dddd dddd dddd "));
	auto s4(fixed_string::make(a, s0, s1, s2, s3));
	std::string rs0(
		"aaaa aaaa aaaa aaaa "
		"bbbb bbbb bbbb bbbb "
		"cccc cccc cccc cccc "
		"dddd dddd dddd dddd "
	);
	BOOST_CHECK_EQUAL_COLLECTIONS(
		rs0.begin(), rs0.end(), s4.begin(), s4.end()
	);
	fixed_string::destroy(a, s4);
	fixed_string::destroy(a, s3);
	fixed_string::destroy(a, s1);
	fixed_string::destroy(a, s0);
}

}}
