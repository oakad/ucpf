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

	m0.emplace("bachelor", 111);
	m0.emplace("jar", 222);
	m0.emplace("badge", 333);
	m0.dump(std::cout);

	m0.emplace("baby", 444);
	printf("----\n");
	m0.dump(std::cout);

	auto r0(m0.find("bachelor"));
	BOOST_CHECK(r0 != nullptr);
	BOOST_CHECK_EQUAL(r0->a, 111);

	auto r1(m0.find("jar"));
	BOOST_CHECK(r1 != nullptr);
	BOOST_CHECK_EQUAL(r1->a, 222);

	auto r2(m0.find("badge"));
	BOOST_CHECK(r2 != nullptr);
	BOOST_CHECK_EQUAL(r2->a, 333);

	auto r3(m0.find("baby"));
	BOOST_CHECK(r3 != nullptr);
	BOOST_CHECK_EQUAL(r3->a, 444);
}

BOOST_AUTO_TEST_CASE(string_map_1)
{
	string_map<char, int> m0;
	m0.emplace(std::string("abcd"), 1);
	m0.emplace(std::string("abcdefghijkl"), 2);
	m0.emplace(std::string("abcdefghijklmnopqrst"), 3);
	m0.emplace(std::string("abcdefghijklmnopqrstuvwxyz"), 4);

	auto p(m0.find(std::string("abcd")));
	BOOST_CHECK_EQUAL(*p, 1);
	p = m0.find(std::string("abcdefghijkl"));
	BOOST_CHECK_EQUAL(*p, 2);
	p = m0.find(std::string("abcdefghijklmnopqrst"));
	BOOST_CHECK_EQUAL(*p, 3);
	p = m0.find(std::string("abcdefghijklmnopqrstuvwxyz"));
	BOOST_CHECK_EQUAL(*p, 4);

	using boost::test_tools::output_test_stream;
        output_test_stream out("ref/string_map/for_each.00.out", true);

	auto r_idx(m0.make_index());

	r_idx.for_each([&out](std::string const &key, int val) -> void {
		out << "x1 |" << key << "| " << val << '\n';
	});
	BOOST_CHECK(out.match_pattern());

	r_idx.for_each_prefix(
		std::string("abcdefghijklmnopqrstu"),
		[&out](std::string const &key, int val) -> void {
			out << "x2 |" << key << "| " << val << '\n';
		}
	);
	BOOST_CHECK(out.match_pattern());

	r_idx.for_each_prefix(
		std::string("abcdefghijkl"),
		[&out](std::string const &key, int val) -> void {
			out << "x3 |" << key << "| " << val << '\n';
		}
	);
	BOOST_CHECK(out.match_pattern());

	r_idx.for_each_prefix(
		std::string("abc"),
		[&out](std::string const &key, int val) -> void {
			out << "x4 |" << key << "| " << val << '\n';
		}
	);
	BOOST_CHECK(out.match_pattern());
}

}}
