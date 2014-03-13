/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#define BOOST_TEST_MODULE yesod
#include <boost/test/included/unit_test.hpp>

#include <yesod/coder/base86.hpp>

namespace ucpf { namespace yesod { namespace coder {

BOOST_AUTO_TEST_CASE(base85_0)
{
	unsigned char const *in(reinterpret_cast<unsigned char const *>(
		"HelloWorld"
	)), *in_p(in);
	uint32_t v1(0), v2(0);
	uint32_t v3(0x15c1ba2b), v4(0x6b716e3a);
	base86::decode(v1, in_p);
	base86::decode(v2, in_p);
	BOOST_CHECK_EQUAL(v1, v3);
	BOOST_CHECK_EQUAL(v2, v4);

	unsigned char out[11] = {0};
	unsigned char *out_p(&out[0]);
	base86::encode(out_p, v3);
	base86::encode(out_p, v4);
	BOOST_CHECK(0 == std::strcmp(
		reinterpret_cast<char const *>(out),
		reinterpret_cast<char const *>(in)
	));
}

BOOST_AUTO_TEST_CASE(base85_1)
{
	static std::random_device src;
	std::mt19937 gen(src());
	std::uniform_int_distribution<uint32_t> dis;
	constexpr static int count = 10000;
	std::array<unsigned char, 6> buf;
	unsigned char *p;
	buf[5] = 0;

	for (int c(0); c < count; ++c) {
		uint32_t d_r(dis(gen));
		p = &buf.front();
		base86::encode(p, d_r);
		uint32_t d_d(0);
		p = &buf.front();
		base86::decode(d_d, p);
		BOOST_CHECK_EQUAL(d_r, d_d);
	}
}

}}}
