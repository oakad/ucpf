/*=============================================================================
    Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#define BOOST_TEST_MODULE yesod_iterator
#include <boost/test/included/unit_test.hpp>

#include <yesod/iterator/range.hpp>
#include <yesod/iterator/joined_range.hpp>

namespace ucpf { namespace yesod { namespace iterator {

BOOST_AUTO_TEST_CASE(range_0)
{
	std::array<int, 3> r0 = {{12, 23, 34}};
	std::list<int> r1 = {45, 56, 67};
	std::vector<int> r2 = {78, 89, 90};

	auto j0(make_joined_range(r0, r1, r2));

	std::array<int, 9> ref0 = {{12, 23, 34, 45, 56, 67, 78, 89, 90}};

	BOOST_CHECK(std::equal(j0.begin(), j0.end(), ref0.begin()));
}

BOOST_AUTO_TEST_CASE(range_1)
{
	char const *s0 = "abcd";
	std::string s1("efgh");

	auto r0(make_range(s0, std::strlen(s0)));
	auto j0(make_joined_range(r0, s1));

	std::string ref0("abcdefgh");

	BOOST_CHECK(std::equal(j0.begin(), j0.end(), ref0.begin()));
}

}}}
