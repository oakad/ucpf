/*=============================================================================
    Copyright (c) 2000-2007 Aleksey Gurtovoy
    Copyright (c) 2003-2004 David Abrahams
    Copyright (c) 2013      Alex Dubov <oakad@yahoo.com>

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#define BOOST_TEST_MODULE yesod_mpl
#include <boost/test/included/unit_test.hpp>

#include <yesod/mpl/at.hpp>
#include <yesod/mpl/set.hpp>
#include <yesod/mpl/size.hpp>
#include <yesod/mpl/find.hpp>
#include <yesod/mpl/clear.hpp>
#include <yesod/mpl/order.hpp>
#include <yesod/mpl/insert.hpp>
#include <yesod/mpl/has_key.hpp>
#include <yesod/mpl/contains.hpp>
#include <yesod/mpl/erase_key.hpp>

#include "data_types.hpp"

namespace ucpf { namespace yesod { namespace mpl {
namespace test {

template <typename Set>
void empty_set_test()
{
	BOOST_CHECK_EQUAL((size<Set>::value), 0);
	BOOST_CHECK((empty<Set>::value));

	BOOST_CHECK((std::is_same<typename clear<Set>::type, set<>>::value));
	BOOST_CHECK((std::is_same<typename at<Set, int>::type, void_>::value));
	BOOST_CHECK((std::is_same<typename at<Set, char>::type, void_>::value));
	BOOST_CHECK((std::is_same<typename at<Set, long>::type, void_>::value));

	BOOST_CHECK(!(has_key<Set, int>::value));
	BOOST_CHECK(!(has_key<Set, char>::value));
	BOOST_CHECK(!(has_key<Set, long>::value));

	typedef typename order<Set, int>::type o1;
	typedef typename order<Set, char>::type o2;
	typedef typename order<Set, long>::type o3;
	BOOST_CHECK((std::is_same<o1, void_>::value));
	BOOST_CHECK((std::is_same<o2, void_>::value));
	BOOST_CHECK((std::is_same<o3, void_>::value));

	typedef typename begin<Set>::type first;
	typedef typename end<Set>::type last;

	BOOST_CHECK((std::is_same<first, last>::value));
	BOOST_CHECK_EQUAL((distance<first, last>::value), 0);
}

template <typename Set>
void int_set_test()
{
	BOOST_CHECK_EQUAL((size<Set>::value), 1);
	BOOST_CHECK(!(empty<Set>::value));

	BOOST_CHECK((std::is_same<typename clear<Set>::type, set<>>::value));
	BOOST_CHECK((std::is_same<typename at<Set, int>::type, int>::value));
	BOOST_CHECK((std::is_same<typename at<Set, char>::type, void_>::value));
	BOOST_CHECK((std::is_same<typename at<Set, long>::type, void_>::value));

	BOOST_CHECK((has_key<Set, int>::value));
	BOOST_CHECK(!(has_key<Set, char>::value));
	BOOST_CHECK(!(has_key<Set, long>::value));

	typedef typename order<Set, int>::type o1;
	typedef typename order<Set, char>::type o2;
	typedef typename order<Set, long>::type o3;
	BOOST_CHECK(!(std::is_same<o1, void_>::value));
	BOOST_CHECK((std::is_same<o2, void_>::value));
	BOOST_CHECK((std::is_same<o3, void_>::value));

	typedef typename begin<Set>::type first;
	typedef typename end<Set>::type last;

	BOOST_CHECK((std::is_same<typename deref<first>::type, int>::value));
	BOOST_CHECK((std::is_same<typename next<first>::type, last>::value));

	BOOST_CHECK_EQUAL((distance<first, last>::value), 1);
	BOOST_CHECK((contains<Set, int>::value));
}

template <typename Set>
void int_char_set_test()
{
	BOOST_CHECK_EQUAL((size<Set>::value), 2);
	BOOST_CHECK(!(empty<Set>::value));
	BOOST_CHECK((std::is_same<typename clear<Set>::type, set<>>::value));
	BOOST_CHECK((std::is_same<typename at<Set, int>::type, int>::value));
	BOOST_CHECK((std::is_same<typename at<Set, char>::type, char>::value));

	BOOST_CHECK((has_key<Set, char>::value));
	BOOST_CHECK(!(has_key<Set, long>::value));

	typedef typename order<Set, int>::type o1;
	typedef typename order<Set, char>::type o2;
	typedef typename order<Set, long>::type o3;
	BOOST_CHECK(!(std::is_same<o1, void_>::value));
	BOOST_CHECK(!(std::is_same<o2, void_>::value));
	BOOST_CHECK((std::is_same<o3, void_>::value));
	BOOST_CHECK(!(std::is_same<o1, o2>::value));

	typedef typename begin<Set>::type first;
	typedef typename end<Set>::type last;

	BOOST_CHECK_EQUAL((distance<first, last>::value), 2);

	BOOST_CHECK((contains<Set, int>::value));
	BOOST_CHECK((contains<Set, char>::value));
}

template <typename Set>
void int_char_long_set_test()
{
	BOOST_CHECK_EQUAL((size<Set>::value), 3);
	BOOST_CHECK(!(empty<Set>::value));
	BOOST_CHECK((std::is_same<typename clear<Set>::type, set<>>::value));
	BOOST_CHECK((std::is_same<typename at<Set, int>::type, int>::value));
	BOOST_CHECK((std::is_same<typename at<Set, char>::type, char>::value));
	BOOST_CHECK((std::is_same<typename at<Set, long>::type, long>::value));

	BOOST_CHECK((has_key<Set, long>::value));
	BOOST_CHECK((has_key<Set, int>::value));
	BOOST_CHECK((has_key<Set, char>::value));

	typedef typename order<Set, int>::type o1;
	typedef typename order<Set, char>::type o2;
	typedef typename order<Set, long>::type o3;

	BOOST_CHECK(!(std::is_same<o1, void_>::value));
	BOOST_CHECK(!(std::is_same<o2, void_>::value));
	BOOST_CHECK(!(std::is_same<o3, void_>::value));
	BOOST_CHECK(!(std::is_same<o1, o2>::value));
	BOOST_CHECK(!(std::is_same<o1, o3>::value));
	BOOST_CHECK(!(std::is_same<o2, o3>::value));

	typedef typename begin<Set>::type first;
	typedef typename end<Set>::type last;
	BOOST_CHECK_EQUAL((distance<first, last>::value), 3);

	BOOST_CHECK((contains<Set, int>::value));
	BOOST_CHECK((contains<Set, char>::value));
	BOOST_CHECK((contains<Set, long>::value));
}

template <typename S0, typename S1, typename S2, typename S3>
void basic_set_test()
{
	empty_set_test<S0>();
	empty_set_test<typename erase_key<S1,int>::type>();
	empty_set_test<
		typename erase_key<
			typename erase_key<S2, char>::type, int
		>::type
	>();
	empty_set_test<typename erase_key<
		typename erase_key<
			typename erase_key<S3, char>::type, long
		>::type, int
	>::type>();

	int_set_test<S1>();
	int_set_test<typename insert<S0, int>::type>();

	int_set_test<typename erase_key<S2, char>::type>();
	int_set_test<typename erase_key<
		typename erase_key<S3, char>::type, long
	>::type>();

	int_char_set_test<S2>();
	int_char_set_test<
		typename insert<typename insert<S0, char>::type, int
	>::type>();

	int_char_set_test<typename insert<S1, char>::type>();
	int_char_set_test<typename erase_key<S3, long>::type>();

	int_char_long_set_test<S3>();
	int_char_long_set_test<typename insert<
		typename insert<
			typename insert<S0, char>::type, long
		>::type, int
	>::type>();
	int_char_long_set_test<typename insert<
		typename insert<S1, long>::type, char
	>::type>();
	int_char_long_set_test<typename insert<S2, long>::type>();
}

template <typename Set>
void empty_set_types_variety_test()
{
    BOOST_CHECK(!(has_key<Set, char>::value));
    BOOST_CHECK(!(has_key<Set, int>::value));
    BOOST_CHECK(!(has_key<Set, UDT>::value));
    BOOST_CHECK(!(has_key<Set, incomplete>::value));

    BOOST_CHECK(!(has_key<Set, char const>::value));
    BOOST_CHECK(!(has_key<Set, int const>::value));
    BOOST_CHECK(!(has_key<Set, UDT const>::value));
    BOOST_CHECK(!(has_key<Set, incomplete const>::value));

    BOOST_CHECK(!(has_key<Set, int *>::value));
    BOOST_CHECK(!(has_key<Set, UDT *>::value));
    BOOST_CHECK(!(has_key<Set, incomplete *>::value));

    BOOST_CHECK(!(has_key<Set, int &>::value));
    BOOST_CHECK(!(has_key<Set, UDT &>::value));
    BOOST_CHECK(!(has_key<Set, incomplete &>::value));
}

template <typename Set>
void set_types_variety_test()
{
	BOOST_CHECK_EQUAL((size<Set>::value), 8);

	BOOST_CHECK((has_key<Set, char>::value));
	BOOST_CHECK((has_key<Set, int const>::value));
	BOOST_CHECK((has_key<Set, long *>::value));
	BOOST_CHECK((has_key<Set, UDT * const>::value));
	BOOST_CHECK((has_key<Set, incomplete>::value));
	BOOST_CHECK((has_key<Set, abstract>::value));
	BOOST_CHECK((has_key<Set, incomplete volatile &>::value));
	BOOST_CHECK((has_key<Set, abstract const &>::value));

	BOOST_CHECK(!(has_key<Set, char const>::value));
	BOOST_CHECK(!(has_key<Set, int>::value));
	BOOST_CHECK(!(has_key<Set, long * const>::value));
	BOOST_CHECK(!(has_key<Set, UDT *>::value));
	BOOST_CHECK(!(has_key<Set, incomplete const>::value));
	BOOST_CHECK(!(has_key<Set, abstract volatile>::value));
	BOOST_CHECK(!(has_key<Set, incomplete &>::value));
	BOOST_CHECK(!(has_key<Set, abstract &>::value));
}

template <typename Set>
void find_test()
{
	BOOST_CHECK_EQUAL((size<Set>::value), 3);

	typedef typename end<Set>::type not_found;
	BOOST_CHECK(!(                                              \
		std::is_same<                                       \
			typename find<Set, int>::type, not_found    \
		>::value                                            \
	));
	BOOST_CHECK(!(                                              \
		std::is_same<                                       \
			typename find<Set, long>::type, not_found   \
		>::value                                            \
	));
	BOOST_CHECK(!(                                              \
		std::is_same<                                       \
			typename find<Set, char>::type, not_found   \
		>::value                                            \
	));
	BOOST_CHECK((                                               \
		std::is_same<                                       \
			typename find<Set, char *>::type, not_found \
		>::value                                            \
	));
}

template <typename Set, typename Set::value_type k>
struct at_c : at<Set, integral_constant<typename Set::value_type, k>>::type {
};

}

BOOST_AUTO_TEST_CASE(set_0)
{
	typedef set<> s01;
	typedef set<int> s11;
	typedef set<int, char> s21;
	typedef set<char, int> s22;
	typedef set<int, char, long> s31;
	typedef set<int, long, char> s32;
	typedef set<long, char, int> s33;

	test::basic_set_test<s01, s11, s21, s31>();
	test::basic_set_test<s01, s11, s22, s31>();
	test::basic_set_test<s01, s11, s22, s32>();
	test::basic_set_test<s01, s11, s22, s33>();
}

BOOST_AUTO_TEST_CASE(set_1)
{
	using namespace test;

	empty_set_types_variety_test<set<>>();
	empty_set_types_variety_test<set<>::type>();

	typedef set<
		char, int const,
		long*, UDT* const, incomplete, abstract,
		incomplete volatile &,
		abstract const &
	> s;

	set_types_variety_test<s>();
	set_types_variety_test<s::type>();
}

BOOST_AUTO_TEST_CASE(set_2)
{
	typedef set<int, long, char> s;
	test::find_test<s>();
	test::find_test<s::type>();
}

BOOST_AUTO_TEST_CASE(set_3)
{
	typedef set_c<bool, true>::type s1;
	typedef set_c<bool, false>::type s2;
	typedef set_c<bool, true, false>::type s3;

	BOOST_CHECK_EQUAL((size<s1>::value), 1);
	BOOST_CHECK_EQUAL((size<s2>::value), 1);
	BOOST_CHECK_EQUAL((size<s3>::value), 2);

	BOOST_CHECK((std::is_same<s1::value_type, bool>::value));
	BOOST_CHECK((std::is_same<s3::value_type, bool>::value));
	BOOST_CHECK((std::is_same<s2::value_type, bool>::value));

	BOOST_CHECK_EQUAL((test::at_c<s1, true>::value), true);
	BOOST_CHECK_EQUAL((test::at_c<s2, false>::value), false);
	BOOST_CHECK_EQUAL((test::at_c<s3, true>::value), true);
	BOOST_CHECK_EQUAL((test::at_c<s3, false>::value), false);

	BOOST_CHECK((std::is_same<                          \
		typename test::at_c<s1, false>::type, void_ \
	>::value));
	BOOST_CHECK((std::is_same<                         \
		typename test::at_c<s2, true>::type, void_ \
	>::value));

	typedef begin<s1>::type first1;
	typedef end<s1>::type last1;
	BOOST_CHECK_EQUAL((distance<first1, last1>::value), 1);

	typedef begin<s2>::type first2;
	typedef end<s2>::type last2;
	BOOST_CHECK_EQUAL((distance<first2, last2>::value), 1);

	typedef begin<s3>::type first3;
	typedef end<s3>::type last3;
	BOOST_CHECK_EQUAL((distance<first3, last3>::value), 2);
}

BOOST_AUTO_TEST_CASE(set_4)
{
	typedef set_c<char, 'a'>::type s1;
	typedef set_c<char, 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'>::type s2;

	BOOST_CHECK_EQUAL((size<s1>::value), 1);
	BOOST_CHECK_EQUAL((size<s2>::value), 8);

	BOOST_CHECK((std::is_same<s1::value_type, char>::value));
	BOOST_CHECK((std::is_same<s2::value_type, char>::value));

	BOOST_CHECK_EQUAL((test::at_c<s1, 'a'>::value), 'a');
	BOOST_CHECK_EQUAL((test::at_c<s2, 'a'>::value), 'a');
	BOOST_CHECK_EQUAL((test::at_c<s2, 'd'>::value), 'd');
	BOOST_CHECK_EQUAL((test::at_c<s2, 'h'>::value), 'h');

	BOOST_CHECK((std::is_same<test::at_c<s1, 'z'>::type, void_>::value));
	BOOST_CHECK((std::is_same<test::at_c<s2, 'k'>::type, void_>::value));

	typedef begin<s1>::type first1;
	typedef end<s1>::type last1;
	BOOST_CHECK_EQUAL((distance<first1, last1>::value), 1);

	typedef begin<s2>::type first2;
	typedef end<s2>::type last2;
	BOOST_CHECK_EQUAL((distance<first2, last2>::value), 8);
}

}}}
