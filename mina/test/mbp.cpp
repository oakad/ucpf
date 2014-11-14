/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE mina
#include <boost/test/unit_test.hpp>

#include <mina/mbp.hpp>
#include <string>

namespace ucpf { namespace mina { namespace mbp {
namespace test {

struct a {
	a() : x(0), y(0), z(0) {}

	a(int x_, uint16_t y_, int64_t z_)
	: x(x_), y(y_), z(z_) {}

	bool operator==(a const &other)
	{
		return (x == other.x) && (y == other.y) && (z == other.z);
	}

	int x = 0x134567;
	uint16_t y = 0x6563;
	int64_t z = 0x109483209482309LL;
};

}

template <>
struct custom<test::a> {
	template <typename OutputIterator>
	static void pack(OutputIterator &sink, test::a const &va)
	{
		mbp::pack(sink, va.x, va.y, va.z);
	}

	template <typename ForwardIterator>
	static bool unpack(
		ForwardIterator &first, ForwardIterator last, test::a &va
	)
	{
		return mbp::unpack(first, last, va.x, va.y, va.z);
	}
};

BOOST_AUTO_TEST_CASE(mbp_0)
{
	std::vector<uint8_t> buf;
	auto sink(std::back_inserter(buf));

	pack(sink, 0xfedbca98);
	pack(sink, 8, -1, 0x1000, 0x10000, -15);
	pack(sink, 4637.534f, 2394.235);

	auto pos(buf.begin());
	uint32_t v1;
	BOOST_CHECK(unpack(pos, buf.end(), v1));
	BOOST_CHECK_EQUAL(v1, 0xfedbca98);
	int8_t v2, v3;
	int16_t v4;
	int v5, v6;
	BOOST_CHECK(unpack(pos, buf.end(), v2, v3, v4, v5, v6));
	BOOST_CHECK_EQUAL(int(v2), 8);
	BOOST_CHECK_EQUAL(int(v3), -1);
	BOOST_CHECK_EQUAL(int(v4), 0x1000);
	BOOST_CHECK_EQUAL(v5, 0x10000);
	BOOST_CHECK_EQUAL(v6, -15);
	float v7;
	double v8;
	BOOST_CHECK(unpack(pos, buf.end(), v7, v8));
	BOOST_CHECK_EQUAL(v7, 4637.534f);
	BOOST_CHECK_EQUAL(v8, 2394.235);
}

BOOST_AUTO_TEST_CASE(pack_1)
{
	std::vector<uint8_t> buf;
	auto sink(std::back_inserter(buf));

	test::a a0(0x134567, 0x6563, 0x109483209482309LL);
	std::vector<float> b0 = {1.1, 2.2, 3.3, 4.4};
	std::string c0("abcdefghijklmn");
	auto d0 = "nhfejlj";

	pack(sink, a0, b0, c0, d0, "shgfsd");
	test::a a1;
	std::vector<float> b1;
	std::string c1, d1, e1;
	auto pos(buf.begin());
	BOOST_CHECK(unpack(pos, buf.end(), a1, b1, c1, d1, e1));
	BOOST_CHECK(a1 == a0);
	BOOST_CHECK(b1 == b0);
	BOOST_CHECK_EQUAL(c1, c0);
	BOOST_CHECK_EQUAL(d1, d0);
	BOOST_CHECK_EQUAL(e1, "shgfsd");
}

}}}
