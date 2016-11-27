/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#define BOOST_TEST_MODULE yesod
#include <boost/test/included/unit_test.hpp>

#include <yesod/iterator/object_collector.hpp>

namespace ucpf { namespace yesod { namespace iterator {
namespace test {

struct a {
	a()
	: v(0)
	{}

	a(int v_)
	: v(v_)
	{}

	int v;
};

}

BOOST_AUTO_TEST_CASE(object_collector_0)
{
	std::array<int, 16> d_in{
		100, 101, 102, 103, 104, 105, 106, 107,
		108, 109, 110, 111, 112, 113, 114, 115
	};

	object_collector<test::a, 5> c(true);
	std::transform(
		d_in.begin(), d_in.end(), c, [](int v) -> test::a {
			return test::a(v);
		}
	);

	BOOST_CHECK_EQUAL(c.size(), d_in.size());
	std::array<test::a, d_in.size()> d_out;
	BOOST_CHECK_EQUAL(c.copy(d_out.begin(), d_out.size(), 0), d_out.size());

	BOOST_CHECK(std::equal(
		d_in.begin(), d_in.end(), d_out.begin(),
		[](int v1, test::a const &v2) -> bool {
			return v1 == v2.v;
		}
	));
}

}}}
