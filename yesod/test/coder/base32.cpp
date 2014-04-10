/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#define BOOST_TEST_MODULE yesod
#include <boost/test/included/unit_test.hpp>

#include <yesod/coder/base32.hpp>

namespace ucpf { namespace yesod { namespace coder {

BOOST_AUTO_TEST_CASE(base32_0)
{
	auto const *in(reinterpret_cast<uint8_t const *>(
		"He11oWor1dUgy"
	)), *in_p(in);
	uint64_t v1(0), v2(0x34c72242909491cull);
	base32::decode(v1, in_p);
	BOOST_CHECK_EQUAL(v1, v2);

	uint8_t out[13] = {0};
	uint8_t *out_p(&out[0]);
	base32::encode(out_p, v2);
	BOOST_CHECK(0 == strcasecmp(
		reinterpret_cast<char const *>(out),
		reinterpret_cast<char const *>(in)
	));
}

BOOST_AUTO_TEST_CASE(base32_1)
{
	static std::random_device src;
	std::mt19937 gen(src());
	std::uniform_int_distribution<uint64_t> dis;
	constexpr static int count = 10000;
	std::array<uint8_t, 13> buf;
	unsigned char *p;
	buf[12] = 0;

	for (int c(0); c < count; ++c) {
		uint64_t d_r(dis(gen));
		p = &buf.front();
		base32::encode(p, d_r);
		uint64_t d_d(0);
		p = &buf.front();
		base32::decode(d_d, p);
		BOOST_CHECK_EQUAL(d_r, d_d);
	}
}
}}}
