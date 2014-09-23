/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#define BOOST_TEST_MODULE yesod
#include <boost/test/included/unit_test.hpp>

#include <yesod/detail/compressed_index_map.hpp>

namespace ucpf { namespace yesod { namespace detail {

BOOST_AUTO_TEST_CASE(compressed_index_map_a_0)
{
	compressed_index_map_a<uint32_t, 6, 4> m0;

	m0.set(19, 12);
	m0.set(23, 5);
	m0.set(52, 11);
	m0.set(29, 12);

	BOOST_CHECK_EQUAL(m0[19], 12);
	BOOST_CHECK_EQUAL(m0[23], 5);
	BOOST_CHECK_EQUAL(m0[52], 11);
	BOOST_CHECK_EQUAL(m0[29], 12);

	compressed_index_map_a<uint32_t, 6, 4> m1({
		std::make_pair(35, 8),
		std::make_pair(31, 13),
		std::make_pair(55, 3),
		std::make_pair(2, 6)
	});

	BOOST_CHECK_EQUAL(m1[35], 8);
	BOOST_CHECK_EQUAL(m1[31], 13);
	BOOST_CHECK_EQUAL(m1[55], 3);
	BOOST_CHECK_EQUAL(m1[2], 6);
}

BOOST_AUTO_TEST_CASE(compressed_index_map_b_0)
{
	compressed_index_map_b<uint32_t, 256, 6, 4> m0;

	m0.set(19, 12);
	m0.set(23, 5);
	m0.set(52, 11);
	m0.set(29, 12);

	BOOST_CHECK_EQUAL(m0[19], 12);
	BOOST_CHECK_EQUAL(m0[23], 5);
	BOOST_CHECK_EQUAL(m0[52], 11);
	BOOST_CHECK_EQUAL(m0[29], 12);

	compressed_index_map_b<uint32_t, 256, 6, 4> m1({
		std::make_pair(35, 8),
		std::make_pair(31, 13),
		std::make_pair(55, 3),
		std::make_pair(2, 6)
	});

	BOOST_CHECK_EQUAL(m1[35], 8);
	BOOST_CHECK_EQUAL(m1[31], 13);
	BOOST_CHECK_EQUAL(m1[55], 3);
	BOOST_CHECK_EQUAL(m1[2], 6);
}

}}}
