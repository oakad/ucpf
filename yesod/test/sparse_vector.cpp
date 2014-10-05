/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#define BOOST_TEST_MODULE yesod
#include <boost/test/included/unit_test.hpp>

#include <unordered_map>
#include <unordered_set>

#include <yesod/sparse_vector.hpp>

namespace ucpf { namespace yesod {
namespace test {

template <typename Set>
struct s {
	s()
	: ss(nullptr), value(0)
	{}

	s(std::size_t value_, Set *ss_)
	: ss(ss_), value(value_)
	{
		ss->emplace(value);
	}

	s(s &&other)
	: ss(other.ss), value(other.value)
	{
		other.ss = nullptr;
	}

	~s()
	{
		if (ss)
			ss->erase(value);
	}

	Set *ss;
	std::size_t value;
};

template <typename Set>
std::ostream &operator<<(std::ostream &os, s<Set> const &v)
{
	os << v.value << " (" << v.ss << ')';
	return os;
}

}

BOOST_AUTO_TEST_CASE(sparse_vector_0)
{
	static std::random_device src;
	std::mt19937 gen(src());
	std::uniform_int_distribution<std::size_t> dis;
	constexpr static std::size_t max_value = 1000;
	constexpr static std::size_t count = 10;

	std::unordered_set<std::size_t> s0;
	std::unordered_set<std::size_t> s1;

	{
		std::unordered_map<std::size_t, test::s<decltype(s0)>> m0;
		sparse_vector<test::s<decltype(s1)>> v0;

		for (std::size_t c(0); c < count; ++c) {
			auto pos(dis(gen) % max_value);
			printf("c %zd pos %zd\n", c, pos);
			if (m0.emplace(
				std::piecewise_construct,
				std::forward_as_tuple(pos),
				std::forward_as_tuple(c, &s0)
			).second)
				v0.emplace_at(pos, c, &s1);
		}

		v0.dump(std::cout);
		BOOST_CHECK(s0 == s1);

		for (auto const &p: m0) {
			printf("xx %zd - %zd\n", p.first, p.second.value);
			BOOST_CHECK_EQUAL(p.second.value, v0[p.first].value);
		}
	}
	BOOST_CHECK(s1.empty());
}

}}
