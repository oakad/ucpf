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
	s(int a_)
	: a(a_)
	{}

	int a;
};

std::ostream &operator<<(std::ostream &os, s const &v)
{
	os << "s(" << v.a << ')';
	return os;
}

}

BOOST_AUTO_TEST_CASE(string_map_0)
{
	string_map<char, test::s> m0;

	m0.emplace_at("bachelor", 111);
	m0.emplace_at("jar", 222);

	m0.dump(std::cout);

	auto r0(m0.find("bachelor"));
	BOOST_CHECK(r0 != nullptr);
	BOOST_CHECK_EQUAL(r0->a, 111);

	auto r1(m0.find("jar"));
	BOOST_CHECK(r1 != nullptr);
	BOOST_CHECK_EQUAL(r1->a, 222);

	BOOST_CHECK(false);
}

}}
