/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#define BOOST_TEST_MODULE yesod
#include <boost/test/included/unit_test.hpp>

#include <yesod/coder/xxhash.hpp>

namespace ucpf { namespace yesod { namespace coder {

BOOST_AUTO_TEST_CASE(xxhash_0)
{
	std::string s0("xgnnmqokr");
	std::string s1("wvuxuvbworkuyxsxmblnygo");
	std::string s2("trosycntqxpchtfbjfiltclcsibjahpptewafm");
	uint32_t seed(0xd385be64);

	xxhash<> h(seed);
	h.update(s0.begin(), s0.end());
	BOOST_CHECK_EQUAL(h.digest(), 0xc3548371);
	h.reset(seed);
	h.update(s1.begin(), s1.end());
	BOOST_CHECK_EQUAL(h.digest(), 0x5bb63324);
	h.reset(seed);
	h.update(s2.begin(), s2.end());
	BOOST_CHECK_EQUAL(h.digest(), 0x4b486c85);
	h.reset(seed);
	h.update(s0.begin(), s0.end());
	h.update(s1.begin(), s1.end());
	h.update(s2.begin(), s2.end());
	BOOST_CHECK_EQUAL(h.digest(), 0x053ca93e);
}

}}}
