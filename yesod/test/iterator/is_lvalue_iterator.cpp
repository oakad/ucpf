/*=============================================================================
    Copyright (c) 2003 David Abrahams
    Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#define BOOST_TEST_MODULE yesod_iterator
#include <boost/test/included/unit_test.hpp>

#include <yesod/iterator/is_lvalue_iterator.hpp>

#include "test_iterator.hpp"

namespace ucpf { namespace yesod { namespace iterator {
namespace test {

struct v {
	v();
	~v();
};

struct noncopyable {
	noncopyable(noncopyable const &) = delete;
	noncopyable &operator=(noncopyable const &) = delete;
	~noncopyable();
};

struct value_iterator : std::iterator<std::input_iterator_tag, v> {
	v operator*() const;
};

struct noncopyable_iterator : std::iterator<
	std::forward_iterator_tag, noncopyable
> {
    noncopyable const &operator*() const;
};

template <typename T>
struct proxy_iterator : std::iterator<std::output_iterator_tag, T> {
	typedef T value_type;

	struct proxy {
		operator value_type&() const;
		proxy &operator=(value_type) const;
	};

	proxy operator*() const;
};

template <typename T>
struct lvalue_iterator {
	typedef T value_type;
	typedef T &reference;
	typedef T difference_type;
	typedef std::input_iterator_tag iterator_category;
	typedef T *pointer;

	T &operator*() const;
	lvalue_iterator &operator++();
	lvalue_iterator operator++(int);
};

template <typename T>
struct constant_lvalue_iterator {
	typedef T value_type;
	typedef T const &reference;
	typedef T difference_type;
	typedef std::input_iterator_tag iterator_category;
	typedef T const *pointer;

	T const &operator*() const;
	constant_lvalue_iterator &operator++();
	constant_lvalue_iterator operator++(int);
};

}

BOOST_AUTO_TEST_CASE(is_lvalue_iterator_0)
{
	BOOST_CHECK((is_lvalue_iterator<test::v *>::value));
	BOOST_CHECK((is_lvalue_iterator<test::v const *>::value));
	BOOST_CHECK((is_lvalue_iterator<
		std::deque<test::v>::iterator
	>::value));
	BOOST_CHECK((is_lvalue_iterator<
		std::deque<test::v>::const_iterator
	>::value));
	BOOST_CHECK(!(is_lvalue_iterator<
		std::back_insert_iterator<std::deque<test::v>>
	>::value));
	BOOST_CHECK(!(is_lvalue_iterator<
		std::ostream_iterator<test::v>
	>::value));
	BOOST_CHECK(!(is_lvalue_iterator<
		test::proxy_iterator<test::v>
	>::value));
	BOOST_CHECK(!(is_lvalue_iterator<test::proxy_iterator<int>>::value));
	BOOST_CHECK(!(is_lvalue_iterator<test::value_iterator>::value));

	BOOST_CHECK((is_lvalue_iterator<test::noncopyable_iterator>::value));

	BOOST_CHECK((is_lvalue_iterator<
		test::lvalue_iterator<test::v>
	>::value));
	BOOST_CHECK((is_lvalue_iterator<test::lvalue_iterator<int>>::value));
	BOOST_CHECK((is_lvalue_iterator<
		test::lvalue_iterator<char *>
	>::value));
	BOOST_CHECK((is_lvalue_iterator<test::lvalue_iterator<float>>::value));

	
	BOOST_CHECK((is_lvalue_iterator<
		test::constant_lvalue_iterator<test::v>
	>::value));
	BOOST_CHECK((is_lvalue_iterator<
		test::constant_lvalue_iterator<int>
	>::value));
	BOOST_CHECK((is_lvalue_iterator<
		test::constant_lvalue_iterator<char *>
	>::value));
	BOOST_CHECK((is_lvalue_iterator<
		test::constant_lvalue_iterator<float>>::value
	));

	BOOST_CHECK((is_non_const_lvalue_iterator<test::v *>::value));
	BOOST_CHECK(!(is_non_const_lvalue_iterator<test::v const *>::value));
	BOOST_CHECK((is_non_const_lvalue_iterator<
		std::deque<test::v>::iterator
	>::value));
	BOOST_CHECK(!(is_non_const_lvalue_iterator<
		std::deque<test::v>::const_iterator
	>::value));
	BOOST_CHECK(!(is_non_const_lvalue_iterator<
		std::back_insert_iterator<std::deque<test::v>>>::value
	));
	BOOST_CHECK(!(is_non_const_lvalue_iterator<
		std::ostream_iterator<test::v>
	>::value));
	BOOST_CHECK(!(is_non_const_lvalue_iterator<
		test::proxy_iterator<test::v>
	>::value));
	BOOST_CHECK(!(is_non_const_lvalue_iterator<
		test::proxy_iterator<int>
	>::value));

	BOOST_CHECK(!(is_non_const_lvalue_iterator<
		test::value_iterator
	>::value));

	BOOST_CHECK(!(is_non_const_lvalue_iterator<
		test::noncopyable_iterator
	>::value));

	BOOST_CHECK((is_non_const_lvalue_iterator<
		test::lvalue_iterator<test::v>
	>::value));

	BOOST_CHECK((is_non_const_lvalue_iterator<
		test::lvalue_iterator<int>
	>::value));

	BOOST_CHECK((is_non_const_lvalue_iterator<
		test::lvalue_iterator<char *>
	>::value));
	BOOST_CHECK((is_non_const_lvalue_iterator<
		test::lvalue_iterator<float>
	>::value));

	BOOST_CHECK(!(is_non_const_lvalue_iterator<
		test::constant_lvalue_iterator<test::v>
	>::value));
	BOOST_CHECK(!(is_non_const_lvalue_iterator<
		test::constant_lvalue_iterator<int>
	>::value));
	BOOST_CHECK(!(is_non_const_lvalue_iterator<
		test::constant_lvalue_iterator<char *>
	>::value));
	BOOST_CHECK(!(is_non_const_lvalue_iterator<
		test::constant_lvalue_iterator<float>
	>::value));
}
}}}
