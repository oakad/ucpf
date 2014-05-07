/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#define BOOST_TEST_MODULE yesod
#include <boost/test/included/unit_test.hpp>

#include <yesod/flat_map.hpp>

namespace ucpf { namespace yesod {

BOOST_AUTO_TEST_CASE(flat_map_0)
{
	flat_map<int, int> m0{
		std::make_tuple(1, 10),
		std::make_tuple(2, 20),
		std::make_tuple(3, 30)
	};

	BOOST_CHECK_EQUAL(m0.size(), 3);
	BOOST_CHECK_EQUAL(m0[2], 20);

	{
		auto i_p(m0.insert(std::make_tuple(4, 40)));
		BOOST_CHECK(i_p.second);
		BOOST_CHECK(*i_p.first == std::make_tuple(4, 40));
	}

	{
		auto i_p(m0.insert(std::make_pair(3, 30)));
		BOOST_CHECK(!i_p.second);
		BOOST_CHECK(*i_p.first == std::make_tuple(3, 30));
	}

	BOOST_CHECK(*m0.find(1) == std::make_tuple(1, 10));
	m0.erase(1);
	BOOST_CHECK(m0.find(1) == m0.end());
}

}}
