/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE yesod
#include <boost/test/unit_test.hpp>

#include <map>
#include <yesod/string_map.hpp>
#include <yesod/coder/xxhash.hpp>
#include <yesod/iterator/range.hpp>

namespace std {

ostream &operator<<(ostream &os, pair<uint32_t, string> const &v)
{
	os << '|' << v.second << "|(" << v.first << ')';
	return os;
}

}

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

namespace iterator {

std::ostream &operator<<(std::ostream &os, range<char *> const &r)
{
	for (auto c: r)
		os << c;

	return os;
}

}

BOOST_AUTO_TEST_CASE(string_map_0)
{
	string_map<char, test::s> m0;

	m0.emplace("bachelor", 111);
	m0.emplace("jar", 222);
	m0.emplace("badge", 333);
	m0.emplace("baby", 444);

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

	m0.for_each([&out](char *first, char *last, int val) -> bool {
		auto key(iterator::make_range(first, last));
		out << "x1 |" << key << "| " << val << '\n';
		return false;
	});
	BOOST_CHECK(out.match_pattern());

	m0.for_each(
		std::string("abcdefghijklmnopqrstu"),
		[&out](char *first, char *last, int val) -> bool {
			auto key(iterator::make_range(first, last));
			out << "x2 |" << key << "| " << val << '\n';
			return false;
		}
	);
	BOOST_CHECK(out.match_pattern());

	m0.for_each(
		std::string("abcdefghijkl"),
		[&out](char *first, char *last, int val) -> bool {
			auto key(iterator::make_range(first, last));
			out << "x3 |" << key << "| " << val << '\n';
			return false;
		}
	);
	BOOST_CHECK(out.match_pattern());

	m0.for_each(
		std::string("abc"),
		[&out](char *first, char *last, int val) -> bool {
			auto key(iterator::make_range(first, last));
			out << "x4 |" << key << "| " << val << '\n';
			return false;
		}
	);
	BOOST_CHECK(out.match_pattern());
}

BOOST_AUTO_TEST_CASE(string_map_2)
{
	std::ifstream f_in("ref/string_map/names.00.in");
	string_map<char, uint32_t> m0;
	std::map<uint32_t, std::string> ref0;
	std::string s_in;
	coder::xxhash<> h;

	while (std::getline(f_in, s_in)) {
		h.update(s_in.c_str(), s_in.size());
		auto d(h.digest());
		m0.emplace(s_in, d);
		ref0.emplace(d, s_in);
		h.reset(0);
		s_in.clear();
	}
	f_in.close();

	std::map<uint32_t, std::string> ref1;
	m0.for_each([&ref1, &h](char *first, char *last, uint32_t v) -> bool {
		std::string s_out(first, last);
		h.update(first, last);
		auto d(h.digest());
		BOOST_CHECK_EQUAL(d, v);
		ref1.emplace(d, s_out);
		h.reset(0);
		return false;
	});

	BOOST_CHECK_EQUAL(ref0.size(), ref1.size());
	BOOST_CHECK_EQUAL_COLLECTIONS(
		ref0.begin(), ref0.end(), ref1.begin(), ref1.end()
	);
}

}}
