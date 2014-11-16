/*==============================================================================
    Copyright (c) 2000-2004 Aleksey Gurtovoy
    Copyright (c) 2013      Alex Dubov <oakad@yahoo.com>

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE yesod_mpl
#include <boost/test/unit_test.hpp>

#include <yesod/mpl/integral.hpp>
#include <yesod/mpl/next_prior.hpp>

namespace ucpf { namespace yesod { namespace mpl {

BOOST_AUTO_TEST_CASE(next_prior_0)
{
	typedef int_<0> _0;
	typedef int_<1> _1;
	typedef int_<2> _2;

	BOOST_CHECK((std::is_same<next<_0>::type, _1>::value));
	BOOST_CHECK((std::is_same<next<_1>::type, _2>::value));
	BOOST_CHECK((std::is_same<prior<_1>::type, _0>::value));
	BOOST_CHECK((std::is_same<prior<_2>::type, _1>::value));
}

}}}
