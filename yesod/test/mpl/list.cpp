/*=============================================================================
    Copyright (c) 2001-2004 Aleksey Gurtovoy
    Copyright (c) 2013      Alex Dubov <oakad@yahoo.com>

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#define BOOST_TEST_MODULE yesod_mpl
#include <boost/test/unit_test.hpp>

#include <yesod/mpl/list.hpp>
#include <yesod/mpl/size.hpp>

namespace ucpf { namespace yesod { namespace mpl {

BOOST_AUTO_TEST_CASE(list_0)
{
	typedef list<> l0;
	typedef list<char> l1;
	typedef list<long, char> l2;
	typedef list<char, char, char, char, char, char, char, char, char> l9;

	BOOST_CHECK_EQUAL((size<l0>::value), 0);
	BOOST_CHECK_EQUAL((size<l1>::value), 1);
	BOOST_CHECK_EQUAL((size<l2>::value), 2);
	BOOST_CHECK_EQUAL((size<l9>::value), 9);

	BOOST_CHECK((empty<l0>::value));
	BOOST_CHECK(!(empty<l1>::value));
	BOOST_CHECK(!(empty<l2>::value));
	BOOST_CHECK(!(empty<l9>::value));

	BOOST_CHECK((std::is_same<front<l1>::type, char>::value));
	BOOST_CHECK((std::is_same<front<l2>::type, long>::value));
	BOOST_CHECK((std::is_same<front<l9>::type, char>::value));
}

BOOST_AUTO_TEST_CASE(list_1)
{
	typedef mpl::list<char, long> l2;

	typedef mpl::begin<l2>::type i1;
	typedef next<i1>::type i2;
	typedef next<i2>::type i3;

	BOOST_CHECK((std::is_same<deref<i1>::type, char>::value));
	BOOST_CHECK((std::is_same<deref<i2>::type, long>::value));
	BOOST_CHECK((std::is_same<i3, end<l2>::type>::value));
}

BOOST_AUTO_TEST_CASE(list_2)
{
	typedef list<> l0;
	typedef push_front<l0, char>::type l1;

	BOOST_CHECK((std::is_same<front<l1>::type, char>::value));

	typedef push_front<l1, long>::type l2;
	BOOST_CHECK((std::is_same<front<l2>::type, long>::value));
}

BOOST_AUTO_TEST_CASE(list_3)
{
	typedef list_c<bool, true>::type l1;
	typedef list_c<bool, false>::type l2;

	BOOST_CHECK((std::is_same<l1::value_type, bool>::value));
	BOOST_CHECK((std::is_same<l2::value_type, bool>::value));

	BOOST_CHECK_EQUAL((front<l1>::type::value), true);
	BOOST_CHECK_EQUAL((front<l2>::type::value), false);
}

BOOST_AUTO_TEST_CASE(list_4)
{
	typedef list_c<int, -1>::type l1;
	typedef list_c<int, 0, 1>::type l2;
	typedef list_c<int, 1, 2, 3>::type l3;

	BOOST_CHECK((std::is_same<l1::value_type, int>::value));
	BOOST_CHECK((std::is_same<l2::value_type, int>::value));
	BOOST_CHECK((std::is_same<l3::value_type, int>::value));

	BOOST_CHECK_EQUAL((size<l1>::value), 1);
	BOOST_CHECK_EQUAL((size<l2>::value), 2);
	BOOST_CHECK_EQUAL((size<l3>::value), 3);
	BOOST_CHECK_EQUAL((front<l1>::type::value), -1);
	BOOST_CHECK_EQUAL((front<l2>::type::value), 0);
	BOOST_CHECK_EQUAL((front<l3>::type::value), 1);
}

BOOST_AUTO_TEST_CASE(list_5)
{
	typedef list_c<unsigned, 0>::type l1;
	typedef list_c<unsigned, 1, 2>::type l2;

	BOOST_CHECK((std::is_same<l1::value_type, unsigned>::value));
	BOOST_CHECK((std::is_same<l2::value_type, unsigned>::value));

	BOOST_CHECK_EQUAL((size<l1>::value), 1);
	BOOST_CHECK_EQUAL((size<l2>::value), 2);
	BOOST_CHECK_EQUAL((front<l1>::type::value), 0);
	BOOST_CHECK_EQUAL((front<l2>::type::value), 1);
}

BOOST_AUTO_TEST_CASE(list_6)
{
	typedef list_c<unsigned, 2, 1> l2;

	BOOST_CHECK((std::is_same<l2::value_type, unsigned>::value));

	typedef begin<l2>::type i1;
	typedef next<i1>::type i2;
	typedef next<i2>::type i3;

	BOOST_CHECK_EQUAL((deref<i1>::type::value), 2);
	BOOST_CHECK_EQUAL((deref<i2>::type::value), 1);
	BOOST_CHECK((std::is_same<i3, end<l2>::type>::value));
}

}}}
