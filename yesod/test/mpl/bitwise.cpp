/*=============================================================================
    Copyright (c) 2003-2004 Aleksey Gurtovoy
    Copyright (c) 2003      Jaap Suter
    Copyright (c) 2013      Alex Dubov <oakad@yahoo.com>

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSEtest::_1test::_0.txt or copy at http://www.boost.org/LICENSEtest::_1test::_0.txt)
==============================================================================*/

#define BOOST_TEST_MODULE yesod_mpl
#include <boost/test/included/unit_test.hpp>

#include <yesod/mpl/bitwise.hpp>

namespace ucpf { namespace yesod { namespace mpl {
namespace test {

typedef integral_constant<unsigned int, 0> _0;
typedef integral_constant<unsigned int, 1> _1;
typedef integral_constant<unsigned int, 2> _2;
typedef integral_constant<unsigned int, 8> _8;
typedef integral_constant<unsigned int, 0xffffffff> _ffffffff;

}

BOOST_AUTO_TEST_CASE(bitwise_0)
{
	BOOST_CHECK_EQUAL((bitand_<test::_0, test::_0>::value), 0);
	BOOST_CHECK_EQUAL((bitand_<test::_1, test::_0>::value), 0);
	BOOST_CHECK_EQUAL((bitand_<test::_0, test::_1>::value), 0);
	BOOST_CHECK_EQUAL((bitand_<test::_0, test::_ffffffff>::value), 0);
	BOOST_CHECK_EQUAL((bitand_<test::_1, test::_ffffffff>::value), 1);
	BOOST_CHECK_EQUAL((bitand_<test::_8, test::_ffffffff>::value), 8);
}

BOOST_AUTO_TEST_CASE(bitwise_1)
{
	BOOST_CHECK_EQUAL((bitor_<test::_0, test::_0>::value), 0);
	BOOST_CHECK_EQUAL((bitor_<test::_1, test::_0>::value), 1);
	BOOST_CHECK_EQUAL((bitor_<test::_0, test::_1>::value), 1);
	BOOST_CHECK_EQUAL(                                             \
		(bitor_<test::_0, test::_ffffffff>::value), 0xffffffff \
	);
	BOOST_CHECK_EQUAL(                                             \
		(bitor_<test::_1, test::_ffffffff>::value), 0xffffffff \
	);
	BOOST_CHECK_EQUAL(                                             \
		(bitor_<test::_8, test::_ffffffff>::value), 0xffffffff \
	);
}

BOOST_AUTO_TEST_CASE(bitwise_2)
{
	BOOST_CHECK_EQUAL((bitxor_<test::_0, test::_0>::value), 0);
	BOOST_CHECK_EQUAL((bitxor_<test::_1, test::_0>::value), 1);
	BOOST_CHECK_EQUAL((bitxor_<test::_0, test::_1>::value), 1);
	BOOST_CHECK_EQUAL(                                                    \
		(bitxor_<test::_0, test::_ffffffff>::value), (0xffffffff ^ 0) \
	);
	BOOST_CHECK_EQUAL(                                                    \
		(bitxor_<test::_1, test::_ffffffff>::value), (0xffffffff ^ 1) \
	);
	BOOST_CHECK_EQUAL(                                                    \
		(bitxor_<test::_8, test::_ffffffff>::value), (0xffffffff ^ 8) \
	);
}

BOOST_AUTO_TEST_CASE(bitwise_3)
{
	BOOST_CHECK_EQUAL((shift_right<test::_0, test::_0>::value), 0);
	BOOST_CHECK_EQUAL((shift_right<test::_1, test::_0>::value), 1);
	BOOST_CHECK_EQUAL((shift_right<test::_1, test::_1>::value), 0);
	BOOST_CHECK_EQUAL((shift_right<test::_2, test::_1>::value), 1);
	BOOST_CHECK_EQUAL((shift_right<test::_8, test::_1>::value), 4);
}

BOOST_AUTO_TEST_CASE(bitwise_4)
{
	BOOST_CHECK_EQUAL((shift_left<test::_0, test::_0>::value), 0);
	BOOST_CHECK_EQUAL((shift_left<test::_1, test::_0>::value), 1);
	BOOST_CHECK_EQUAL((shift_left<test::_1, test::_1>::value), 2);
	BOOST_CHECK_EQUAL((shift_left<test::_2, test::_1>::value), 4);
	BOOST_CHECK_EQUAL((shift_left<test::_8, test::_1>::value), 16);
}

}}}
