/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE yesod
#include <boost/test/unit_test.hpp>

#include <yesod/is_sequence.hpp>
#include <vector>
#include <list>

namespace ucpf { namespace yesod {
namespace test {

struct s0 {
	int a;
};

struct s1 {
	typedef int value_type;
	int begin() const
	{
		return 0;
	}
};

}

BOOST_AUTO_TEST_CASE(is_sequence_0)
{
	BOOST_CHECK(!is_sequence<test::s0>::value);
	BOOST_CHECK(!is_sequence<test::s1>::value);
	BOOST_CHECK(is_sequence<std::vector<int>>::value);
	BOOST_CHECK(is_sequence<std::list<int>>::value);
}

}}
