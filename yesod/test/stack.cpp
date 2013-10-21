/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#define BOOST_TEST_MODULE yesod
#include <boost/test/included/unit_test.hpp>

#include <yesod/stack.hpp>

namespace ucpf { namespace yesod {
namespace test {

struct v_type {
	long value;
	slist_node h;

	v_type(long v) : value(v)
	{}
};

}

BOOST_AUTO_TEST_CASE(stack_0)
{
	stack<test::v_type, decltype(test::v_type::h), &test::v_type::h> s;
	test::v_type v0(10), v1(20), v2(30);

	s.push(v2);
	s.push(v1);
	s.push(v0);

	test::v_type *out;
	BOOST_CHECK((out = s.pop()));
	BOOST_CHECK_EQUAL(out->value, 10);
	BOOST_CHECK((out = s.pop()));
	BOOST_CHECK_EQUAL(out->value, 20);
	BOOST_CHECK((out = s.pop()));
	BOOST_CHECK_EQUAL(out->value, 30);
	BOOST_CHECK(!(out = s.pop()));
}

BOOST_AUTO_TEST_CASE(stack_1)
{
	stack<test::v_type, decltype(test::v_type::h), &test::v_type::h> s0;
	test::v_type v0(10), v1(20), v2(30);

	s0.push(v2);
	s0.push(v1);
	s0.push(v0);

	stack<test::v_type, decltype(test::v_type::h), &test::v_type::h> s1;
	test::v_type v3(40), v4(50);

	s1.push(v4);
	s1.push(v3);

	s1.splice(s0);

	test::v_type *out;
	BOOST_CHECK(!(out = s0.pop()));

	BOOST_CHECK((out = s1.pop()));
	BOOST_CHECK_EQUAL(out->value, 10);
	BOOST_CHECK((out = s1.pop()));
	BOOST_CHECK_EQUAL(out->value, 20);
	BOOST_CHECK((out = s1.pop()));
	BOOST_CHECK_EQUAL(out->value, 30);
	BOOST_CHECK((out = s1.pop()));
	BOOST_CHECK_EQUAL(out->value, 40);
	BOOST_CHECK((out = s1.pop()));
	BOOST_CHECK_EQUAL(out->value, 50);
	BOOST_CHECK(!(out = s1.pop()));
}

}}
