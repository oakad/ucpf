/*
 * Copyright (c) 2016 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#define BOOST_TEST_MODULE holam
#include <boost/test/unit_test.hpp>

#include <holam/out.hpp>
#include "test.hpp"

namespace ucpf { namespace holam { namespace test {

BOOST_AUTO_TEST_CASE(out_0)
{
	char const *s0 = "abcdefghij";
	output::collector<> dest;
	BOOST_CHECK_EQUAL(10, out(dest, s0));
	BOOST_CHECK_EQUAL(dest, s0);
}

}}}
