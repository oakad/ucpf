/*=============================================================================
    Copyright (c) 2001      David Abrahams
    Copyright (c) 2001-2003 Jeremy Siek
    Copyright (c) 2001      Thomas Witt
    Copyright (c) 2013      Alex Dubov <oakad@yahoo.com>

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#if !defined(UCPF_YESOD_ITERATOR_TEST_ITERATOR_DEC_20_2013_2300)
#define UCPF_YESOD_ITERATOR_TEST_ITERATOR_DEC_20_2013_2300

#include <yesod/iterator/is_lvalue_iterator.hpp>

namespace ucpf { namespace yesod { namespace iterator { namespace test {
namespace is_incrementable_ {

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
	bool, is_incrementable_::impl<T>::value
>;

template <typename T>
using is_postfix_incrementable = std::integral_constant<
	bool, is_incrementable_::postfix_impl<T>::value
>;

template <typename Iterator, typename T>
void input_iterator_test(Iterator iter0, T v0, T v1)
{
	Iterator iter1(iter0);

	BOOST_CHECK(iter0 == iter1);
	BOOST_CHECK(!(iter0 != iter1));

	BOOST_CHECK(*iter1 == v0);
	BOOST_CHECK(*iter0 == v0);

	BOOST_CHECK(*iter0++ == v0);

	iter1 = iter0;

	BOOST_CHECK(iter0 == iter1);
	BOOST_CHECK(!(iter0 != iter1));

	BOOST_CHECK(*iter1 == v1);
	BOOST_CHECK(*iter0 == v1);

	++iter0;
}

template <typename Iterator, typename ConstIterator>
void const_nonconst_iterator_test(Iterator iter0, ConstIterator iter1)
{
	BOOST_CHECK(iter0 != iter1);
	BOOST_CHECK(iter1 != iter0);

	ConstIterator iter2(iter0);
	BOOST_CHECK(iter2 == iter0);
	BOOST_CHECK(iter0 == iter2);

	iter2 = iter0;
	BOOST_CHECK(iter2 == iter0);
	BOOST_CHECK(iter0 == iter2);
}

template <typename Iterator, typename T>
void readable_iterator_traversal_test(Iterator iter, T v, std::true_type)
{
	T v2(*iter++);
	BOOST_CHECK_EQUAL(v, v2);
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
	BOOST_CHECK(v1 == v);
	BOOST_CHECK(v2 == v);
	readable_iterator_traversal_test(
		iter, v, is_postfix_incrementable<Iterator>()
	);
}

template <typename Iterator, typename T>
void writable_iterator_test(Iterator iter, T v0, T v1)
{
	Iterator other(iter);
	*other = v0;

	writable_iterator_traversal_test(iter, v1, mpl::and_<
		is_incrementable<Iterator>,
		is_postfix_incrementable<Iterator>
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
	BOOST_CHECK(b0 == a1);
	BOOST_CHECK(b1 == a0);
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
	BOOST_CHECK((std::is_same<value_type const &, reference>::value));
	T const &v1 = *other;
	BOOST_CHECK(v == v1);

	BOOST_CHECK(is_lvalue_iterator<Iterator>::value);
	BOOST_CHECK(!is_non_const_lvalue_iterator<Iterator>::value);
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
	BOOST_CHECK((std::is_same<value_type &, reference>::value));
	T &v2 = *other;
	BOOST_CHECK(v0 == v2);
	*iter = v1;
	T &v3 = *iter;
	BOOST_CHECK(v1 == v3);
	BOOST_CHECK(is_lvalue_iterator<Iterator>::value);
	BOOST_CHECK(is_non_const_lvalue_iterator<Iterator>::value);
}

template <typename Iterator, typename T>
void forward_readable_iterator_test(Iterator i, Iterator j, T val1, T val2)
{
	Iterator i2;
	Iterator i3(i);
	i2 = i;
	BOOST_CHECK(i2 == i3);
	BOOST_CHECK(i != j);
	BOOST_CHECK(i2 != j);
	readable_iterator_test(i, val1);
	readable_iterator_test(i2, val1);
	readable_iterator_test(i3, val1);

	BOOST_CHECK(i == i2++);
	BOOST_CHECK(i != ++i3);

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

	BOOST_CHECK(i == iter--);
	BOOST_CHECK(i != --i2);

	readable_iterator_test(i, v2);
	readable_iterator_test(iter, v1);
	readable_iterator_test(i2, v1);

	--i;
	BOOST_CHECK(i == iter);
	BOOST_CHECK(i == i2);
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
		BOOST_CHECK(i == (j + c));
		BOOST_CHECK(*i == vals[c]);
		typename std::iterator_traits<
			Iterator
		>::value_type x = j[c];
		BOOST_CHECK(*i == x);
		BOOST_CHECK(*i == *(j + c));
		BOOST_CHECK(*i == *(c + j));
		++i;
		BOOST_CHECK(i > j);
		BOOST_CHECK(i >= j);
		BOOST_CHECK(j <= i);
		BOOST_CHECK(j < i);
	}

	Iterator k = j + N - 1;
	for (auto c(0); c < N-1; ++c) {
		BOOST_CHECK(i == k - c);
		BOOST_CHECK(*i == vals[N - 1 - c]);
		typename std::iterator_traits<
			Iterator
		>::value_type x = j[N - 1 - c];
		BOOST_CHECK(*i == x);
		Iterator q = k - c; 
		BOOST_CHECK(*i == *q);
		BOOST_CHECK(i > j);
		BOOST_CHECK(i >= j);
		BOOST_CHECK(j <= i);
		BOOST_CHECK(j < i);
		--i;
	}
}

}}}}
#endif
