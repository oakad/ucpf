/*
 * Copyright (c) 2017 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#define BOOST_TEST_MODULE yesod_detail
#include <boost/test/unit_test.hpp>

#include <bitset>
#include <vector>
#include <random>
#include <yesod/detail/bitmap.hpp>

#include "../test.hpp"

namespace ucpf::yesod::detail {
namespace test {

template <size_t BitmapSize, size_t SetBitCount>
struct bitmap_tester {
	bitmap_tester()
	{
		static std::random_device src;
		std::mt19937 gen(src());
		std::uniform_int_distribution<size_t> dis;

		size_t count(0);
		while (count < SetBitCount) {
			auto pos(dis(gen) % BitmapSize);
			if (!ref_bits.test(pos)) {
				bitmap::set(data, pos);
				ref_bits.set(pos);
				bit_index.push_back(pos);
				++count;
			}
		}
		std::sort(bit_index.begin(), bit_index.end());
	}

	bitmap_tester &test_count()
	{
		BOOST_TEST(
			bitmap::count_ones(data, 0, BitmapSize) == SetBitCount
		);
		return *this;
	}

	bitmap_tester &test_nth_one(size_t step)
	{
		size_t next_idx(step - 1);
		size_t first = 0;
		do {
			auto pos(bitmap::find_nth_one(
				data, step, first, BitmapSize
			));
			BOOST_TEST((pos + first) == bit_index[next_idx]);
			first += pos + 1;
			next_idx += step;
		} while (next_idx < bit_index.size());
		return *this;
	}

	bitmap_tester &test_nth_one_r(size_t step)
	{
		size_t next_idx(bit_index.size() - step);
		size_t last(BitmapSize);
		while (true) {
			auto pos(bitmap::find_nth_one_r(
				data, step, 0, last
			));
			BOOST_TEST((last - pos) == bit_index[next_idx]);
			last -= pos;
			if (next_idx < step)
				return *this;
			next_idx -= step;
		}
	}

	std::vector<size_t> bit_index;
	std::bitset<BitmapSize> ref_bits;
	uint8_t data[bitmap::byte_count(BitmapSize)] = {0};
};

}

BOOST_AUTO_TEST_CASE(bitmap_0)
{
	test::bitmap_tester<47, 13> b0;
	test::bitmap_tester<64, 17> b1;
	test::bitmap_tester<123, 63> b2;
	test::bitmap_tester<373, 111> b3;
	test::bitmap_tester<512, 200> b4;

	b0.test_count();
	b1.test_count();
	b2.test_count();
	b3.test_count();
	b4.test_count();

	for (size_t step(1); step < 4; ++step) {
		b0.test_nth_one(step).test_nth_one_r(step);
		b1.test_nth_one(step).test_nth_one_r(step);
		b2.test_nth_one(step).test_nth_one_r(step);
		b3.test_nth_one(step).test_nth_one_r(step);
		b4.test_nth_one(step).test_nth_one_r(step);
	}

}
}
