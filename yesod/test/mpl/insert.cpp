/*=============================================================================
    Copyright (c) 2001-2004 Aleksey Gurtovoy
    Copyright (c) 2013      Alex Dubov <oakad@yahoo.com>

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#define BOOST_TEST_MODULE yesod_mpl
#include <boost/test/included/unit_test.hpp>

#include <yesod/mpl/list.hpp>
#include <yesod/mpl/find.hpp>
#include <yesod/mpl/size.hpp>
#include <yesod/mpl/equal.hpp>
#include <yesod/mpl/clear.hpp>
#include <yesod/mpl/insert.hpp>
#include <yesod/mpl/vector.hpp>
#include <yesod/mpl/range_c.hpp>

namespace ucpf { namespace yesod { namespace mpl {

BOOST_AUTO_TEST_CASE(insert_0)
{
	typedef vector_c<int, 0, 1, 3, 4, 5, 6, 7, 8, 9> numbers;
	typedef find<numbers, int_<3>>::type pos;
	typedef insert<numbers, pos, int_<2>>::type range;

	BOOST_CHECK_EQUAL((size<range>::value), 10);
	BOOST_CHECK((equal<range, range_c<int, 0, 10>>::value));
}

BOOST_AUTO_TEST_CASE(insert_1)
{
	typedef vector_c<int, 0, 1, 7, 8, 9> numbers;
	typedef find<numbers, int_<7>>::type pos;
	typedef insert_range<numbers, pos, range_c<int, 2, 7>>::type range;

	BOOST_CHECK_EQUAL((size<range>::value), 10);
	BOOST_CHECK((equal<range, range_c<int, 0, 10>>::value));

	typedef insert_range<
		list<>, end<list<>>::type, list<int>
	>::type result2;
	BOOST_CHECK_EQUAL((size<result2>::value), 1);
}

}}}
