/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#define BOOST_TEST_MODULE yesod
#include <boost/test/unit_test.hpp>

#include <yesod/array_element_count.hpp>

namespace ucpf { namespace yesod {

BOOST_AUTO_TEST_CASE(array_element_count_0)
{
	BOOST_CHECK_EQUAL(array_element_count<int>::value, 1);

	BOOST_CHECK_EQUAL(array_element_count<int[5]>::value, 5);
	BOOST_CHECK_EQUAL(array_element_count<int[5][6]>::value, 11);
	BOOST_CHECK_EQUAL(array_element_count<int[5][6][7]>::value, 18);
}

}}
