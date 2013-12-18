/*==============================================================================
    Copyright (c) 2013      Alex Dubov <oakad@yahoo.com>

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#define BOOST_TEST_MODULE yesod_mpl
#include <boost/test/included/unit_test.hpp>

#include <yesod/mpl/list.hpp>
#include <yesod/mpl/vector.hpp>
#include <yesod/mpl/value_cast.hpp>

#include <string>

namespace ucpf { namespace yesod { namespace mpl {

BOOST_AUTO_TEST_CASE(make_value_0)
{
	typedef vector_c<char, 'a', 'b', 'c', 'd', 'e'> v1;
	auto str1(value_cast<v1, std::string>());

	BOOST_CHECK_EQUAL(str1, std::string("abcde"));

	typedef list_c<int, 1, 2, 3, 4, 5> l2;
	std::vector<int> ans2({1, 2, 3, 4, 5});

	auto v2(value_cast<l2, std::vector<int>>());
	BOOST_CHECK(std::equal(ans2.begin(), ans2.end(), v2.begin()));
}

}}}
