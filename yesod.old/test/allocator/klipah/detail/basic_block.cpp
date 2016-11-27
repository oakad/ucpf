/*
 * Copyright (c) 2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#define BOOST_TEST_MODULE yesod
#include <boost/test/unit_test.hpp>

#include <yesod/allocator/klipah/detail/basic_block.hpp>

#include <string>

namespace ucpf { namespace yesod { namespace allocator { namespace klipah {
namespace detail {
namespace test {

struct s0 {
	uintptr_t p0;
	uintptr_t p1;
};

struct [[gnu::packed]] alignas(8) s1
{
	uint8_t d0[5];
};

}

BOOST_AUTO_TEST_CASE(basic_block_0)
{
	basic_block<64> b0;
	BOOST_CHECK_EQUAL(
		b0.count<test::s0>(), b0.size() / sizeof(test::s0)
	);
	BOOST_CHECK_EQUAL(
		b0.count<test::s1>(), b0.size() / sizeof(test::s1)
	);

	b0.assign<uint8_t>(0, b0.size(), 'a');
	for (unsigned c = 0; c < b0.count<test::s1>(); ++c)
		b0.construct<test::s1>(
			c, 1, test::s1{{'v', 'w', 'x', 'y', 'z'}}
		);

	auto cptr(b0.get<char>(0, b0.size()));
	std::string ss(cptr.get(), b0.size());
	BOOST_CHECK_EQUAL(
		ss,
		"vwxyzaaavwxyzaaavwxyzaaavwxyzaaavwxyzaaavwxyzaaavwxyzaaavwxyzaaa"
	);
}

}
}}}}
