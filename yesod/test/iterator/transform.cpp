/*=============================================================================
    Copyright (c) 2001-2002 Jeremy Siek
    Copyright (c) 2002      Thomas Witt
    Copyright (c) 2014      Alex Dubov <oakad@yahoo.com>

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#define BOOST_TEST_MODULE yesod_iterator
#include <boost/test/included/unit_test.hpp>

#include <yesod/iterator/transform.hpp>

#include "test_iterator.hpp"

namespace ucpf { namespace yesod { namespace iterator {
namespace test {

struct mult_functor {
	mult_functor()
	{}

	mult_functor(int aa)
	: a(aa)
	{}

	int operator()(int b) const
	{
		return a * b;
	}

	int a;
};

struct adaptable_mult_functor : mult_functor {
	typedef int result_type;
	typedef int argument_type;

	adaptable_mult_functor()
	{}

	adaptable_mult_functor(int aa)
	: mult_functor(aa)
	{}
};

struct const_select_first {
	typedef int const &result_type;

	int const &operator()(std::pair<int, int> const &p) const
	{
		return p.first;
	}
};

struct select_first : const_select_first {
	typedef int &result_type;

	int &operator()(std::pair<int, int> &p) const
	{
		return p.first;
	}
};

struct select_second {
	typedef int &result_type;

	int &operator()(std::pair<int, int> &p) const
	{
		return p.second;
	}
};

struct value_select_first {
	typedef int result_type;

	int operator()(std::pair<int, int> const &p) const
	{
		return p.first;
	}
};

int mult_2(int arg)
{
	return arg * 2;
}

struct polymorphic_mult_functor {
	template <typename F>
	struct result;

	template <typename F, typename T>
	struct result<const F(T)> {
		typedef T type;
	};

	template <typename F, typename T>
	struct result<const F(T &)> {
		typedef T type;
	};

	template <typename F, typename T>
	struct result<const F(T const &)> {
		typedef T type;
	};

	template <typename F, typename T>
	struct result<F(T)> {
		typedef void type;
	};

	template <typename F, typename T>
	struct result<F(T &)> {
		typedef void type;
	};

	template <typename F, typename T>
	struct result<F(T const &)> {
		typedef void type;
	};

	template <typename T> 
	T operator()(T const &arg) const
	{
		return arg * 2;
	}

	template <typename T> 
	void operator()(T const &arg)
	{
		BOOST_CHECK(false);
	}
};

}

BOOST_AUTO_TEST_CASE(transform_0)
{
	constexpr static std::size_t count = 10;

	int x[count], y[count];

	for (std::size_t c = 0; c < count; ++c)
		x[c] = c;

	std::copy(x, x + count, y);

	for (std::size_t c = 0; c < count; ++c)
		x[c] = x[c] * 2;

	typedef transform<test::adaptable_mult_functor, int *> iter_type;
	iter_type iter(y, test::adaptable_mult_functor(2));
	test::input_iterator_test(iter, x[0], x[1]);
	test::input_iterator_test(
		iter_type(&y[0], test::adaptable_mult_functor(2)), x[0], x[1]
	);
	test::random_access_readable_iterator_test(iter, count, x);
}

BOOST_AUTO_TEST_CASE(transform_1)
{
	constexpr static std::size_t count = 10;

	int x[count], y[count];

	for (std::size_t c = 0; c < count; ++c)
		x[c] = c;

	std::copy(x, x + count, y);

	for (std::size_t c = 0; c < count; ++c)
		x[c] = x[c] * 2;

	typedef transform<test::mult_functor, int *, int> iter_type;
	iter_type iter(y, test::mult_functor(2));
	test::input_iterator_test(iter, x[0], x[1]);
	test::input_iterator_test(
		iter_type(&y[0], test::mult_functor(2)), x[0], x[1]
	);
	test::random_access_readable_iterator_test(iter, count, x);
}

BOOST_AUTO_TEST_CASE(transform_2)
{
	{
		typedef transform<
			test::adaptable_mult_functor, int *, float
		> iter_type;
		BOOST_CHECK((
			std::is_same<iter_type::reference,  float>::value
		));
		BOOST_CHECK((
			std::is_same<iter_type::value_type, float>::value
		));
	}
	{
		typedef transform<
			test::adaptable_mult_functor, int *
		> iter_type;
		BOOST_CHECK((
			std::is_same<iter_type::reference,  int>::value
		));
		BOOST_CHECK((
			std::is_same<iter_type::value_type, int>::value
		));
	}
	{
		typedef transform<
			test::adaptable_mult_functor, int*, float, double
		> iter_type;
		BOOST_CHECK((
			std::is_same<iter_type::reference,  float>::value
		));
		BOOST_CHECK((
			std::is_same<iter_type::value_type, double>::value
		));
	}
}

BOOST_AUTO_TEST_CASE(transform_3)
{
	constexpr static std::size_t count = 10;

	int x[count], y[count];

	for (std::size_t c = 0; c < count; ++c)
		x[c] = c;

	std::copy(x, x + count, y);

	for (std::size_t c = 0; c < count; ++c)
		x[c] = x[c] * 2;

	test::input_iterator_test(
		make_transform(y, test::mult_2), x[0], x[1]
	);

	test::input_iterator_test(
		make_transform(&y[0], test::mult_2), x[0], x[1]
	);
 
	test::random_access_readable_iterator_test(
		make_transform(y, test::mult_2), count, x
	);
}

BOOST_AUTO_TEST_CASE(transform_4)
{
	constexpr static std::size_t count = 10;
	typedef std::pair<int, int> pair_type;

	int x[count], y[count];
	pair_type values[count];

	for(std::size_t c(0); c < count; ++c) {
		x[c] = c;
		y[c] = count - (c + 1);
	}

	std::copy(x, x + count, make_transform(values, test::select_first()));
	std::copy(y, y + count, make_transform(values, test::select_second()));

	test::random_access_readable_iterator_test(
		make_transform(values, test::value_select_first()), count, x
	);

	test::random_access_readable_iterator_test(
		make_transform(values, test::const_select_first()), count, x
	);
#if 0
	test::constant_lvalue_iterator_test(
		make_transform(values, test::const_select_first()), x[0]
	); 

	test::non_const_lvalue_iterator_test(
		make_transform(values, test::select_first()), x[0], 17
	);
#endif
	test::const_nonconst_iterator_test(
		++make_transform(values, test::select_first()),
		make_transform(values, test::const_select_first())
	);
}

BOOST_AUTO_TEST_CASE(transform_5)
{
	constexpr static std::size_t count = 10;

	int x[count], y[count];

	for (std::size_t c = 0; c < count; ++c)
		x[c] = c;

	std::copy(x, x + count, y);

	for (std::size_t c = 0; c < count; ++c)
		x[c] = x[c] * 2;

	test::input_iterator_test(make_transform(
		y, test::polymorphic_mult_functor()
	), x[0], x[1]);

	test::input_iterator_test(make_transform(
		&y[0], test::polymorphic_mult_functor()
	), x[0], x[1]);
 
	test::random_access_readable_iterator_test(make_transform(
		y, test::polymorphic_mult_functor()
	), count, x);
}

}}}
