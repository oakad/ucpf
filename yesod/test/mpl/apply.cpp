/*=============================================================================
    Copyright (c) 2000-2004 Aleksey Gurtovoy
    Copyright (c) 2013      Alex Dubov <oakad@yahoo.com>

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSEarg<0>_0.txt or copy at http://www.boost.org/LICENSEarg<0>_0.txt)
==============================================================================*/

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE yesod_mpl
#include <boost/test/unit_test.hpp>

#include <yesod/mpl/apply.hpp>
#include <yesod/mpl/lambda.hpp>
#include <yesod/mpl/arithmetic.hpp>

namespace ucpf { namespace yesod { namespace mpl {
namespace test {

template <typename T>
struct std_vector {};

}

BOOST_AUTO_TEST_CASE(apply_0)
{
	typedef plus<int_<2>, int_<3>> plus1;
	typedef lambda<plus1>::type plus2;
	BOOST_CHECK((std::is_same<plus1, plus2>::value));

	typedef lambda<test::std_vector<int>>::type v;
	BOOST_CHECK((std::is_same<v, test::std_vector<int>>::value));

	typedef lambda<test::std_vector<arg<0>>>::type make_vector;
	typedef apply_wrap<make_vector, int>::type v_int;
	BOOST_CHECK((std::is_same<v_int, test::std_vector<int>>::value));
}

BOOST_AUTO_TEST_CASE(apply_1)
{
	typedef plus<arg<0>, arg<1>> plus_fun;
	typedef apply<plus_fun, int_<2>, int_<3> >::type res;

	BOOST_CHECK_EQUAL((res::value), 5);
}

BOOST_AUTO_TEST_CASE(apply_2)
{
	typedef apply<arg<0>, plus<arg<0>, arg<1>>>::type plus_fun;
	BOOST_CHECK((std::is_same<plus_fun, plus<arg<0>, arg<1>>>::value));

	typedef apply<plus_fun, int_<2>, int_<3>>::type res;
	BOOST_CHECK_EQUAL((res::value), 5);
}

BOOST_AUTO_TEST_CASE(apply_3)
{
	typedef lambda<lambda<arg<0>>>::type make_lambda;
	typedef apply_wrap<make_lambda, test::std_vector<int>>::type v;
	BOOST_CHECK((std::is_same<v, test::std_vector<int>>::value));

	typedef apply_wrap<
		make_lambda, test::std_vector<arg<0>>
	>::type make_vector;
	typedef apply_wrap<make_vector, int>::type v_int;
	BOOST_CHECK((std::is_same<v_int, test::std_vector<int>>::value));
}

BOOST_AUTO_TEST_CASE(apply_4)
{
	typedef apply<arg<0>, test::std_vector<int>>::type v;
	BOOST_CHECK((std::is_same<v, test::std_vector<int>>::value));

	typedef apply<arg<0>, test::std_vector<arg<0>>>::type v_lambda;
	typedef apply<v_lambda, int>::type v_int;
	BOOST_CHECK((std::is_same<v_int, test::std_vector<int>>::value));
}

BOOST_AUTO_TEST_CASE(apply_5)
{
	typedef apply<lambda<arg<0>>, test::std_vector<int>>::type v;
	BOOST_CHECK((std::is_same<v, test::std_vector<int>>::value));

	typedef apply<
		lambda<arg<0>>, test::std_vector<arg<0>>
	>::type make_vector;
	typedef apply_wrap<make_vector, int>::type v_int;
	BOOST_CHECK((std::is_same<v_int, test::std_vector<int>>::value));
}

BOOST_AUTO_TEST_CASE(apply_6)
{
	typedef apply<lambda<arg<0>>, plus<arg<0>, arg<1>>>::type plus_fun;
	typedef apply_wrap<plus_fun, int_<2>, int_<3>>::type res;

	BOOST_CHECK_EQUAL((res::value), 5);
}

BOOST_AUTO_TEST_CASE(apply_7)
{
	typedef bind<plus<>, arg<0>, arg<0>> b1;
	typedef lambda<b1>::type b2;
	BOOST_CHECK((std::is_same<b1, b2>::value));
}

BOOST_AUTO_TEST_CASE(apply_8)
{
	typedef lambda<lambda<bind<plus<>, arg<0>, arg<0>>>>::type make_lambda;
	typedef apply_wrap<make_lambda::type, int_<5>>::type res;
	BOOST_CHECK_EQUAL((res::value), 10);
}

BOOST_AUTO_TEST_CASE(apply_9)
{
	typedef apply<bind<plus<>, arg<0>, arg<0>>, int_<5>>::type res;
	BOOST_CHECK_EQUAL((res::value), 10);
}

BOOST_AUTO_TEST_CASE(apply_10)
{
	typedef apply<arg<0>, lambda<plus<arg<0>, arg<1>>>>::type plus_fun;
	typedef apply_wrap<plus_fun::type, int_<2>, int_<3>>::type res;

	BOOST_CHECK_EQUAL((res::value), 5);
}

}}}
