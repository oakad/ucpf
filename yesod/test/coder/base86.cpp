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

namespace std {

std::ostream &operator<<(std::ostream &os, std::array<uint8_t, 4> const &v)
{
	os << std::hex;
	os << '[' << static_cast<int>(v[0]);
	os << ':' << static_cast<int>(v[1]);
	os << ':' << static_cast<int>(v[2]);
	os << ':' << static_cast<int>(v[3]) << ']';
	return os;
}

}
namespace ucpf { namespace yesod { namespace coder {

BOOST_AUTO_TEST_CASE(base86_0)
{
	auto *in(reinterpret_cast<uint8_t const *>(
		"HelloWorld"
	)), *in_p(in);
	base86::value_type v1, v2;
	base86::value_type v3{0x2b, 0xba, 0xc1, 0x15};
	base86::value_type v4{0x3a, 0x6e, 0x71, 0x6b};
	base86::decode(v1, in_p);
	base86::decode(v2, in_p);
	BOOST_CHECK_EQUAL(v1, v3);
	BOOST_CHECK_EQUAL(v2, v4);

	uint8_t out[11] = {0};
	uint8_t *out_p(&out[0]);
	base86::encode(out_p, v3);
	base86::encode(out_p, v4);
	BOOST_CHECK(0 == std::strcmp(
		reinterpret_cast<char const *>(out),
		reinterpret_cast<char const *>(in)
	));
}

BOOST_AUTO_TEST_CASE(base86_1)
{
	static std::random_device src;
	std::mt19937 gen(src());
	std::uniform_int_distribution<uint32_t> dis;
	constexpr static int count = 10000;
	std::array<uint8_t, 6> buf;
	uint8_t *p;
	buf[5] = 0;

	for (int c(0); c < count; ++c) {
		base86::value_type d_r;
		*reinterpret_cast<uint32_t *>(d_r.data()) = dis(gen);
		p = &buf.front();
		base86::encode(p, d_r);
		base86::value_type d_d;
		p = &buf.front();
		base86::decode(d_d, p);
		BOOST_CHECK_EQUAL(d_r, d_d);
	}
}

}}}
