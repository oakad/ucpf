/*
 * Copyright (c) 2016 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#define BOOST_TEST_MODULE holam
#include <boost/test/unit_test.hpp>

#include <holam/out.hpp>
#include "test.hpp"

namespace ucpf { namespace holam {

BOOST_AUTO_TEST_CASE(out_0)
{
	char const *s0 = "abcde{#}fghij {} klmn {} pif pif {}";
	char const *s1 = "abcde{}ghij 123 klmn 456 pif pif jaba";
	output::collector<> dest;
	BOOST_CHECK_EQUAL(
		std::strlen(s1), out(dest, s0, 123, 456, "jaba"
	).count());
	BOOST_CHECK_EQUAL(dest, s1);
}

}}
