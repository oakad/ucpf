/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE mina
#include <boost/test/unit_test.hpp>

#include <vector>
#include <string>
#include <mina/string_utils.hpp>

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

BOOST_AUTO_TEST_CASE(fixed_string_2)
{
	std::allocator<void> a;
	{
		int v0(1234), w0(0);
		fixed_string s0;
		std::string r0("+1234");

		BOOST_CHECK(to_fixed_string(s0, v0, a));
		BOOST_CHECK_EQUAL_COLLECTIONS(
			s0.begin(), s0.end(), r0.begin(), r0.end()
		);
		fixed_string::destroy(a, s0);

		s0 = fixed_string::make(a, r0);
		BOOST_CHECK(from_fixed_string(w0, s0, a));
		BOOST_CHECK_EQUAL(v0, w0);
		fixed_string::destroy(a, s0);
	}

	{
		double v1(1234.456), w1(0);
		fixed_string s1;
		std::string r1("+1234.456");

		BOOST_CHECK(to_fixed_string(s1, v1, a));
		BOOST_CHECK_EQUAL_COLLECTIONS(
			s1.begin(), s1.end(), r1.begin(), r1.end()
		);
		fixed_string::destroy(a, s1);

		s1 = fixed_string::make(a, r1);
		BOOST_CHECK(from_fixed_string(w1, s1, a));
		BOOST_CHECK_EQUAL(v1, w1);
		fixed_string::destroy(a, s1);
	}
	{
		std::vector<double> v2{123.45, -56.678, 45.789}, w2;
		fixed_string s2;
		std::string r2("+123.45, -56.678, +45.789");

		BOOST_CHECK(to_fixed_string(s2, v2, a));
		BOOST_CHECK_EQUAL_COLLECTIONS(
			s2.begin(), s2.end(), r2.begin(), r2.end()
		);
		fixed_string::destroy(a, s2);

		s2 = fixed_string::make(a, r2);
		BOOST_CHECK(from_fixed_string(w2, s2, a));
		BOOST_CHECK_EQUAL_COLLECTIONS(
			v2.begin(), v2.end(), w2.begin(), w2.end()
		);
		fixed_string::destroy(a, s2);
	}
	{
		std::string v3("abcdefg"), w3;
		fixed_string s3;
		std::string r3("abcdefg");

		BOOST_CHECK(to_fixed_string(s3, v3, a));
		BOOST_CHECK_EQUAL_COLLECTIONS(
			s3.begin(), s3.end(), r3.begin(), r3.end()
		);
		fixed_string::destroy(a, s3);

		s3 = fixed_string::make(a, r3);
		BOOST_CHECK(from_fixed_string(w3, s3, a));
		BOOST_CHECK_EQUAL(v3, w3);
		fixed_string::destroy(a, s3);
	}
}
}}
