/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#define BOOST_TEST_MODULE yesod_iterator
#include <boost/test/unit_test.hpp>

#include <yesod/iterator/seq_range.hpp>

namespace ucpf { namespace yesod { namespace iterator {
namespace test {

struct s {
	s()
	{}

	s(std::initializer_list<int> v_)
	: v(v_.begin(), v_.end())
	{}

	std::vector<int> v;
};

};

BOOST_AUTO_TEST_CASE(seq_range_0)
{
	std::vector<std::vector<int>> l0{
		{11, 12, 13},
		{14, 15, 16},
		{17, 18, 19}
	};
	std::vector<int> r0{
		11, 12, 13, 14, 15, 16, 17, 18, 19
	};

	auto s0(make_seq_range(l0));
	BOOST_CHECK_EQUAL_COLLECTIONS(
		s0.begin(), s0.end(), r0.begin(), r0.end()
	);

	auto iter_0(s0.begin());
	std::advance(iter_0, 7);
	BOOST_CHECK_EQUAL(*iter_0, 18);
	auto iter_1(s0.begin());
	std::advance(iter_1, 2);
	BOOST_CHECK_EQUAL(*iter_1, 13);
	BOOST_CHECK_EQUAL(std::distance(iter_0, iter_1), -5);
	BOOST_CHECK_EQUAL(std::distance(iter_1, iter_0), 5);
}

BOOST_AUTO_TEST_CASE(seq_range_1)
{
	std::vector<test::s> l0{
		{11, 12, 13},
		{14, 15, 16},
		{17, 18, 19}
	};
	std::vector<int> r0{
		11, 12, 13, 14, 15, 16, 17, 18, 19
	};

	seq_range_m<
		decltype(l0), decltype(test::s::v), &test::s::v
	> s0(l0);
	BOOST_CHECK_EQUAL_COLLECTIONS(
		s0.begin(), s0.end(), r0.begin(), r0.end()
	);

	auto iter_0(s0.begin());
	std::advance(iter_0, 7);
	BOOST_CHECK_EQUAL(*iter_0, 18);
	auto iter_1(s0.begin());
	std::advance(iter_1, 2);
	BOOST_CHECK_EQUAL(*iter_1, 13);
	BOOST_CHECK_EQUAL(std::distance(iter_0, iter_1), -5);
	BOOST_CHECK_EQUAL(std::distance(iter_1, iter_0), 5);
}

}}}
