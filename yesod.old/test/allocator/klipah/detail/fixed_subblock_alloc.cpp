/*
 * Copyright (c) 2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#define BOOST_TEST_MODULE yesod
#include <boost/test/unit_test.hpp>

#include <yesod/allocator/klipah/policy/basic.hpp>
#include <yesod/allocator/klipah/detail/fixed_subblock_alloc.hpp>

namespace ucpf { namespace yesod { namespace allocator { namespace klipah {
namespace detail {

BOOST_AUTO_TEST_CASE(fixed_block_alloc_0)
{
	fixed_subblock_alloc<policy::basic, 16, 64> a0;
	BOOST_CHECK(std::is_standard_layout<decltype(a0)>::value);
}

}
}}}}
