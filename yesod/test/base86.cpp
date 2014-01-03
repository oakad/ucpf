/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#define BOOST_TEST_MODULE yesod
#include <boost/test/included/unit_test.hpp>

#include <yesod/base86.hpp>

namespace ucpf { namespace yesod {

BOOST_AUTO_TEST_CASE(base85_0)
{
	char const *in("HelloWorld"), *in_p(in);
	uint32_t v1(0), v2(0);
	uint32_t v3(0x15c1ba2b), v4(0x6b716e3a);
	base86::decode(v1, in_p);
	base86::decode(v2, in_p);
	BOOST_CHECK_EQUAL(v1, v3);
	BOOST_CHECK_EQUAL(v2, v4);

	char out[11] = {0};
	char *out_p(&out[0]);
	base86::encode(out_p, v3);
	base86::encode(out_p, v4);
	BOOST_CHECK(0 == std::strcmp(out, in));
}

}}
