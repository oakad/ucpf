/*=============================================================================
    Copyright (c) 2001-2004 Aleksey Gurtovoy
    Copyright (c) 2013      Alex Dubov <oakad@yahoo.com>

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#define BOOST_TEST_MODULE yesod_mpl
#include <boost/test/included/unit_test.hpp>

#include <yesod/mpl/arithmetic.hpp>

namespace ucpf { namespace yesod { namespace mpl {

BOOST_AUTO_TEST_CASE(arithmetic)
{
	typedef int_<0> _0;
	typedef int_<1> _1;
	typedef int_<3> _3;
	typedef int_<10> _10;

	BOOST_CHECK_EQUAL((plus<_0, _10>::value), 10);
	BOOST_CHECK_EQUAL((plus<_10, _0>::value), 10);
	BOOST_CHECK_EQUAL((plus<_10, _1, _10, _10, _1>::value), 32);

	BOOST_CHECK_EQUAL((minus<_0, _10>::value), -10);
	BOOST_CHECK_EQUAL((minus<_10, _0>::value), 10);
	BOOST_CHECK_EQUAL((minus<_10, _1, _10, _10, _1>::value), -12);

	BOOST_CHECK_EQUAL((times<_1, _10>::value), 10);
	BOOST_CHECK_EQUAL((times<_10, _1>::value), 10);
	BOOST_CHECK_EQUAL((times<_1, _10, _10>::value), 100);
	BOOST_CHECK_EQUAL((times<_10, _1, _10>::value), 100);

	BOOST_CHECK_EQUAL((divides<_10, _1>::value), 10);
	BOOST_CHECK_EQUAL((divides<_10, _1>::value), 10);
	BOOST_CHECK_EQUAL((divides<_10, _1, _10>::value), 1);
	BOOST_CHECK_EQUAL((divides<_10, _1 ,_10>::value), 1);

	BOOST_CHECK_EQUAL((modulus<_10, _1>::value), 0);
	BOOST_CHECK_EQUAL((modulus<_10, _3>::value), 1);

	BOOST_CHECK_EQUAL((negate<_10>::value), -10);
	BOOST_CHECK_EQUAL((increment<_10>::value), 11);
	BOOST_CHECK_EQUAL((decrement<_10>::value), 9);
}

}}}
