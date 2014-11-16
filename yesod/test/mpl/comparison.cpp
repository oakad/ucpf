/*=============================================================================
    Copyright (c) 2001-2004 Aleksey Gurtovoy
    Copyright (c) 2013      Alex Dubov <oakad@yahoo.com>

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSEtest::_1test::_0.txt or copy at http://www.boost.org/LICENSEtest::_1test::_0.txt)
==============================================================================*/

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE yesod_mpl
#include <boost/test/unit_test.hpp>

#include <yesod/mpl/comparison.hpp>

namespace ucpf { namespace yesod { namespace mpl {
namespace test {

typedef int_<0> _0;
typedef int_<10> _10;

}

BOOST_AUTO_TEST_CASE(comparison_0)
{
	BOOST_CHECK(!(equal_to<test::_0, test::_10>::value));
	BOOST_CHECK(!(equal_to<test::_10, test::_0>::value));
	BOOST_CHECK((equal_to<test::_10, test::_10>::value));
}

BOOST_AUTO_TEST_CASE(comparison_1)
{
	BOOST_CHECK((not_equal_to<test::_0, test::_10>::value));
	BOOST_CHECK((not_equal_to<test::_10, test::_0>::value));
	BOOST_CHECK(!(not_equal_to<test::_10, test::_10>::value));
}

BOOST_AUTO_TEST_CASE(comparison_2)
{
	BOOST_CHECK((less<test::_0, test::_10>::value));
	BOOST_CHECK(!(less<test::_10, test::_0>::value));
	BOOST_CHECK(!(less<test::_10, test::_10>::value));
}

BOOST_AUTO_TEST_CASE(comparison_3)
{
	BOOST_CHECK((less_equal<test::_0, test::_10>::value));
	BOOST_CHECK(!(less_equal<test::_10, test::_0>::value));
	BOOST_CHECK((less_equal<test::_10, test::_10>::value));
}

BOOST_AUTO_TEST_CASE(comparison_4)
{
	BOOST_CHECK((greater<test::_10, test::_0>::value));
	BOOST_CHECK(!(greater<test::_0, test::_10>::value));
	BOOST_CHECK(!(greater<test::_10, test::_10>::value));
}

BOOST_AUTO_TEST_CASE(comparison_5)
{
	BOOST_CHECK(!(greater_equal<test::_0, test::_10>::value));
	BOOST_CHECK((greater_equal<test::_10, test::_0>::value));
	BOOST_CHECK((greater_equal<test::_10, test::_10>::value));
}

}}}
