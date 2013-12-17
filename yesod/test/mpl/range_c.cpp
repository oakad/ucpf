/*=============================================================================
    Copyright (c) 2000-2004 Aleksey Gurtovoy
    Copyright (c) 2013      Alex Dubov <oakad@yahoo.com>

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#define BOOST_TEST_MODULE yesod_mpl
#include <boost/test/included/unit_test.hpp>

#include <yesod/mpl/size.hpp>
#include <yesod/mpl/range_c.hpp>
#include <yesod/mpl/advance.hpp>
#include <yesod/mpl/distance.hpp>
#include <yesod/mpl/front_back.hpp>

namespace ucpf { namespace yesod { namespace mpl {

BOOST_AUTO_TEST_CASE(range_c_0)
{
	typedef range_c<int, 0, 0> range0;
	typedef range_c<int, 0, 1> range1;
	typedef range_c<int, 0, 10> range10;

	BOOST_CHECK_EQUAL((size<range0>::value), 0);
	BOOST_CHECK_EQUAL((size<range1>::value), 1);
	BOOST_CHECK_EQUAL((size<range10>::value), 10);

	BOOST_CHECK((empty<range0>::value));
	BOOST_CHECK(!(empty<range1>::value));
	BOOST_CHECK(!(empty<range10>::value));

	BOOST_CHECK((std::is_same<                       \
		begin<range0>::type, end<range0>::type   \
	>::value));
	BOOST_CHECK(!(std::is_same<                      \
		begin<range1>::type, end<range1>::type   \
	>::value));
	BOOST_CHECK(!(std::is_same<                      \
		begin<range10>::type, end<range10>::type \
	>::value));

	BOOST_CHECK_EQUAL((front<range1>::type::value), 0);
	BOOST_CHECK_EQUAL((back<range1>::type::value), 0);
	BOOST_CHECK_EQUAL((front<range10>::type::value), 0);
	BOOST_CHECK_EQUAL((back<range10>::type::value), 9);
}

BOOST_AUTO_TEST_CASE(range_c_1)
{
	typedef range_c<unsigned char, 0, 10> r;
	typedef begin<r>::type first;
	typedef end<r>::type last;

	BOOST_CHECK((std::is_same<advance_c<first, 10>::type, last>::value));
	BOOST_CHECK((std::is_same<advance_c<last, -10>::type, first>::value));

	BOOST_CHECK_EQUAL((distance<first, last>::value), 10);

	typedef advance_c<first, 5>::type iter;
	BOOST_CHECK_EQUAL((deref<iter>::type::value), 5);
}

}}}
