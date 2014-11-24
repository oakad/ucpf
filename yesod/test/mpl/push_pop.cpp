/*=============================================================================
    Copyright (c) 2000-2004 Aleksey Gurtovoy
    Copyright (c) 2009      Steven Watanabe
    Copyright (c) 2013      Alex Dubov <oakad@yahoo.com>

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#define BOOST_TEST_MODULE yesod_mpl
#include <boost/test/unit_test.hpp>

#include <yesod/mpl/list.hpp>
#include <yesod/mpl/size.hpp>
#include <yesod/mpl/push_pop.hpp>
#include <yesod/mpl/front_back.hpp>

namespace ucpf { namespace yesod { namespace mpl {
namespace test {

struct no_push_back_tag {};

struct no_push_back {
	typedef no_push_back_tag tag;
};

struct has_push_back_tag {};

struct with_push_back {
	typedef has_push_back_tag tag;
};

struct no_push_front_tag {};

struct no_push_front {
    typedef no_push_front_tag tag;
};

}

namespace detail {

template <>
struct push_back_impl<test::has_push_back_tag> {
	template<class Seq, class T>
	struct apply {
		typedef test::no_push_back type;
	};
};

}

BOOST_AUTO_TEST_CASE(push_pop_0)
{
	BOOST_CHECK(!(has_push_back<test::no_push_back>::value));
	BOOST_CHECK((has_push_back<test::with_push_back>::value));

	typedef push_back<test::with_push_back, int>::type t;
	BOOST_CHECK((std::is_same<t, test::no_push_back>::value));
}

BOOST_AUTO_TEST_CASE(push_pop_1)
{
	typedef push_front<list<>, long>::type res1;
	typedef push_front<list<long>, int>::type res2;
	typedef push_front<list<int, long>, char>::type res3;

	BOOST_CHECK_EQUAL((size<res1>::value), 1);
	BOOST_CHECK_EQUAL((size<res2>::value), 2);
	BOOST_CHECK_EQUAL((size<res3>::value), 3);

	BOOST_CHECK((std::is_same<front<res1>::type, long>::value));
	BOOST_CHECK((std::is_same<front<res2>::type, int>::value));
	BOOST_CHECK((std::is_same<front<res3>::type, char>::value));

	BOOST_CHECK((has_push_front<list<>>::value));
	BOOST_CHECK((has_push_front<list<long>>::value));

	BOOST_CHECK(!(has_push_back<list<>>::value));

	BOOST_CHECK(!(has_push_front<test::no_push_front>::value));
}

BOOST_AUTO_TEST_CASE(push_pop_2)
{
	typedef list<long>::type types1;
	typedef list<int, long>::type types2;
	typedef list<char, int, long>::type types3;

	typedef pop_front<types1>::type result1;
	typedef pop_front<types2>::type result2;
	typedef pop_front<types3>::type result3;

	BOOST_CHECK_EQUAL((size<result1>::value), 0);
	BOOST_CHECK_EQUAL((size<result2>::value), 1);
	BOOST_CHECK_EQUAL((size<result3>::value), 2);

	BOOST_CHECK((std::is_same<front<result2>::type, long>::value));
	BOOST_CHECK((std::is_same<front<result3>::type, int>::value));
}

}}}
