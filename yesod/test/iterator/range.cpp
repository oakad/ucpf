/*=============================================================================
    Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#define BOOST_TEST_MODULE yesod_iterator
#include <boost/test/unit_test.hpp>

#include <yesod/iterator/range.hpp>

namespace ucpf { namespace yesod { namespace iterator {

BOOST_AUTO_TEST_CASE(range_0)
{
	std::array<int, 9> a0 = {{12, 23, 34, 45, 56, 67, 78, 89, 90}};
	auto r0(make_range(a0));
	std::vector<int> v0;
	std::copy(r0.begin(), r0.end(), std::back_inserter(v0));
	BOOST_CHECK(std::equal(a0.begin(), a0.end(), v0.begin()));
}

}}}
