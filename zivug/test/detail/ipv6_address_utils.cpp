/*
 * Copyright (c) 2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#define BOOST_TEST_MODULE zivug
#include <boost/test/unit_test.hpp>

#include <zivug/detail/ipv6_address_utils.hpp>

extern "C" {

#include <arpa/inet.h>

}

bool operator==(in6_addr const &l, in6_addr const &r)
{
	return !__builtin_memcmp(l.s6_addr, r.s6_addr, 16);
}

std::ostream &operator<<(std::ostream &os, in6_addr const &a)
{
	char out[4] = {':', 0, 0, 0};
	out[1] = 0x30 | (a.s6_addr[0] >> 4);
	out[2] = 0x30 | (a.s6_addr[0] & 0xf);
	if (out[1] > 0x39)
		out[1] += 7;
	if (out[2] > 0x39)
		out[2] += 7;

	os << out + 1;

	for (int c(1); c < 16; ++c) {
		out[1] = 0x30 | (a.s6_addr[c] >> 4);
		out[2] = 0x30 | (a.s6_addr[c] & 0xf);

		if (out[1] > 0x39)
			out[1] += 7;

		if (out[2] > 0x39)
			out[2] += 7;

		os << out;
	}

	return os;
}

namespace ucpf { namespace zivug { namespace detail {

BOOST_AUTO_TEST_CASE(ipv6_addr_parse_0)
{
	std::array<std::string, 14> addr_in = {{
		"ABCD:EF01:2345:6789:ABCD:EF01:2345:6789",
		"2001:DB8:0:0:8:800:200C:417A",
		"2001:DB8::8:800:200C:417A",
		"FF01:0:0:0:0:0:0:101",
		"FF01::101",
		"0:0:0:0:0:0:0:1",
		"::1",
		"0:0:0:0:0:0:0:0",
		"::",
		"0:0:0:0:0:0:13.1.68.3",
		"::13.1.68.3",
		"0:0:0:0:0:FFFF:129.144.52.38",
		"::FFFF:129.144.52.38",
		"ABCD::"
	}};

	for (auto &a_in: addr_in) {
		in6_addr out;
		in6_addr ref;

		BOOST_CHECK(ipv6_ascii_to_in6_addr(out, a_in.begin(), a_in.end()));
		inet_pton(AF_INET6, a_in.c_str(), &ref);
		BOOST_CHECK_EQUAL(out, ref);
	}

	for (auto &a_in: addr_in) {
		in6_addr ref_0, ref_1;
		inet_pton(AF_INET6, a_in.c_str(), &ref_0);
		std::string s_ref;
		ipv6_in6_addr_to_ascii(std::back_inserter(s_ref), ref_0);
		inet_pton(AF_INET6, s_ref.c_str(), &ref_1);
		BOOST_CHECK_EQUAL(ref_0, ref_1);
	}
}

}}}
