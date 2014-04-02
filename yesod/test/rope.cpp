/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#define BOOST_TEST_MODULE yesod
#include <boost/test/included/unit_test.hpp>
#include <boost/test/output_test_stream.hpp>

#include <yesod/rope.hpp>
#include <iostream>

namespace ucpf {  namespace yesod {

BOOST_AUTO_TEST_CASE(rope_0)
{
	using boost::test_tools::output_test_stream;
	output_test_stream out("ref/rope/append.00.out", true);
	output_test_stream h1("ref/rope/append.01.out", false);

	crope a("aaaa");
	crope b("bbbb");

	crope c(a + b);
	out << c << std::endl;
	h1 << c.dump<char>() << std::endl;
	BOOST_CHECK(out.match_pattern());
	BOOST_CHECK(h1.match_pattern());
}

}}
