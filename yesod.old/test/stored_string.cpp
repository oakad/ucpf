/*
 * Copyright (c) 2016 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#define BOOST_TEST_MODULE yesod
#include <boost/test/unit_test.hpp>

#include <yesod/stored_string.hpp>
#include <yesod/storage/memory.hpp>

namespace ucpf { namespace yesod {

BOOST_AUTO_TEST_CASE(stored_string_0)
{
	storage::memory m;
	stored_string<char, storage::memory> s0;
	std::string t0("abcdef"), t1("abcdefghijklmnopqrstuvwxyz");
	s0.assign(t0.cbegin(), t0.size(), m);
	s0.assign(t1.cbegin(), t1.size(), m);
	s0.release(m);
}

}}
