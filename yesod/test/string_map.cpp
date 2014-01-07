/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#define BOOST_TEST_MODULE yesod
#include <boost/test/included/unit_test.hpp>

#include <yesod/string_map.hpp>

namespace ucpf { namespace yesod {
namespace test {

struct s {
};

}

BOOST_AUTO_TEST_CASE(string_map_0)
{
	string_map<char, test::s> m0;
}

}}
