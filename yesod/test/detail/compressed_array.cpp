/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#define BOOST_TEST_MODULE yesod
#include <boost/test/included/unit_test.hpp>

#include <unordered_set>

#include <yesod/detail/compressed_array.hpp>

namespace ucpf { namespace yesod { namespace detail {
namespace test {

struct not_zero {
	template <typename T>
	static bool test(T v)
	{
		return v != 0;
	}
};

}

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
		constexpr static std::size_t sub_order(2);
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

BOOST_AUTO_TEST_CASE(compressed_array_1)
{
	constexpr static std::size_t base_order(7);
	static std::random_device src;
	std::mt19937 gen(src());
	std::uniform_int_distribution<std::size_t> dis;
	std::unordered_set<std::size_t> ref0;
	std::unordered_set<std::size_t> ref1;

	constexpr static std::size_t sub_order(6);
	constexpr static std::size_t set_count(40);
	compressed_array<int, base_order, sub_order, test::not_zero> ca;
	ca.init(std::allocator<void>());

	for (std::size_t c(1); c < (set_count + 1); ++c) {
		auto pos(dis(gen) & ((std::size_t(1) << base_order) - 1));
		if (ref0.emplace(pos).second)
			ca.emplace_at(std::allocator<void>(), pos, c);
	}

	for (
		std::size_t c(ca.find_vacant(0)); c < ca.size();
		c = ca.find_vacant(++c)
	)
		ref1.emplace(c);

	BOOST_CHECK_EQUAL(ref0.size() + ref1.size(), ca.size());
	for (std::size_t c(0); c < ca.size(); ++c)
		BOOST_CHECK(ref0.count(c) != ref1.count(c));
}

}}}
