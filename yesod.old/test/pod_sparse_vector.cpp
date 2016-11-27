/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#define BOOST_TEST_MODULE yesod
#include <boost/test/unit_test.hpp>

#include <unordered_map>
#include <unordered_set>

#include <yesod/pod_sparse_vector.hpp>

namespace ucpf { namespace yesod {
namespace test {

struct pair_type {
	uintptr_t base;
	uintptr_t check;

	static pair_type make(uintptr_t base_, uintptr_t check_)
	{
		return pair_type{base_, check_};
	}
};

std::ostream &operator<<(std::ostream &os, pair_type p)
{
	os << "b: " << p.base << ", c: " << p.check;
	return os;
}

}

BOOST_AUTO_TEST_CASE(pod_sparse_vector_0)
{
	static std::random_device src;
	std::mt19937 gen(src());
	std::uniform_int_distribution<std::size_t> dis;
	constexpr static std::size_t max_value = 1000000;
	constexpr static std::size_t max_range = 10000;
	constexpr static std::size_t test_count = 1000;

	pod_sparse_vector<test::pair_type> v0;
	std::unordered_map<uintptr_t, std::size_t> m0;

	for (std::size_t c(0); c < test_count; ++c) {
		auto pos(dis(gen) % max_value);
		auto count(dis(gen) % max_range);
		if (!count)
			continue;

		v0.for_each_pos(
			pos, pos + count,
			[c, &m0](auto x_pos, auto &v) -> void {
				v = test::pair_type::make(x_pos, c);
				m0[x_pos] = c;
			}
		);

		decltype(pos) last_pos(pos);
		decltype(count) last_count(0);
		v0.for_each(
			pos + 1, [pos, count, c, &last_pos, &last_count](
				auto x_pos, auto const &v
			) -> bool {
				BOOST_CHECK_EQUAL(last_pos, x_pos - 1);
				BOOST_CHECK_EQUAL(x_pos, v.base);
				BOOST_CHECK_EQUAL(c, v.check);
				last_pos = x_pos;
				++last_count;
				return x_pos == (pos + count - 1);
			}
		);
		BOOST_CHECK_EQUAL(count, last_count + 1);
	}

	for (std::size_t c(0); c < 100 * test_count; ++c) {
		auto pos(dis(gen) % max_value);
		auto p(v0.ptr_at(pos));
		auto iter(m0.find(pos));
		if (iter != m0.end()) {
			BOOST_REQUIRE(p != nullptr);
			BOOST_CHECK_EQUAL(p->base, pos);
			BOOST_CHECK_EQUAL(p->check, iter->second);
		} else {
			BOOST_CHECK(
				(p == nullptr) || (p->base == 0)
			);
		}
	}
}

}}
