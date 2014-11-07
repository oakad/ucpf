/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#define BOOST_TEST_MODULE yesod
#include <boost/test/included/unit_test.hpp>

#include <yesod/collector.hpp>

namespace ucpf { namespace yesod {

BOOST_AUTO_TEST_CASE(collector_0)
{
	constexpr static std::size_t block_size(16);
	collector<int, block_size> c0;
	collector<int, block_size, true> c1;

	BOOST_CHECK_GE(sizeof(c1), sizeof(c0) + sizeof(int) * block_size);
}

}}
