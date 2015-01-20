/*
 * Copyright (c) 2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#define BOOST_TEST_MODULE zivug
#include <boost/test/unit_test.hpp>

#include <zivug/detail/ipv6_addr_parse.hpp>

namespace ucpf { namespace zivug { namespace detail {

BOOST_AUTO_TEST_CASE(ipv6_addr_parse_0)
{
	in6_addr out;

	std::string a00("ABCD:EF01:2345:6789:ABCD:EF01:2345:6789");
	auto first(a00.begin());
	BOOST_CHECK(ipv6_addr_parse(out, first, a00.end()));

	std::string a10("2001:DB8:0:0:8:800:200C:417A");
	first = a10.begin();
	BOOST_CHECK(ipv6_addr_parse(out, first, a10.end()));

	std::string a11("2001:DB8::8:800:200C:417A");
	first = a11.begin();
	BOOST_CHECK(ipv6_addr_parse(out, first, a11.end()));

	std::string a20("FF01:0:0:0:0:0:0:101");
	first = a20.begin();
	BOOST_CHECK(ipv6_addr_parse(out, first, a20.end()));

	std::string a21("FF01::101");
	first = a21.begin();
	BOOST_CHECK(ipv6_addr_parse(out, first, a21.end()));

	std::string a30("0:0:0:0:0:0:0:1");
	first = a30.begin();
	BOOST_CHECK(ipv6_addr_parse(out, first, a30.end()));

	std::string a31("::1");
	first = a31.begin();
	BOOST_CHECK(ipv6_addr_parse(out, first, a31.end()));

	std::string a40("0:0:0:0:0:0:0:0");
	first = a40.begin();
	BOOST_CHECK(ipv6_addr_parse(out, first, a40.end()));

	std::string a41("::");
	first = a41.begin();
	BOOST_CHECK(ipv6_addr_parse(out, first, a41.end()));

	std::string a50("0:0:0:0:0:0:13.1.68.3");
	first = a50.begin();
	BOOST_CHECK(ipv6_addr_parse(out, first, a50.end()));

	std::string a51("::13.1.68.3");
	first = a51.begin();
	BOOST_CHECK(ipv6_addr_parse(out, first, a51.end()));

	std::string a60("0:0:0:0:0:FFFF:129.144.52.38");
	first = a60.begin();
	BOOST_CHECK(ipv6_addr_parse(out, first, a60.end()));

	std::string a61("::FFFF:129.144.52.38");
	first = a61.begin();
	BOOST_CHECK(ipv6_addr_parse(out, first, a61.end()));
}

}}}
