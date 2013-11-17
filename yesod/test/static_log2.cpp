/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#define BOOST_TEST_MODULE yesod
#include <boost/test/included/unit_test.hpp>

#include <yesod/static_log2.hpp>

namespace ucpf { namespace yesod {

BOOST_AUTO_TEST_CASE(static_log2_0)
{
	BOOST_CHECK_EQUAL((static_log2<uint32_t, 1234>::msb), 10);
	BOOST_CHECK_EQUAL((static_log2<uint32_t, 1234>::value), 11);
	BOOST_CHECK_EQUAL((static_log2<uint32_t, 4096>::msb), 12);
	BOOST_CHECK_EQUAL((static_log2<uint32_t, 4096>::value), 12);
}

}}
