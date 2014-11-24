/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#define BOOST_TEST_MODULE yesod
#include <boost/test/unit_test.hpp>
#include <boost/test/output_test_stream.hpp>

#include <yesod/rope.hpp>
#include <yesod/iterator/range.hpp>

#include <iostream>

namespace ucpf {  namespace yesod {

BOOST_AUTO_TEST_CASE(rope_0)
{
	using boost::test_tools::output_test_stream;
	output_test_stream out("ref/rope/append.00.out", true);
	output_test_stream h1("ref/rope/append.01.out", false);

	crope a(iterator::str("aaaa"));
	crope b(iterator::str("bbbb"));

	crope c(a + b);
	out << c << std::endl;
	h1 << c.dump<char>() << std::endl;
	BOOST_CHECK(out.match_pattern());
	BOOST_CHECK(h1.match_pattern());
}

BOOST_AUTO_TEST_CASE(rope_1)
{
	using boost::test_tools::output_test_stream;
	output_test_stream out("ref/rope/split.00.out", true);
	output_test_stream h1("ref/rope/split.01.out", false);

	crope in =  crope_file_reader("ref/rope/split.00.out");

	crope s0(in.cbegin(), in.cbegin() + 20);
	crope s1(in.cbegin() + 20, in.cend() - 20);
	crope s2(in.cend() - 20, in.cend());

	crope c(s0 + s1 + s2);
	out << c;
	h1 << c.dump<char>() << std::endl;
	BOOST_CHECK(out.match_pattern());
	BOOST_CHECK(h1.match_pattern());
}

}}
