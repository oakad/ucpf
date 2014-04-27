/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#define BOOST_TEST_MODULE yesod
#include <boost/test/included/unit_test.hpp>

#include <yesod/bitops.hpp>

namespace ucpf { namespace yesod {
namespace test {

template <typename T, T v>
struct static_order_base_2 {
	static constexpr int msb = fls(v);
	static constexpr int value = order_base_2(v);
};

template <typename T, T v>
constexpr int static_order_base_2<T, v>::msb;

template <typename T, T v>
constexpr int static_order_base_2<T, v>::value;

}

BOOST_AUTO_TEST_CASE(bitops_0)
{
	BOOST_CHECK_EQUAL((test::static_order_base_2<uint32_t, 1234>::msb), 10);
	BOOST_CHECK_EQUAL((                                           \
		test::static_order_base_2<uint32_t, 1234>::value), 11 \
	);
	BOOST_CHECK_EQUAL((test::static_order_base_2<uint32_t, 4096>::msb), 12);
	BOOST_CHECK_EQUAL((                                           \
		test::static_order_base_2<uint32_t, 4096>::value), 12 \
	);
}

}}
