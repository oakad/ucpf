/*=============================================================================
    Copyright (c) 2000-2004 Aleksey Gurtovoy
    Copyright (c) 2013      Alex Dubov <oakad@yahoo.com>

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#define BOOST_TEST_MODULE yesod_mpl
#include <boost/test/included/unit_test.hpp>

#include <yesod/mpl/count.hpp>
#include <yesod/mpl/vector.hpp>
#include <yesod/mpl/comparison.hpp>

namespace ucpf { namespace yesod { namespace mpl {

BOOST_AUTO_TEST_CASE(count_0)
{
	typedef vector<int, char, long, short, char, long, double, long> types;

	BOOST_CHECK_EQUAL((count<types, int>::value), 1);
	BOOST_CHECK_EQUAL((count<types, double>::value), 1);
	BOOST_CHECK_EQUAL((count<types, char>::value), 2);
	BOOST_CHECK_EQUAL((count<types, long>::value), 3);
	BOOST_CHECK_EQUAL((count<types, unsigned>::value), 0);
}

BOOST_AUTO_TEST_CASE(count_1)
{
	typedef vector_c<int, 1, 0, 5, 1, 7, 5, 0, 5> values;

	BOOST_CHECK_EQUAL((count<values, integral_constant<int, 1>>::value), 2);
	BOOST_CHECK_EQUAL((count<values, integral_constant<int, 0>>::value), 2);
	BOOST_CHECK_EQUAL((count<values, integral_constant<int, 5>>::value), 3);
	BOOST_CHECK_EQUAL((count<values, integral_constant<int, 7>>::value), 1);
	BOOST_CHECK_EQUAL((count<values, integral_constant<int, 8>>::value), 0);
}

BOOST_AUTO_TEST_CASE(count_2)
{
	typedef vector<
		int, char &, long, short, char &, long, double, long
	> types;
	typedef vector_c<int, 1, 0, 5, 1, 7, 5, 0, 5> values;

	BOOST_CHECK_EQUAL((count_if<                   \
		types, std::is_floating_point<arg<-1>> \
	>::value), 1);
	BOOST_CHECK_EQUAL((count_if<                   \
		types, std::is_same<arg<-1>, char &>   \
	>::value), 2);
	BOOST_CHECK_EQUAL((count_if<                   \
		types, std::is_same<arg<-1>, void *>   \
	>::value), 0);
	BOOST_CHECK_EQUAL((count_if<                   \
		values, less<arg<-1>, int_<5>>         \
	>::value), 4);
	BOOST_CHECK_EQUAL((count_if<                   \
		values, equal_to<int_<0>, arg<-1>>     \
	>::value), 2);
	BOOST_CHECK_EQUAL((count_if<                   \
		values, equal_to<int_<-1>, arg<-1>>    \
	>::value), 0);
}

}}}
