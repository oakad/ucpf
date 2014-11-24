/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#define BOOST_TEST_MODULE yesod
#include <boost/test/unit_test.hpp>

#include <yesod/shared_mutex.hpp>

namespace ucpf { namespace yesod {

BOOST_AUTO_TEST_CASE(shared_mutex_0)
{
	shared_mutex m;
	m.lock();
	m.unlock();
	m.lock_shared();
	m.unlock_shared();
}

}}
