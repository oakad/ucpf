/*
 * Copyright (c) 2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#define BOOST_TEST_MODULE yesod
#include <boost/test/unit_test.hpp>

#include <yesod/allocator/klipah/detail/list_utils.hpp>

#include <experimental/optional>

namespace ucpf { namespace yesod { namespace allocator { namespace klipah {
namespace detail {
namespace test {

struct slist_node {
	slist_node(long value_)
	: value(value_)
	{}

	slist_node *next;
	long value;
};

struct dlist_node {
	dlist_node(long value_)
	: value(value_)
	{}

	dlist_node *next;
	dlist_node *prev;
	long value;
};

struct slist_traits {
	constexpr static bool is_circular = false;

	slist_node *list_head(slist_node *p) const
	{
		p->next = nullptr;
		return p;
	}

	slist_node *next(slist_node *p) const
	{
		return p->next;
	}

	slist_node *link_after(slist_node *p, slist_node *q) const
	{
		q->next = p->next;
		p->next = q;
		return q;
	}

	bool precedes(slist_node *p, slist_node *q) const
	{
		return p->value <= q->value;
	}
};

struct cslist_traits {
	constexpr static bool is_circular = true;

	slist_node *list_head(slist_node *p) const
	{
		p->next = p;
		return p;
	}

	slist_node *next(slist_node *p) const
	{
		return p->next;
	}

	slist_node *link_after(slist_node *p, slist_node *q) const
	{
		q->next = p->next;
		p->next = q;
		return q;
	}

	bool precedes(slist_node *p, slist_node *q) const
	{
		return p->value <= q->value;
	}
};

struct cdlist_traits {
	constexpr static bool is_circular = true;

	dlist_node *list_head(dlist_node *p) const
	{
		p->next = p;
		p->prev = p;
		return p;
	}

	dlist_node *next(dlist_node *p) const
	{
		return p->next;
	}

	dlist_node *link_after(dlist_node *p, dlist_node *q) const
	{
		q->prev = p;
		q->next = p->next;
		p->next->prev = q;
		p->next = q;
		return q;
	}

	bool precedes(dlist_node *p, dlist_node *q) const
	{
		return p->value <= q->value;
	}
};

template <typename NodeType, typename ValueGen, typename Traits>
NodeType *make_list(ValueGen &&g, Traits const &t = Traits())
{
	auto ov(g());
	if (!ov)
		return nullptr;

	auto head(t.list_head(new NodeType(ov.value())));
	auto tail(head);
	ov = g();
	while (ov) {
		tail = t.link_after(tail, new NodeType(ov.value()));
		ov = g();
	}

	return head;
}

}

BOOST_AUTO_TEST_CASE(list_utils_sort_0)
{
	namespace f = std::experimental::fundamentals_v1;

	bool once(true);
	test::slist_traits tr;

	auto l0(test::make_list<test::slist_node>([&once]() {
		if (once) {
			once = false;
			return f::optional<long>(500);
		} else
			return f::optional<long>();
	}, tr));

	BOOST_CHECK_EQUAL(list_size(l0, tr), 1);
	BOOST_CHECK(list_is_sorted(l0, tr));
}

BOOST_AUTO_TEST_CASE(list_utils_sort_1)
{
	namespace f = std::experimental::fundamentals_v1;
	constexpr static std::size_t count = 100;
	constexpr static std::size_t max_size = 10000;
	static std::random_device src;
	std::mt19937 gen(src());
	std::uniform_int_distribution<long> dis;
	test::slist_traits tr;

	std::size_t iter_num(0);
	while (iter_num < count) {
		auto l_size(dis(gen) % max_size);
		if (l_size < 0)
			l_size = -l_size;

		if (!l_size)
			continue;

		std::size_t l_count(0);
		auto l0(test::make_list<test::slist_node>(
			[&l_count, &dis, &gen, l_size]() {
				if (l_count < l_size) {
					++l_count;
					return f::optional<long>(dis(gen));
				} else
					return f::optional<long>();
			}, tr
		));

		BOOST_CHECK_EQUAL(
			list_size(l0, tr), l_size
		);

		auto l1(list_sort(l0, tr));
		BOOST_CHECK_EQUAL(
			list_size(l1, tr), l_size
		);
		BOOST_CHECK(list_is_sorted(l1, tr));

		list_consume([](test::slist_node *p) {
			delete p;
		}, l1, tr);
		++iter_num;
	}
}

BOOST_AUTO_TEST_CASE(list_utils_sort_2)
{
	namespace f = std::experimental::fundamentals_v1;

	bool once(true);
	test::cslist_traits tr;

	auto l0(test::make_list<test::slist_node>([&once]() {
		if (once) {
			once = false;
			return f::optional<long>(500);
		} else
			return f::optional<long>();
	}, tr));

	BOOST_CHECK_EQUAL(list_size(l0, tr), 1);
	BOOST_CHECK(list_is_sorted(l0, tr));
}

BOOST_AUTO_TEST_CASE(list_utils_sort_3)
{
	namespace f = std::experimental::fundamentals_v1;
	constexpr static std::size_t count = 100;
	constexpr static std::size_t max_size = 10000;
	static std::random_device src;
	std::mt19937 gen(src());
	std::uniform_int_distribution<long> dis;
	test::cslist_traits tr;

	std::size_t iter_num(0);
	while (iter_num < count) {
		auto l_size(dis(gen) % max_size);
		if (l_size < 0)
			l_size = -l_size;

		if (!l_size)
			continue;

		std::size_t l_count(0);
		auto l0(test::make_list<test::slist_node>(
			[&l_count, &dis, &gen, l_size]() {
				if (l_count < l_size) {
					++l_count;
					return f::optional<long>(dis(gen));
				} else
					return f::optional<long>();
			}, tr
		));

		BOOST_CHECK_EQUAL(
			list_size(l0, tr), l_size
		);

		auto l1(list_sort(l0, tr));
		BOOST_CHECK_EQUAL(
			list_size(l1, tr), l_size
		);
		BOOST_CHECK(list_is_sorted(l1, tr));

		list_consume([](test::slist_node *p) {
			delete p;
		}, l1, tr);
		++iter_num;
	}
}

BOOST_AUTO_TEST_CASE(list_utils_sort_4)
{
	namespace f = std::experimental::fundamentals_v1;

	bool once(true);
	test::cdlist_traits tr;

	auto l0(test::make_list<test::dlist_node>([&once]() {
		if (once) {
			once = false;
			return f::optional<long>(500);
		} else
			return f::optional<long>();
	}, tr));

	BOOST_CHECK_EQUAL(list_size(l0, tr), 1);
	BOOST_CHECK(list_is_sorted(l0, tr));
}

BOOST_AUTO_TEST_CASE(list_utils_sort_5)
{
	namespace f = std::experimental::fundamentals_v1;
	constexpr static std::size_t count = 100;
	constexpr static std::size_t max_size = 10000;
	static std::random_device src;
	std::mt19937 gen(src());
	std::uniform_int_distribution<long> dis;
	test::cdlist_traits tr;

	std::size_t iter_num(0);
	while (iter_num < count) {
		auto l_size(dis(gen) % max_size);
		if (l_size < 0)
			l_size = -l_size;

		if (!l_size)
			continue;

		std::size_t l_count(0);
		auto l0(test::make_list<test::dlist_node>(
			[&l_count, &dis, &gen, l_size]() {
				if (l_count < l_size) {
					++l_count;
					return f::optional<long>(dis(gen));
				} else
					return f::optional<long>();
			}, tr
		));

		BOOST_CHECK_EQUAL(
			list_size(l0, tr), l_size
		);

		auto l1(list_sort(l0, tr));
		BOOST_CHECK_EQUAL(
			list_size(l1, tr), l_size
		);
		BOOST_CHECK(list_is_sorted(l1, tr));

		list_consume([](test::dlist_node *p) {
			delete p;
		}, l1, tr);
		++iter_num;
	}
}

}
}}}}
