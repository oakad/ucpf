/*
 * Copyright (c) 2017 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#define BOOST_TEST_MODULE yesod
#include <boost/test/unit_test.hpp>

#include <yesod/paged_storage.hpp>
#include <bitset>
#include <vector>
#include <random>

namespace ucpf::yesod {
namespace test {

struct simple_value {
	simple_value()
	: flag(~size_t(0))
	{
		printf("-xx- constructed %p\n", this);
	}

	simple_value(size_t flag_)
	: flag(flag_)
	{}

	~simple_value()
	{
		printf("-xx- destroyed %p - %zd\n", this, flag);
	}

	size_t flag;
};

template <
	size_t MaxValue, size_t ValueCount, size_t ValueNodeOrder,
	size_t PtrNodeOrder
>
struct storage_tester {
	storage_tester()
	{
		static std::random_device src;
		std::mt19937 gen(src());
		std::uniform_int_distribution<size_t> dis;

		size_t count(0);
		while (count < ValueCount) {
			auto pos(dis(gen) % MaxValue);
			if (!ref_bits.test(pos)) {
				storage.put(storage.at(pos), simple_value(pos));
				ref_bits.set(pos);
				index.push_back(pos);
				++count;
			}
		}
		std::sort(index.begin(), index.end());
	}

	std::vector<size_t> index;
	std::bitset<MaxValue> ref_bits;
	paged_storage<simple_value, ValueNodeOrder, PtrNodeOrder> storage;
};

}

BOOST_AUTO_TEST_CASE(paged_storage_0)
{
	test::storage_tester<8, 4, 3, 2> st0;
}
}
