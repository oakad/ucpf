/*
 * Copyright (c) 2017 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#define BOOST_TEST_MODULE yesod
#include <boost/test/unit_test.hpp>

#include <yesod/concurrent/thread.hpp>

namespace ucpf::yesod::concurrent {
namespace test {

void runner(char const *a, int b)
{
	printf("xxx %s %d\n", a, b);
}

}

BOOST_AUTO_TEST_CASE(thread_0)
{
	thread t0(test::runner, "xuxux", 6);
	thread t1(
		std::allocator_arg, std::allocator<void>(),
		test::runner, "xuxux", 8
	);
	t1.join();
	t0.join();
	BOOST_TEST(false);
}

}
