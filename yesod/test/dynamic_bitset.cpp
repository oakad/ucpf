/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#define BOOST_TEST_MODULE yesod
#include <boost/test/included/unit_test.hpp>

#include <yesod/dynamic_bitset.hpp>

namespace ucpf { namespace yesod {

BOOST_AUTO_TEST_CASE(dynamic_bitset_0)
{
	dynamic_bitset<> b;

	b.set(10);
	b.set(20);
	b.set(30);
	BOOST_CHECK(!b.test(15) && !b.test(25) && !b.test(35) && !b.test(105));
	BOOST_CHECK(b.test(10) && b.test(20) && b.test(30));
}

}}
