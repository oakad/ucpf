/*=============================================================================
    Copyright (c) 2000-2004 Aleksey Gurtovoy
    Copyright (c) 2013      Alex Dubov <oakad@yahoo.com>

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE yesod_mpl
#include <boost/test/unit_test.hpp>

#include <yesod/mpl/detail/template_arity.hpp>

namespace ucpf { namespace yesod { namespace mpl {
namespace test {

	struct my {};
	template <typename T0> struct my1 {};
	template <typename T0, typename T1 = void> struct my2 {};
	template <typename T0, typename T1, typename T2> struct my3 {};
}

BOOST_AUTO_TEST_CASE(template_arity)
{
	/* BOOST_CHECK_EQUAL would not work due to missing ::value linker
	 * reference.
	 */
	BOOST_CHECK((detail::template_arity<test::my>::value == 0));
	BOOST_CHECK((detail::template_arity<test::my1<int>>::value == 1));
	BOOST_CHECK((\
		detail::template_arity<test::my2<int, long>>::value == 2  \
	));
	BOOST_CHECK((detail::template_arity<test::my2<int>>::value == 2));
	BOOST_CHECK((                                                     \
		detail::template_arity<test::my3<int, long, void>>::value \
		== 3                                                      \
	));
}

}}}
