/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#define BOOST_TEST_MODULE yesod
#include <boost/test/unit_test.hpp>

#include <yesod/collector.hpp>

namespace ucpf { namespace yesod {

BOOST_AUTO_TEST_CASE(collector_0)
{
        static std::random_device src;
        std::mt19937 gen(src());
        std::uniform_int_distribution<int> dis;

        constexpr static std::size_t count = 10000;
	constexpr static std::size_t block_size(16);

	collector<int, block_size> c0;
	collector<int, block_size, true> c1;
	std::vector<int> r0;

	r0.reserve(count);
	BOOST_CHECK_GE(sizeof(c1), sizeof(c0) + sizeof(int) * block_size);

	for (std::size_t c(0); c < count; ++c) {
		auto v(dis(gen));
		c0.emplace_back(v);
		c1.emplace_back(v);
		r0.emplace_back(v);
	}

	BOOST_CHECK_EQUAL_COLLECTIONS(
		c0.begin(), c0.end(), r0.begin(), r0.end()
	);
	BOOST_CHECK_EQUAL_COLLECTIONS(
		c1.begin(), c1.end(), r0.begin(), r0.end()
	);

	while (!r0.empty()) {
		BOOST_CHECK_EQUAL(c0.back(), r0.back());
		BOOST_CHECK_EQUAL(c1.back(), r0.back());
		c0.pop_back();
		c1.pop_back();
		r0.pop_back();
	}
}

}}
