/*=============================================================================
    Copyright (c) 2000-2005 Aleksey Gurtovoy
    Copyright (c) 2013      Alex Dubov <oakad@yahoo.com>

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE yesod_mpl
#include <boost/test/unit_test.hpp>

#include <yesod/mpl/at.hpp>
#include <yesod/mpl/size.hpp>
#include <yesod/mpl/clear.hpp>
#include <yesod/mpl/order.hpp>
#include <yesod/mpl/equal.hpp>
#include <yesod/mpl/insert.hpp>
#include <yesod/mpl/vector.hpp>
#include <yesod/mpl/contains.hpp>

namespace ucpf { namespace yesod { namespace mpl {

BOOST_AUTO_TEST_CASE(vector_0)
{
	typedef vector<> v0;
	typedef vector<char> v1;
	typedef vector<char, long> v2;
	typedef vector<char, char, char, char, char, char, char, char, int> v9;

	BOOST_CHECK((equal<v0, v0::type>::value));
	BOOST_CHECK((equal<v1, v1::type>::value));
	BOOST_CHECK((equal<v2, v2::type>::value));
	BOOST_CHECK((equal<v9, v9::type>::value));

	BOOST_CHECK_EQUAL(size<v0>::value, 0);
	BOOST_CHECK_EQUAL(size<v1>::value, 1);
	BOOST_CHECK_EQUAL(size<v2>::value, 2);
	BOOST_CHECK_EQUAL(size<v9>::value, 9);

	BOOST_CHECK((empty<v0>::value));
	BOOST_CHECK(!(empty<v1>::value));
	BOOST_CHECK(!(empty<v2>::value));
	BOOST_CHECK(!(empty<v9>::value));

	BOOST_CHECK((std::is_same<front<v1>::type, char>::value));
	BOOST_CHECK((std::is_same<back<v1>::type, char>::value));
	BOOST_CHECK((std::is_same<front<v2>::type, char>::value));
	BOOST_CHECK((std::is_same<back<v2>::type, long>::value));
	BOOST_CHECK((std::is_same<front<v9>::type, char>::value));
	BOOST_CHECK((std::is_same<back<v9>::type, int>::value));
}

BOOST_AUTO_TEST_CASE(vector_1)
{
	typedef vector<char, long> v2;

	typedef begin<v2>::type i1;
	typedef next<i1>::type i2;
	typedef next<i2>::type i3;

	BOOST_CHECK((std::is_same<deref<i1>::type, char>::value));
	BOOST_CHECK((std::is_same<deref<i2>::type, long>::value));
	BOOST_CHECK((std::is_same< i3, end<v2>::type>::value));
}

BOOST_AUTO_TEST_CASE(vector_2)
{
	typedef vector<> v0;

	typedef push_back<v0, int>::type v1;
	typedef push_front<v1, char>::type v2;
	typedef push_back<v2, long>::type v3;

	BOOST_CHECK((std::is_same<back<v1>::type, int>::value));
	BOOST_CHECK((std::is_same<back<v2>::type, int>::value));
	BOOST_CHECK((std::is_same<front<v2>::type, char>::value));
	BOOST_CHECK((std::is_same<back<v3>::type, long>::value));

	BOOST_CHECK((equal<v1, v1::type>::value));
	BOOST_CHECK((equal<v2, v2::type>::value));
	BOOST_CHECK((equal<v3, v3::type>::value));
}

BOOST_AUTO_TEST_CASE(vector_3)
{
	typedef vector<char, bool, char, char, char, char, bool, long, int> v9;

	typedef pop_back<v9>::type v8;
	typedef pop_front<v8>::type v7;

	BOOST_CHECK((std::is_same<back<v9>::type, int>::value));
	BOOST_CHECK((std::is_same<back<v8>::type, long>::value));
	BOOST_CHECK((std::is_same<back<v7>::type, long>::value));
	BOOST_CHECK((std::is_same<front<v7>::type, bool>::value));

	BOOST_CHECK((equal<v9, v9::type>::value));
	BOOST_CHECK((equal<v8, v8::type>::value));
	BOOST_CHECK((equal<v7, v7::type>::value));
}

BOOST_AUTO_TEST_CASE(vector_4)
{
	typedef vector<> v0;
	typedef vector<char> v1;
	typedef vector<char, long> v2;
	typedef vector<char, char, char, char, char, char, char, char, int> v9;

	BOOST_CHECK((equal<v0, v0::type>::value));
	BOOST_CHECK((equal<v1, v1::type>::value));
	BOOST_CHECK((equal<v2, v2::type>::value));
	BOOST_CHECK((equal<v9, v9::type>::value));

	BOOST_CHECK_EQUAL(size<v0>::value, 0);
	BOOST_CHECK_EQUAL(size<v1>::value, 1);
	BOOST_CHECK_EQUAL(size<v2>::value, 2);
	BOOST_CHECK_EQUAL(size<v9>::value, 9);
}

BOOST_AUTO_TEST_CASE(vector_5)
{
	typedef vector_c<bool, true>::type v1;
	typedef vector_c<bool, false>::type v2;

	BOOST_CHECK((std::is_same<v1::value_type, bool>::value));
	BOOST_CHECK((std::is_same<v2::value_type, bool>::value));

	BOOST_CHECK_EQUAL((front<v1>::type::value), true);
	BOOST_CHECK_EQUAL((front<v2>::type::value), false);
}

BOOST_AUTO_TEST_CASE(vector_6)
{
	typedef vector_c<int, -1> v1;
	typedef vector_c<int, 0, 1> v2;
	typedef vector_c<int, 1, 2, 3> v3;

	BOOST_CHECK((std::is_same<v1::value_type, int>::value));
	BOOST_CHECK((std::is_same<v2::value_type, int>::value));
	BOOST_CHECK((std::is_same<v3::value_type, int>::value));

	BOOST_CHECK_EQUAL((size<v1>::value), 1);
	BOOST_CHECK_EQUAL((size<v2>::value), 2);
	BOOST_CHECK_EQUAL((size<v3>::value), 3);

	BOOST_CHECK_EQUAL((front<v1>::type::value), -1);
	BOOST_CHECK_EQUAL((front<v2>::type::value), 0);
	BOOST_CHECK_EQUAL((front<v3>::type::value), 1);
}

BOOST_AUTO_TEST_CASE(vector_7)
{
	typedef vector_c<unsigned, 0> v1;
	typedef vector_c<unsigned, 1, 2> v2;

	BOOST_CHECK((std::is_same<v1::value_type, unsigned>::value));
	BOOST_CHECK((std::is_same<v2::value_type, unsigned>::value));

	BOOST_CHECK_EQUAL((size<v1>::type::value), 1);
	BOOST_CHECK_EQUAL((size<v2>::type::value), 2);

	BOOST_CHECK_EQUAL((front<v1>::type::value), 0);
	BOOST_CHECK_EQUAL((front<v2>::type::value), 1);
}

}}}
