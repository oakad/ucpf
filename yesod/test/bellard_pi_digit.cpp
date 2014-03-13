/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#define BOOST_TEST_MODULE yesod
#include <boost/test/included/unit_test.hpp>

#include <yesod/detail/bellard_pi_digit.hpp>

namespace ucpf { namespace yesod { namespace detail {

BOOST_AUTO_TEST_CASE(bellard_pi_digit_0)
{
	uint32_t xd(bellard_pi_digit(0));
	BOOST_CHECK_EQUAL(xd, 0x243f6a88);
}

}}}
