/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#define BOOST_TEST_MODULE mina
#include <boost/test/included/unit_test.hpp>

#include <mina/np_packager.hpp>
#include <test/gdbm_store.hpp>

namespace ucpf { namespace mina { namespace test {

BOOST_AUTO_TEST_CASE(np_packager_0)
{
	gdbm_store gs("ref/np_packager/gdbm_pack.00.in");
}

}}}
