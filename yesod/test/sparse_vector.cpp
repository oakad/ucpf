/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE yesod
#include <boost/test/unit_test.hpp>

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

	s(s &&other) noexcept
	: ss(other.ss), value(other.value)
	{
		other.ss = nullptr;
	}

	s(s const &other)
	: ss(nullptr), value(other.value)
	{
	}

	~s()
	{
		if (ss)
			ss->erase(value);
	}

	s &operator=(s &&other) noexcept
	{
		ss = other.ss;
		value = other.value;
		other.ss = nullptr;
		return *this;
	}

	bool operator==(s const &other) const
	{
		return value == other.value;
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
	constexpr static std::size_t max_value = 1000000;
	constexpr static std::size_t count = 900000;

	std::unordered_set<std::size_t> s0;
	std::unordered_set<std::size_t> s1;
	std::unordered_set<std::size_t> s2;

	{
		std::unordered_map<std::size_t, test::s<decltype(s0)>> m0;
		std::unordered_map<std::size_t, test::s<decltype(s0)>> m1;
		sparse_vector<test::s<decltype(s1)>> v0;

		for (std::size_t c(0); c < count; ++c) {
			auto pos(dis(gen) % max_value);
			if (m0.emplace(
				std::piecewise_construct,
				std::forward_as_tuple(pos),
				std::forward_as_tuple(c, &s0)
			).second)
				v0.emplace(pos, c, &s1);
		}
		//v0.dump(std::cout);
		BOOST_CHECK_EQUAL_COLLECTIONS(
			s0.begin(), s0.end(), s1.begin(), s1.end()
		);
		

		for (auto const &p: m0)
			BOOST_CHECK_EQUAL(p.second.value, v0[p.first].value);

		auto not_all(v0.for_each(
			0, [&m1](auto pos, auto &v) -> bool {
				m1.insert(std::make_pair(pos, v));
				return false;
			}
		));
		BOOST_CHECK(!not_all);
		BOOST_CHECK(m0 == m1);
	}
	BOOST_CHECK(s1.empty());
}

BOOST_AUTO_TEST_CASE(sparse_vector_1)
{
	static std::random_device src;
	std::mt19937 gen(src());
	std::uniform_int_distribution<std::size_t> dis;
	constexpr static std::size_t max_value = 1000000;
	constexpr static std::size_t count = 999000;
	constexpr static std::size_t test_count = 10000;

	std::unordered_set<std::size_t> s0;
	sparse_vector<test::s<decltype(s0)>> v0;

	for (std::size_t c(0); c < count; ++c) {
		auto pos(dis(gen) % max_value);
		v0.emplace(pos, c, &s0);
	}

	for (std::size_t c(0); c < test_count; ++c) {
		auto pos(dis(gen) % max_value);
		auto v_pos(v0.find_vacant(pos));

		BOOST_CHECK_GE(v_pos, pos);
		BOOST_CHECK_EQUAL(
			v0.ptr_at(v_pos),
			typename decltype(v0)::pointer(nullptr)
		);

		for (; pos < v_pos; ++pos)
			BOOST_CHECK_NE(
				v0.ptr_at(pos),
				typename decltype(v0)::pointer(nullptr)
			);
	}
}

BOOST_AUTO_TEST_CASE(sparse_vector_2)
{
	static std::random_device src;
	std::mt19937 gen(src());
	std::uniform_int_distribution<std::size_t> dis;
	constexpr static std::size_t max_value = 1000000;
	constexpr static std::size_t max_range = 10000;
	constexpr static std::size_t test_count = 1000;

	std::unordered_set<std::size_t> s0;
	typedef test::s<decltype(s0)> s_type;
	sparse_vector<s_type> v0;

	for (std::size_t c(0); c < test_count; ++c) {
		auto pos(dis(gen) % max_value);
		auto count(dis(gen) % max_range);
		if (!count)
			continue;

		v0.for_each_pos(
			pos, pos + count,
			[&s0, &v0](auto x_pos, auto &v) -> void {
				v = std::move(s_type(x_pos, &s0));
			}
		);

		decltype(pos) last_pos(pos);
		decltype(count) last_count(0);
		v0.for_each(
			pos + 1, [pos, count, &last_pos, &last_count](
				auto x_pos, auto const &v
			) -> bool {
				BOOST_CHECK_EQUAL(last_pos, x_pos - 1);
				BOOST_CHECK_EQUAL(x_pos, v.value);
				last_pos = x_pos;
				++last_count;
				return x_pos == (pos + count - 1);
			}
		);
		BOOST_CHECK_EQUAL(count, last_count + 1);
	}
}

}}
