/*=============================================================================
    Copyright (c) 2000-2004 Aleksey Gurtovoy
    Copyright (c) 2013      Alex Dubov <oakad@yahoo.com>

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE yesod_mpl
#include <boost/test/unit_test.hpp>

#include <yesod/mpl/if.hpp>
#include <yesod/mpl/identity.hpp>

namespace ucpf { namespace yesod { namespace mpl {

BOOST_AUTO_TEST_CASE(if_test_0)
{
	typedef if_<std::true_type, char, long>::type t1;
	typedef if_c<true, char, long>::type t2;
	typedef if_<std::false_type, char, long>::type t3;
	typedef if_c<false, char, long>::type t4;

	BOOST_CHECK((std::is_same<t1, char>::value));
	BOOST_CHECK((std::is_same<t2, char>::value));
	BOOST_CHECK((std::is_same<t3, long>::value));
	BOOST_CHECK((std::is_same<t4, long>::value));
}

BOOST_AUTO_TEST_CASE(if_test_1)
{
	typedef eval_if<
		std::true_type, identity<char>, identity<long>
	>::type t1;
	typedef eval_if_c<
		true, identity<char>, identity<long>
	>::type t2;
	typedef eval_if<
		std::false_type, identity<char>, identity<long>
	>::type t3;
	typedef eval_if_c<
		false, identity<char>, identity<long>
	>::type t4;

	BOOST_CHECK((std::is_same<t1, char>::value));
	BOOST_CHECK((std::is_same<t2, char>::value));
	BOOST_CHECK((std::is_same<t3, long>::value));
	BOOST_CHECK((std::is_same<t4, long>::value));
}

}}}
