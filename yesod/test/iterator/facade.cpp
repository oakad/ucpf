/*=============================================================================
    Copyright (c) 2004 David Abrahams
    Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#define BOOST_TEST_MODULE yesod_iterator
#include <boost/test/included/unit_test.hpp>

#include <yesod/iterator/facade.hpp>

#include "test_iterator.hpp"

namespace ucpf { namespace yesod { namespace iterator {
namespace test {

template <typename Ref>
struct counter_iterator : facade<
	counter_iterator<Ref>, int const, single_pass_traversal_tag, Ref
> {
	counter_iterator()
	{}

	counter_iterator(int *state)
	: state(state)
	{}

	void increment()
	{
		++*state;
	}

	Ref dereference() const
	{
		return *state;
	}

	bool equal(counter_iterator const &y) const
	{
		return *this->state == *y.state;
	}

	int *state;
};

struct proxy {
	proxy(int &x)
	: state(x) {}

	operator int const&() const
	{
		return state;
	}

	int &operator=(int x)
	{
		state = x;
		return state;
	}

	int &state;
};

struct value
{
	void mutator()
	{}
};

struct input_iter : facade<
	input_iter, value, single_pass_traversal_tag, value
> {
	input_iter()
	{}

	void increment()
	{}

	value dereference() const
	{
		return value();
	}

	bool equal(input_iter const &y) const
	{
		return false;
	}
};

template <typename T>
struct wrapper {
	explicit wrapper(typename boost::call_traits<T>::param_type x)
	: m_x(x)
	{}

	template <class U>
	wrapper(
		wrapper<U> const &other,
		typename std::enable_if<
			std::is_convertible<U, T>::value
		>::type * = nullptr
	) : m_x(other.m_x)
	{}

	T m_x;
};

struct iterator_with_proxy_reference : facade<
	iterator_with_proxy_reference, wrapper<int>,
	incrementable_traversal_tag, wrapper<int &>
> {
	explicit iterator_with_proxy_reference(int &x)
	: m_x(x)
	{}

	void increment()
	{}

	wrapper<int &> dereference() const
	{
		return wrapper<int &>(m_x);
	}

	int &m_x;
};

}

BOOST_AUTO_TEST_CASE(facade_0)
{
	int state = 0;
	test::readable_iterator_test(
		test::counter_iterator<int const &>(&state), 0
	);

	state = 3;
	test::readable_iterator_test(
		test::counter_iterator<test::proxy>(&state), 3
	);
	test::writable_iterator_test(
		test::counter_iterator<test::proxy>(&state), 9, 7
	);
	BOOST_CHECK_EQUAL(state, 8);
}

BOOST_AUTO_TEST_CASE(facade_1)
{
	test::input_iter p;
	(*p).mutator();
	p->mutator();

	BOOST_CHECK((std::is_same<                                  \
		test::input_iter::pointer, decltype(p.operator->()) \
	>::value));
}

BOOST_AUTO_TEST_CASE(facade_2)
{
	int x = 0;
	test::iterator_with_proxy_reference i(x);
	BOOST_CHECK_EQUAL(x, 0);
	BOOST_CHECK_EQUAL(i.m_x, 0);

	++(*i).m_x;
	BOOST_CHECK_EQUAL(x, 1);
	BOOST_CHECK_EQUAL(i.m_x, 1);

	++i->m_x;
	BOOST_CHECK_EQUAL(x, 2);
	BOOST_CHECK_EQUAL(i.m_x, 2);

}

}}}
