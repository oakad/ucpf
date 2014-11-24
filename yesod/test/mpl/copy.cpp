/*=============================================================================
    Copyright (c) 2000-2004 Aleksey Gurtovoy
    Copyright (c) 2003-2004 David Abrahams
    Copyright (c) 2013      Alex Dubov <oakad@yahoo.com>

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#define BOOST_TEST_MODULE yesod_mpl
#include <boost/test/unit_test.hpp>

#include <yesod/mpl/copy.hpp>
#include <yesod/mpl/size.hpp>
#include <yesod/mpl/list.hpp>
#include <yesod/mpl/equal.hpp>
#include <yesod/mpl/clear.hpp>
#include <yesod/mpl/vector.hpp>
#include <yesod/mpl/range_c.hpp>

namespace ucpf { namespace yesod { namespace mpl {
namespace detail {

struct push_back_only_tag {};

}

namespace test {

template <typename Seq>
struct push_back_only {
	typedef detail::push_back_only_tag tag;
	typedef Seq seq;
};

}

namespace detail {

template <>
struct begin_impl<push_back_only_tag> {
	template <typename Seq>
	struct apply : begin<typename Seq::seq> {};
};

template <>
struct end_impl<push_back_only_tag> {
	template <typename Seq>
	struct apply : end<typename Seq::seq> {};
};

template <>
struct size_impl<push_back_only_tag> {
	template <typename Seq>
	struct apply : size<typename Seq::seq> {};
};

template <>
struct empty_impl<push_back_only_tag> {
	template <typename Seq>
	struct apply : empty<typename Seq::seq> {};
};

template <>
struct front_impl<push_back_only_tag> {
	template <typename Seq>
	struct apply : front<typename Seq::seq> {};
};

template <>
struct insert_impl<push_back_only_tag> {
	template <typename Seq, typename Pos, typename X>
	struct apply {
		typedef test::push_back_only<
			typename insert<typename Seq::seq, Pos, X>::type
		> type;
	};
};

template <>
struct insert_range_impl<push_back_only_tag> {
	template <typename Seq, typename Pos, typename X>
	struct apply {
		typedef test::push_back_only <
			typename insert_range<typename Seq::seq, Pos, X>::type
		> type;
	};
};

template <>
struct erase_impl<push_back_only_tag> {
	template <typename Seq, typename Iter1, typename Iter2>
	struct apply {
		typedef test::push_back_only<
			typename erase<typename Seq::seq, Iter1, Iter2>::type
		> type;
	};
};

template <>
struct clear_impl<push_back_only_tag> {
	template <typename Seq>
	struct apply {
		typedef test::push_back_only<
			typename clear<typename Seq::seq>::type
		> type;
	};
};

template <>
struct push_back_impl<push_back_only_tag> {
	template <typename Seq, typename X>
	struct apply {
		typedef test::push_back_only<
			typename push_back<typename Seq::seq, X>::type
		> type;
	};
};

template <>
struct pop_back_impl<push_back_only_tag> {
	template <typename Seq>
	struct apply {
		typedef test::push_back_only <
			typename pop_back<typename Seq::seq>::type
		> type;
	};
};

template <>
struct back_impl<push_back_only_tag> {
	template <typename Seq>
	struct apply : back<typename Seq::seq> {};
};

template <>
struct has_push_back_impl<push_back_only_tag> {
	template <typename Seq>
	struct apply : less<size<Seq>, int_<10>> {};
};

}

BOOST_AUTO_TEST_CASE(copy_0)
{
	typedef vector_c<int, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0> answer;
	typedef copy<
		range_c<int, 0, 10>, front_inserter<vector<>>
	>::type result;

	BOOST_CHECK_EQUAL((size<result>::value), 10);
	BOOST_CHECK((equal<result, answer>::value));
}

BOOST_AUTO_TEST_CASE(copy_1)
{
	typedef vector_c<int, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19> numbers;
	typedef reverse_copy<
		range_c<int,0,10>, front_inserter<numbers>
	>::type result;

	BOOST_CHECK_EQUAL((size<result>::value), 20);
	BOOST_CHECK((equal<result, range_c<int, 0, 20>>::value));
}

BOOST_AUTO_TEST_CASE(copy_2)
{
	typedef vector_c<int, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9> numbers;
	typedef copy<test::push_back_only<numbers>>::type result;

	BOOST_CHECK((equal<numbers, result>::value));
}

BOOST_AUTO_TEST_CASE(copy_3)
{
	typedef list_c<int, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9>::type numbers;
	typedef list_c<int, 4, 3, 2, 1, 0>::type answer;
	typedef copy_if<
		numbers, less<arg<-1>, int_<5>>, front_inserter<list_c<int>>
	>::type result;

	BOOST_CHECK_EQUAL((size<result>::value), 5);
	BOOST_CHECK((equal<result, answer>::value));
}

BOOST_AUTO_TEST_CASE(copy_4)
{
	typedef list<
		int, float, long, float, char, long, double, double
	>::type types;
	typedef list<float, float, double, double>::type float_types;
	typedef reverse_copy_if<
		types, std::is_floating_point<arg<-1>>, front_inserter<list<>>
        >::type result;

	BOOST_CHECK_EQUAL((size<result>::value), 4);
	BOOST_CHECK((equal<result,float_types>::value));
}

}}}
