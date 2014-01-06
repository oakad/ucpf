/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#define BOOST_TEST_MODULE yesod
#include <boost/test/included/unit_test.hpp>

#include <yesod/sparse_vector.hpp>

namespace ucpf { namespace yesod {
namespace test {

struct s {
	static int global_count;
	int count0, count1;

	s()
	: count0(global_count), count1(global_count * 5)
	{
		++global_count;
	}
};

int s::global_count = 0;

struct char_vector_policy : default_sparse_vector_policy<char> {
	constexpr static size_t data_node_order = 8;
};

}

BOOST_AUTO_TEST_CASE(sparse_vector_0)
{
	sparse_vector<test::s> v0;
	sparse_vector<char, test::char_vector_policy> v1;
}
}}
