/*=============================================================================
    Copyright (c) 2001-2004 Aleksey Gurtovoy
    Copyright (c) 2013      Alex Dubov <oakad@yahoo.com>

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#define BOOST_TEST_MODULE yesod_mpl
#include <boost/test/unit_test.hpp>

#include <yesod/mpl/inherit.hpp>

namespace ucpf { namespace yesod { namespace mpl {
namespace test {

	struct her {
		typedef her herself;
	};

	struct my {
		typedef my myself;
	};
}

BOOST_AUTO_TEST_CASE(inherit_0)
{
	BOOST_CHECK((std::is_same<inherit<test::her>::type, test::her>::value));

	typedef inherit<test::her, test::my>::type her_my1;
	BOOST_CHECK((std::is_same<her_my1::herself, test::her>::value));
	BOOST_CHECK((std::is_same<her_my1::myself, test::my>::value));

	typedef inherit<empty_base, test::her>::type her1;
	BOOST_CHECK((std::is_same<her1, test::her>::value));

	typedef inherit<
		empty_base, test::her, empty_base, empty_base
	>::type her2;
	BOOST_CHECK((std::is_same<her2, test::her>::value));

	typedef inherit<test::her, empty_base, test::my>::type her_my2;
	BOOST_CHECK((std::is_same<her_my2::herself, test::her>::value));
	BOOST_CHECK((std::is_same<her_my2::myself, test::my>::value));

	typedef inherit<empty_base, empty_base>::type empty;
	BOOST_CHECK((std::is_same<empty, empty_base>::value));
}

}}}
