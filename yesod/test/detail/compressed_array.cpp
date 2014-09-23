/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#define BOOST_TEST_MODULE yesod
#include <boost/test/included/unit_test.hpp>

#include <yesod/detail/compressed_array.hpp>

namespace ucpf { namespace yesod { namespace detail { namespace test {

BOOST_AUTO_TEST_CASE(compressed_array_0)
{
	constexpr static std::size_t base_order(11);
	static std::random_device src;
	std::mt19937 gen(src());
	std::uniform_int_distribution<uintptr_t> dis;

	std::array<uintptr_t, 1 << base_order> ref;
	for (auto &v: ref)
		v = dis(gen);

	{
		constexpr static std::size_t sub_order(1);
		constexpr static uintptr_t sub_order_mask(
			(uintptr_t(1) << sub_order) - 1
		);

		compressed_array<int, base_order, sub_order, void> ca;
		ca.init(std::allocator<void>());
		for (std::size_t c(0); c < ref.size(); ++c)
			ca.set_index(c, ref[c] & sub_order_mask);

		for (std::size_t c(0); c < ref.size(); ++c)
			BOOST_CHECK_EQUAL(
				ca.get_index(c), ref[c] & sub_order_mask
			);
	}
	{
		constexpr static std::size_t sub_order(3);
		constexpr static uintptr_t sub_order_mask(
			(uintptr_t(1) << sub_order) - 1
		);

		compressed_array<int, base_order, sub_order, void> ca;
		ca.init(std::allocator<void>());
		for (std::size_t c(0); c < ref.size(); ++c)
			ca.set_index(c, ref[c] & sub_order_mask);

		for (std::size_t c(0); c < ref.size(); ++c)
			BOOST_CHECK_EQUAL(
				ca.get_index(c), ref[c] & sub_order_mask
			);
	}
	{
		constexpr static std::size_t sub_order(7);
		constexpr static uintptr_t sub_order_mask(
			(uintptr_t(1) << sub_order) - 1
		);

		compressed_array<int, base_order, sub_order, void> ca;
		ca.init(std::allocator<void>());
		for (std::size_t c(0); c < ref.size(); ++c)
			ca.set_index(c, ref[c] & sub_order_mask);

		for (std::size_t c(0); c < ref.size(); ++c)
			BOOST_CHECK_EQUAL(
				ca.get_index(c), ref[c] & sub_order_mask
			);
	}
	{
		constexpr static std::size_t sub_order(10);
		constexpr static uintptr_t sub_order_mask(
			(uintptr_t(1) << sub_order) - 1
		);

		compressed_array<int, base_order, sub_order, void> ca;
		ca.init(std::allocator<void>());
		for (std::size_t c(0); c < ref.size(); ++c)
			ca.set_index(c, ref[c] & sub_order_mask);

		for (std::size_t c(0); c < ref.size(); ++c)
			BOOST_CHECK_EQUAL(
				ca.get_index(c), ref[c] & sub_order_mask
			);
	}
}

}}}}
