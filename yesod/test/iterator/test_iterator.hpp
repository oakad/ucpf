/*=============================================================================
    Copyright (c) 2001      David Abrahams
    Copyright (c) 2001-2003 Jeremy Siek
    Copyright (c) 2001      Thomas Witt
    Copyright (c) 2013-2017 Alex Dubov <oakad@yahoo.com>

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#if !defined(HPP_EDD055D4183E4E8E3064A51424485CF4)
#define HPP_EDD055D4183E4E8E3064A51424485CF4

#include <yesod/iterator/is_lvalue_iterator.hpp>

namespace ucpf::yesod::iterator::test {
namespace is_incrementable_ns {

struct tag {};

struct any {
	template <typename T>
	any(T const &);
};

tag operator++(any const &);
tag operator++(any const &,int);
tag operator,(tag, int);

template <typename T>
std::true_type check_(T const &);

std::false_type check_(tag);

template <typename T>
struct impl {
	static typename std::remove_cv<T>::type &x;
	static constexpr bool value = decltype(check_((++x, 0)))::value;
};

template <typename T>
struct postfix_impl {
	static typename std::remove_cv<T>::type &x;
	static constexpr bool value = decltype(check_((x++, 0)))::value;
};
}

template <typename T>
using is_incrementable = std::integral_constant<
	bool, is_incrementable_ns::impl<T>::value
>;

template <typename T>
using is_postfix_incrementable = std::integral_constant<
	bool, is_incrementable_ns::postfix_impl<T>::value
>;

template <typename Iterator, typename T>
void input_iterator_test(Iterator iter0, T v0, T v1)
{
	Iterator iter1(iter0);

	BOOST_TEST(iter0 == iter1);
	BOOST_TEST(!(iter0 != iter1));

	BOOST_TEST(*iter1 == v0);
	BOOST_TEST(*iter0 == v0);

	BOOST_TEST(*iter0++ == v0);

	iter1 = iter0;

	BOOST_TEST(iter0 == iter1);
	BOOST_TEST(!(iter0 != iter1));

	BOOST_TEST(*iter1 == v1);
	BOOST_TEST(*iter0 == v1);

	++iter0;
}

template <typename Iterator, typename ConstIterator>
void const_nonconst_iterator_test(Iterator iter0, ConstIterator iter1)
{
	BOOST_TEST(iter0 != iter1);
	BOOST_TEST(iter1 != iter0);

	ConstIterator iter2(iter0);
	BOOST_TEST(iter2 == iter0);
	BOOST_TEST(iter0 == iter2);

	iter2 = iter0;
	BOOST_TEST(iter2 == iter0);
	BOOST_TEST(iter0 == iter2);
}

template <typename Iterator, typename T>
void readable_iterator_traversal_test(Iterator iter, T v, std::true_type)
{
	T v2(*iter++);
	BOOST_TEST(v == v2);
}

template <typename Iterator, typename T>
void readable_iterator_traversal_test(Iterator const iter, T v, std::false_type)
{}

template <typename Iterator, typename T>
void writable_iterator_traversal_test(Iterator iter, T v, std::true_type)
{
	++iter;
	*iter++ = v;
	Iterator x(iter++);
	(void)x;
}

template <typename Iterator, typename T>
void writable_iterator_traversal_test(Iterator const iter, T v, std::false_type)
{}

template <typename Iterator, typename T>
void readable_iterator_test(Iterator const iter, T v)
{
	Iterator other(iter);
	typedef typename std::iterator_traits<Iterator>::reference ref_t;
	ref_t r1 = *iter;
	ref_t r2 = *other;
	T v1 = r1;
	T v2 = r2;
	BOOST_TEST(v1 == v);
	BOOST_TEST(v2 == v);
	readable_iterator_traversal_test(
		iter, v, is_postfix_incrementable<Iterator>()
	);
}

template <typename Iterator, typename T>
void writable_iterator_test(Iterator iter, T v0, T v1)
{
	Iterator other(iter);
	*other = v0;

	writable_iterator_traversal_test(iter, v1, std::integral_constant<
		bool,
		is_incrementable<Iterator>::value
		&& is_postfix_incrementable<Iterator>::value
	>());
}

template <typename Iterator>
void swappable_iterator_test(Iterator iter0, Iterator iter1)
{
	Iterator other0(iter0), other1(iter1);
	typename std::iterator_traits<
		Iterator
	>::value_type b0 = *iter0, b1 = *iter1;

	iter_swap(other0, other1);
	typename std::iterator_traits<
		Iterator
	>::value_type a0 = *iter0, a1 = *iter1;
	BOOST_TEST(b0 == a1);
	BOOST_TEST(b1 == a0);
}

template <typename Iterator, typename T>
void constant_lvalue_iterator_test(Iterator iter, T v)
{
	Iterator other(iter);
	typedef typename std::iterator_traits<
		Iterator
	>::value_type value_type;
	typedef typename std::iterator_traits<
		Iterator
	>::reference reference;
	BOOST_TEST((std::is_same<value_type const &, reference>::value));
	T const &v1 = *other;
	BOOST_TEST(v == v1);

	BOOST_TEST(is_lvalue_iterator<Iterator>::value);
	BOOST_TEST(!is_non_const_lvalue_iterator<Iterator>::value);
}

template <typename Iterator, typename T>
void non_const_lvalue_iterator_test(Iterator iter, T v0, T v1)
{
	Iterator other(iter);
	typedef typename std::iterator_traits<
		Iterator
	>::value_type value_type;
	typedef typename std::iterator_traits<
		Iterator
	>::reference reference;
	BOOST_TEST((std::is_same<value_type &, reference>::value));
	T &v2 = *other;
	BOOST_TEST(v0 == v2);
	*iter = v1;
	T &v3 = *iter;
	BOOST_TEST(v1 == v3);
	BOOST_TEST(is_lvalue_iterator<Iterator>::value);
	BOOST_TEST(is_non_const_lvalue_iterator<Iterator>::value);
}

template <typename Iterator, typename T>
void forward_readable_iterator_test(Iterator i, Iterator j, T val1, T val2)
{
	Iterator i2;
	Iterator i3(i);
	i2 = i;
	BOOST_TEST(i2 == i3);
	BOOST_TEST(i != j);
	BOOST_TEST(i2 != j);
	readable_iterator_test(i, val1);
	readable_iterator_test(i2, val1);
	readable_iterator_test(i3, val1);

	BOOST_TEST(i == i2++);
	BOOST_TEST(i != ++i3);

	readable_iterator_test(i2, val2);
	readable_iterator_test(i3, val2);
	readable_iterator_test(i, val1);
}

template <typename Iterator, typename T>
void forward_swappable_iterator_test(Iterator i, Iterator j, T val1, T val2)
{
	forward_readable_iterator_test(i, j, val1, val2);
	Iterator i2 = i;
	++i2;
	swappable_iterator_test(i, i2);
}

template <typename Iterator, typename T>
void bidirectional_readable_iterator_test(Iterator i, T v1, T v2)
{
	Iterator j(i);
	++j;
	forward_readable_iterator_test(i, j, v1, v2);
	++i;

	Iterator iter = i, i2 = i;

	BOOST_TEST(i == iter--);
	BOOST_TEST(i != --i2);

	readable_iterator_test(i, v2);
	readable_iterator_test(iter, v1);
	readable_iterator_test(i2, v1);

	--i;
	BOOST_TEST(i == iter);
	BOOST_TEST(i == i2);
	++iter;
	++i2;

	readable_iterator_test(i, v1);
	readable_iterator_test(iter, v2);
	readable_iterator_test(i2, v2);
}

template <typename Iterator, typename TrueVals>
void random_access_readable_iterator_test(Iterator i, int N, TrueVals vals)
{
	bidirectional_readable_iterator_test(i, vals[0], vals[1]);
	Iterator const j = i;

	for (auto c(0); c < N-1; ++c) {
		BOOST_TEST(i == (j + c));
		BOOST_TEST(*i == vals[c]);
		typename std::iterator_traits<
			Iterator
		>::value_type x = j[c];
		BOOST_TEST(*i == x);
		BOOST_TEST(*i == *(j + c));
		BOOST_TEST(*i == *(c + j));
		++i;
		BOOST_TEST(i > j);
		BOOST_TEST(i >= j);
		BOOST_TEST(j <= i);
		BOOST_TEST(j < i);
	}

	Iterator k = j + N - 1;
	for (auto c(0); c < N-1; ++c) {
		BOOST_TEST(i == k - c);
		BOOST_TEST(*i == vals[N - 1 - c]);
		typename std::iterator_traits<
			Iterator
		>::value_type x = j[N - 1 - c];
		BOOST_TEST(*i == x);
		Iterator q = k - c; 
		BOOST_TEST(*i == *q);
		BOOST_TEST(i > j);
		BOOST_TEST(i >= j);
		BOOST_TEST(j <= i);
		BOOST_TEST(j < i);
		--i;
	}
}

}
#endif
