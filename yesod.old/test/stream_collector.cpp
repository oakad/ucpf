/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#define BOOST_TEST_MODULE yesod
#include <boost/test/unit_test.hpp>

#include <yesod/stream_collector.hpp>

namespace ucpf { namespace yesod {

BOOST_AUTO_TEST_CASE(stream_collector_0)
{
	static std::random_device src;
	std::mt19937 gen(src());
	std::uniform_int_distribution<int> dis;
	constexpr static std::size_t seq_size(2000);
	constexpr static std::size_t block_size(16);

	std::vector<int> v0(seq_size);
	std::size_t v_pos(0);

	std::generate(
		v0.begin(), v0.end(), [&dis, &gen]() -> int {
			return dis(gen);
		}
	);

	auto prod = [&v0, &v_pos](int *buf, std::size_t count) -> std::size_t {
		auto rem(std::min(v0.size() - v_pos, count));
		std::copy_n(&v0[v_pos], rem, buf);
		v_pos += rem;
		return rem;
	};

	stream_collector<
		int, block_size, decltype(prod), std::allocator<void>
	> c0(prod, std::allocator<void>());

	auto first(c0.begin());

	BOOST_CHECK_EQUAL(std::distance(first, c0.end()), seq_size);
	BOOST_CHECK_EQUAL_COLLECTIONS(
		first, c0.end(), v0.begin(), v0.end()
	);
}

}}
