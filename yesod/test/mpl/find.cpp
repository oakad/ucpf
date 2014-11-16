/*=============================================================================
    Copyright (c) 2000-2004 Aleksey Gurtovoy
    Copyright (c) 2013      Alex Dubov <oakad@yahoo.com>

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE yesod_mpl
#include <boost/test/unit_test.hpp>

#include <yesod/mpl/find.hpp>
#include <yesod/mpl/size.hpp>
#include <yesod/mpl/list.hpp>
#include <yesod/mpl/vector.hpp>

namespace ucpf { namespace yesod { namespace mpl {
namespace test {

typedef vector<
	int, char, long, short, char, long, double, float, char
>::type types;

typedef begin<types>::type first_;

}

BOOST_AUTO_TEST_CASE(find_0)
{
	typedef list<
		int, char, long, short, char, long, double,long
	>::type types;
	typedef list_c<int, 1, 0, 5, 1, 7, 5, 0, 5> values;

	typedef find<types, short>::type types_iter;
	typedef find<values, integral_constant<int, 7>>::type values_iter;

	BOOST_CHECK((std::is_same<deref<types_iter>::type, short>::value));
	BOOST_CHECK_EQUAL((deref<values_iter>::type::value), 7);

	typedef begin<types>::type types_first;
	typedef begin<values>::type values_first;
	BOOST_CHECK_EQUAL((distance<types_first, types_iter>::value), 3);
	BOOST_CHECK_EQUAL((distance<values_first, values_iter>::value), 4);
}

BOOST_AUTO_TEST_CASE(find_1)
{
	typedef find_if<
		test::types, std::is_floating_point<arg<-1>>
	>::type iter;
	BOOST_CHECK((std::is_same<iter::type, double>::value));
	BOOST_CHECK_EQUAL((distance<test::first_, iter>::value), 6);
}

BOOST_AUTO_TEST_CASE(find_2)
{
	typedef find_if<test::types, std::is_same<arg<-1>, long>>::type iter;
	BOOST_CHECK((std::is_same<iter::type, long>::value));
	BOOST_CHECK_EQUAL((distance<test::first_, iter>::value), 2);
}

BOOST_AUTO_TEST_CASE(find_3)
{
	typedef find_if<test::types, std::is_same<arg<-1>, void>>::type iter;
	BOOST_CHECK((std::is_same<iter, end<test::types>::type>::value));
	BOOST_CHECK_EQUAL((                           \
		distance<test::first_, iter>::value), \
		(size<test::types>::value             \
	));
}

}}}
