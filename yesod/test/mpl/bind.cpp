/*=============================================================================
    Copyright (c) 2001-2004 Aleksey Gurtovoy
    Copyright (c) 2001-2002 Peter Dimov
    Copyright (c) 2013      Alex Dubov <oakad@yahoo.com>

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#define BOOST_TEST_MODULE yesod_mpl
#include <boost/test/unit_test.hpp>

#include <yesod/mpl/bind.hpp>
#include <yesod/mpl/quote.hpp>
#include <yesod/mpl/arithmetic.hpp>

namespace ucpf { namespace yesod { namespace mpl {
namespace test {

struct f1 {
	template <typename T1>
	struct apply {
		typedef T1 type;
	};
};

struct f5 {
	template <
		typename T1, typename T2, typename T3,
		typename T4, typename T5
	> struct apply {
		typedef T5 type;
	};
};

}

BOOST_AUTO_TEST_CASE(bind_0)
{
	typedef apply_wrap<bind<test::f1, arg<0>>, int>::type r11;
	typedef apply_wrap<bind<test::f1, arg<4>>, void, void, void, void, int>
	::type r12;

	BOOST_CHECK((std::is_same<r11, int>::value));
	BOOST_CHECK((std::is_same<r12, int>::value));

	typedef apply_wrap<
		bind<test::f5, arg<0>, arg<1>, arg<2>, arg<3>, arg<4>>,
		void, void, void, void, int
	>::type r51;
	typedef apply_wrap<
		bind<test::f5, arg<4>, arg<3>, arg<2>, arg<1>, arg<0>>,
		int, void, void, void, void
	>::type r52;

	BOOST_CHECK((std::is_same<r51, int>::value));
	BOOST_CHECK((std::is_same<r52, int>::value));

}

BOOST_AUTO_TEST_CASE(bind_1)
{
	typedef apply_wrap<bind<test::f1, int>>::type r11;
	typedef apply_wrap<
		bind<test::f5, void, void, void, void, int>
	>::type r51;
	BOOST_CHECK((std::is_same<r11,int>::value));
	BOOST_CHECK((std::is_same<r51,int>::value));
}

BOOST_AUTO_TEST_CASE(bind_2)
{
	typedef apply_wrap<
		bind<
			test::f5, arg<0>, arg<1>, arg<2>, arg<3>,
			bind<test::f1, arg<0>>
		>, int, void, void, void, void
	>::type r51;

	typedef apply_wrap<
		bind<
			test::f5, arg<0>, arg<1>, arg<2>, arg<3>,
			bind<test::f1, arg<4>>
		>, void, void, void, void, int
	>::type r52;

	BOOST_CHECK((std::is_same<r51, int>::value));
	BOOST_CHECK((std::is_same<r52, int>::value));
}

BOOST_AUTO_TEST_CASE(bind_3)
{
	typedef bind<
		quote<if_>, arg<0>, bind<quote<increment>, arg<1>>, arg<2>
	> f;

	typedef apply_wrap<f, true_type, int_<0>, int>::type r1;
	typedef apply_wrap<f, false_type, int, int_<0>>::type r2;

	BOOST_CHECK((std::is_same<r1, int_<1>>::value));
	BOOST_CHECK((std::is_same<r2, int_<0>>::value));
}

}}}
