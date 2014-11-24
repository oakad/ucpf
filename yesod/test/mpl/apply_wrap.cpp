/*=============================================================================
    Copyright (c) 2000-2004 Aleksey Gurtovoy
    Copyright (c) 2013      Alex Dubov <oakad@yahoo.com>

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#define BOOST_TEST_MODULE yesod_mpl
#include <boost/test/unit_test.hpp>

#include <yesod/mpl/apply.hpp>

namespace ucpf { namespace yesod { namespace mpl {

namespace test {

struct f0 {
	template <typename T = int>
	struct apply { typedef char type; };
};

struct g0 {
	struct apply { typedef char type; };
};

struct first1 {
	template <typename U0>
	struct apply { typedef U0 type; };
};

struct last1 {
	template <typename U0> struct apply { typedef U0 type; };
};

struct first2 {
	template <typename U0, typename U1>
	struct apply { typedef U0 type; };
};

struct last2 {
	template <typename U0, typename U1>
	struct apply { typedef U1 type; };
};

struct first3 {
	template <typename U0, typename U1, typename U2>
	struct apply { typedef U0 type; };
};

struct last3 {
	template <typename U0, typename U1, typename U2>
	struct apply { typedef U2 type; };
};

struct first4 {
	template <typename U0, typename U1, typename U2, typename U3>
	struct apply { typedef U0 type; };
};

struct last4 {
	template <typename U0, typename U1, typename U2, typename U3>
	struct apply { typedef U3 type; };
};

}

BOOST_AUTO_TEST_CASE(apply_wrap_0)
{
	typedef apply_wrap<test::f0>::type t;

	BOOST_CHECK((std::is_same<t, char>::value));
}

BOOST_AUTO_TEST_CASE(apply_wrap_1)
{
	typedef apply_wrap<test::g0>::type t;
	BOOST_CHECK((std::is_same<t, char>::value));
}

BOOST_AUTO_TEST_CASE(apply_wrap_2)
{
	typedef apply_wrap<test::first1, char>::type t11;
	typedef apply_wrap<test::last1 , char>::type t21;

	BOOST_CHECK((std::is_same<t11, char>::value));
	BOOST_CHECK((std::is_same<t21, char>::value));

	typedef apply_wrap<test::first2, char, int>::type t12;
	typedef apply_wrap<test::last2, int, char>::type t22;

	BOOST_CHECK((std::is_same<t12, char>::value));
	BOOST_CHECK((std::is_same<t22, char>::value));
	
	typedef apply_wrap<test::first3, char, int, int>::type t13;
	typedef apply_wrap<test::last3, int, int, char>::type t23;

	BOOST_CHECK((std::is_same<t13, char>::value));
	BOOST_CHECK((std::is_same<t23, char>::value));

	typedef apply_wrap<test::first4, char, int, int, int>::type t14;
	typedef apply_wrap<test::last4, int, int, int, char>::type t24;

	BOOST_CHECK((std::is_same<t14, char>::value));
	BOOST_CHECK((std::is_same<t24, char>::value));
}

}}}
