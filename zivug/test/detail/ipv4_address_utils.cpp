/*
 * Copyright (c) 2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#define BOOST_TEST_MODULE zivug
#include <boost/test/unit_test.hpp>

#include <zivug/detail/ipv4_address_utils.hpp>

namespace ucpf { namespace zivug { namespace detail {

BOOST_AUTO_TEST_CASE(ipv4_address_utils_0)
{
	std::array<std::string, 6> addr_in = {{
		"192.0.2.235",
		"0xC0.0x00.0x02.0xEB",
		"0300.0000.0002.0353",
		"0xC00002EB",
		"3221226219",
		"030000001353"
	}};

	::in_addr ref{htonl(0xc00002eb)};
	for (auto &a_in: addr_in) {
		::in_addr out;
		BOOST_CHECK(ipv4_ascii_to_in_addr(out, a_in.begin(), a_in.end()));
		BOOST_CHECK_EQUAL(ref.s_addr, out.s_addr);
	}

	std::string s_ref;
	ipv4_in_addr_to_ascii(std::back_inserter(s_ref), ref);
	BOOST_CHECK_EQUAL(addr_in[0], s_ref);
}

}}}
