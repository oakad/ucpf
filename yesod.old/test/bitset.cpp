/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#define BOOST_TEST_MODULE yesod
#include <boost/test/unit_test.hpp>

#include <bitset>
#include <unordered_set>
#include <yesod/bitset.hpp>

namespace ucpf { namespace yesod {
namespace test {

template <size_t N, size_t U>
void test_for_each()
{
	static std::random_device src;
	std::mt19937 gen(src());
	std::uniform_int_distribution<uint32_t> dis;

	constexpr static size_t set_size(N);

	std::bitset<set_size> ref;
	bitset<set_size> s;

	s.reset();
	size_t u_cnt(0);
	for (size_t u(0); u < U; ++u) {
		auto pos(dis(gen) % set_size);
		if (!ref.test(pos)) {
			ref.set(pos);
			s.set(pos);
			++u_cnt;
		}
	}

	size_t u(0);
	s.for_each_one(0, [&ref, &s, &u](size_t pos) -> bool {
		BOOST_CHECK(ref.test(pos));
		BOOST_CHECK(s.test(pos));
		if (ref.test(pos))
			++u;

		return false;
	});
	BOOST_CHECK_EQUAL(u, u_cnt);
}

template <size_t N, size_t U>
void test_find_first()
{
	static std::random_device src;
	std::mt19937 gen(src());
	std::uniform_int_distribution<uint32_t> dis;

	constexpr static size_t set_size(N);

	bitset<set_size> s;
	std::unordered_set<size_t> ref;

	s.reset();
	size_t u_cnt(0);
	for (size_t u(0); u < U; ++u) {
		auto pos(dis(gen) % set_size);
		if (!ref.count(pos)) {
			ref.insert(pos);
			s.set(pos);
			++u_cnt;
		}
	}

	size_t u(0);
	for (
		auto pos(s.find_first_one(0));
		pos < set_size;
		pos = s.find_first_one(pos + 1)
	) {
		BOOST_CHECK(ref.count(pos));
		if (ref.count(pos))
			++u;
	}

	BOOST_CHECK_EQUAL(u, u_cnt);
}

template <typename T, typename T::size_type (T::*f)(
	typename T::size_type, typename T::size_type
) const>
void test_find_zero_range()
{
	static std::random_device src;
	std::mt19937 gen(src());
	std::uniform_int_distribution<uint32_t> dis;

	T s;
	s.reset();
	auto rem(s.size());

	while (rem) {
		auto len(dis(gen) % (rem + 1));
		if (!len)
			continue;

		auto pos((s.*f)(0, len));
		BOOST_CHECK_LT(pos, s.size());
		s.set(pos, len);
		rem -= len;
	}

	BOOST_CHECK_EQUAL(s.size(), s.count());
}

}

BOOST_AUTO_TEST_CASE(bitset_0)
{
	BOOST_CHECK(std::is_pod<bitset<512>>::value);
	test::test_for_each<47, 13>();
	test::test_for_each<64, 17>();
	test::test_for_each<123, 63>();
	test::test_for_each<373, 111>();
	test::test_for_each<512, 200>();
}

BOOST_AUTO_TEST_CASE(bitset_1)
{
	test::test_find_first<47, 13>();
	test::test_find_first<64, 17>();
	test::test_find_first<123, 63>();
	test::test_find_first<373, 111>();
	test::test_find_first<512, 200>();
}

BOOST_AUTO_TEST_CASE(bitset_2)
{
	test::test_find_zero_range<
		bitset<47>, &bitset<47>::find_zero_range_first
	>();
	test::test_find_zero_range<
		bitset<64>, &bitset<64>::find_zero_range_first
	>();
	test::test_find_zero_range<
		bitset<123>, &bitset<123>::find_zero_range_first
	>();
	test::test_find_zero_range<
		bitset<373>, &bitset<373>::find_zero_range_first
	>();
	test::test_find_zero_range<
		bitset<512>, &bitset<512>::find_zero_range_first
	>();
}

BOOST_AUTO_TEST_CASE(bitset_3)
{
	test::test_find_zero_range<
		bitset<47>, &bitset<47>::find_zero_range_best
	>();
	test::test_find_zero_range<
		bitset<64>, &bitset<64>::find_zero_range_best
	>();
	test::test_find_zero_range<
		bitset<123>, &bitset<123>::find_zero_range_best
	>();
	test::test_find_zero_range<
		bitset<373>, &bitset<373>::find_zero_range_best
	>();
	test::test_find_zero_range<
		bitset<512>, &bitset<512>::find_zero_range_best
	>();
}

}}
