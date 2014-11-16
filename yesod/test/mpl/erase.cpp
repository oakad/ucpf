/*=============================================================================
    Copyright (c) 2000-2004 Aleksey Gurtovoy
    Copyright (c) 2013      Alex Dubov <oakad@yahoo.com>

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE yesod_mpl
#include <boost/test/unit_test.hpp>

#include <yesod/mpl/list.hpp>
#include <yesod/mpl/find.hpp>
#include <yesod/mpl/size.hpp>
#include <yesod/mpl/erase.hpp>

namespace ucpf { namespace yesod { namespace mpl {

BOOST_AUTO_TEST_CASE(erase_0)
{
	typedef list<int, char, long, short, char, long, double, long> types;
	typedef find<types, short>::type iter;

	typedef erase<types, iter>::type result;
	BOOST_CHECK_EQUAL((size<result>::value), 7);

	typedef find<result, short>::type result_iter;
	BOOST_CHECK((std::is_same<result_iter, end<result>::type>::value));
}

BOOST_AUTO_TEST_CASE(erase_1)
{
	typedef list_c<int, 1, 0, 5, 1, 7, 5, 0, 5> values;
	typedef find<values, integral_constant<int, 7>>::type iter;

	typedef erase<values, iter>::type result;
	BOOST_CHECK_EQUAL((size<result>::value), 7);

	typedef find<result, integral_constant<int, 7>>::type result_iter;
	BOOST_CHECK((std::is_same<result_iter, end<result>::type>::value));
}

BOOST_AUTO_TEST_CASE(erase_2)
{
	typedef list<
		int, char, long, short, unsigned, long, double, long
	> types;

	typedef find<types, short>::type iter1;
	typedef find<types, double>::type iter2;

	typedef erase<types,iter1,iter2>::type result;

	BOOST_CHECK_EQUAL((size<result>::value), 5);

	typedef find<result,unsigned>::type iter;
	BOOST_CHECK((std::is_same<iter, end<result>::type>::value));
}

}}}
