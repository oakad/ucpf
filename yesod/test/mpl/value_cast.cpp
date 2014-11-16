/*==============================================================================
    Copyright (c) 2013      Alex Dubov <oakad@yahoo.com>

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE yesod_mpl
#include <boost/test/unit_test.hpp>

#include <yesod/mpl/list.hpp>
#include <yesod/mpl/vector.hpp>
#include <yesod/mpl/value_cast.hpp>

#include <string>

namespace ucpf { namespace yesod { namespace mpl {

BOOST_AUTO_TEST_CASE(make_value_0)
{
	typedef package_c<char, 'z', 'y', 'x', 'w', 'v', 'u'> p0;
	typedef value_cast<p0> xv0;
	std::string str0(xv0::value.begin(), xv0::value.end());
	BOOST_CHECK_EQUAL(str0, std::string("zyxwvu"));

	typedef vector_c<char, 'a', 'b', 'c', 'd', 'e'> v1;
	typedef value_cast<v1> xv1;
	std::string str1(xv1::value.begin(), xv1::value.end());
	BOOST_CHECK_EQUAL(str1, std::string("abcde"));

	typedef list_c<int, 1, 2, 3, 4, 5> l2;
	std::array<int, 5> ans2({1, 2, 3, 4, 5});
	typedef value_cast<l2> xv2;
	BOOST_CHECK(std::equal(ans2.begin(), ans2.end(), xv2::value.begin()));

}

}}}
