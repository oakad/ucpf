/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#define BOOST_TEST_MODULE yesod
#include <boost/test/included/unit_test.hpp>

#include <yesod/detail/compressed_index_lut.hpp>

namespace ucpf { namespace yesod { namespace detail {

BOOST_AUTO_TEST_CASE(compressed_index_lut_0)
{
	compressed_index_lut<uint32_t, 6, 4> l0;

	l0.set(19, 12);
	l0.set(23, 5);
	l0.set(52, 11);
	l0.set(29, 12);

	BOOST_CHECK_EQUAL(l0[19], 12);
	BOOST_CHECK_EQUAL(l0[23], 5);
	BOOST_CHECK_EQUAL(l0[52], 11);
	BOOST_CHECK_EQUAL(l0[29], 12);
}

BOOST_AUTO_TEST_CASE(compressed_index_lut_1)
{
	compressed_index_lut<uint32_t, 6, 4> l0({
		std::make_pair(35, 8),
		std::make_pair(31, 13),
		std::make_pair(55, 3),
		std::make_pair(2, 6)
	});

	BOOST_CHECK_EQUAL(l0[35], 8);
	BOOST_CHECK_EQUAL(l0[31], 13);
	BOOST_CHECK_EQUAL(l0[55], 3);
	BOOST_CHECK_EQUAL(l0[2], 6);
}

}}}
