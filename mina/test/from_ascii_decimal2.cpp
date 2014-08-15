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
#include <mina/from_ascii_decimal.hpp>
#include "float_generator.hpp"

#define CASE_COUNT 100000

namespace ucpf { namespace mina {

BOOST_AUTO_TEST_CASE(from_ascii_decimal2_3)
{
	{
		char const *first = "+0.0";
		char const *last = first + strlen(first);
		double v;
		BOOST_CHECK(from_ascii_decimal(first, last, v));
		BOOST_CHECK_EQUAL(v, 0.0);
	}
}

}}
