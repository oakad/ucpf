/*
 * Copyright (c) 2016 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#define BOOST_TEST_MODULE yesod
#include <boost/test/unit_test.hpp>

#include <yesod/storage/memory.hpp>

namespace ucpf { namespace yesod { namespace storage {
namespace test {

struct s {
	s(int a_, int b_)
	: a(a_), b(b_)
	{}

	int a;
	int b;
};

}

BOOST_AUTO_TEST_CASE(memory_0)
{
	memory m;

	auto p0 = m.allocate(sizeof(test::s));
	auto p1 = m.construct<test::s>(p0, 45, 67);
	BOOST_CHECK_EQUAL(p1->a, 45);
	BOOST_CHECK_EQUAL(p1->b, 67);
	auto p2 = m.storage_cast<test::s>(p0);
	BOOST_CHECK_EQUAL(p2->a, 45);
	BOOST_CHECK_EQUAL(p2->b, 67);
	auto p3 = m.storage_cast(p2);
	BOOST_CHECK((std::is_same<decltype(p0), decltype(p3)>::value));
	m.destroy(p1);
	m.deallocate(p0, sizeof(test::s));
}

}}}
