/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#define BOOST_TEST_MODULE yesod
#include <boost/test/included/unit_test.hpp>

#include <yesod/iterator/collector.hpp>

namespace ucpf { namespace yesod { namespace iterator {

BOOST_AUTO_TEST_CASE(collector_0)
{
	std::string s_in("owbtcwjbjiipxamrpxtogdcjuftrtm");
	collector<char, 8> c(true);
	std::copy(s_in.begin(), s_in.end(), c);
	BOOST_CHECK_EQUAL(s_in.size(), c.size());
	std::string s_out;
	BOOST_CHECK_EQUAL(
		c.copy(std::back_inserter(s_out), c.size(), 0), s_in.size()
	);
	BOOST_CHECK_EQUAL(s_in, s_out);
}

}}}
