/*=============================================================================
    Copyright (c) 2003-2004 Aleksey Gurtovoy
    Copyright (c) 2003-2004 David Abrahams
    Copyright (c) 2013      Alex Dubov <oakad@yahoo.com>

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#define BOOST_TEST_MODULE yesod_mpl
#include <boost/test/unit_test.hpp>

#include <yesod/mpl/at.hpp>
#include <yesod/mpl/map.hpp>
#include <yesod/mpl/size.hpp>
#include <yesod/mpl/clear.hpp>
#include <yesod/mpl/order.hpp>
#include <yesod/mpl/insert.hpp>
#include <yesod/mpl/has_key.hpp>
#include <yesod/mpl/contains.hpp>
#include <yesod/mpl/erase_key.hpp>

namespace ucpf { namespace yesod { namespace mpl {

BOOST_AUTO_TEST_CASE(map_0)
{
	typedef map<
		pair<int, unsigned>,
		pair<char, unsigned char>
	> m_;

	typedef erase_key<m_, char>::type m;

	BOOST_CHECK_EQUAL((size<m>::value), 1);
	BOOST_CHECK(!(empty<m>::value));
	BOOST_CHECK((std::is_same<clear<m>::type, map<>>::value));
	BOOST_CHECK((std::is_same<at<m, int>::type, unsigned>::value));
	BOOST_CHECK((std::is_same<at<m, char>::type, void_>::value));
	BOOST_CHECK((contains<m, pair<int, unsigned>>::value));
	BOOST_CHECK(!(contains<m, pair<int, int>>::value));
	BOOST_CHECK(!(contains<m, pair<char, unsigned char>>::value));
	BOOST_CHECK(!(has_key<m, char>::value));
	BOOST_CHECK((has_key<m, int>::value));

	BOOST_CHECK(!(std::is_same<order<m, int>::type, void_>::value));
	BOOST_CHECK((std::is_same<order<m, char>::type, void_>::value));

	typedef begin<m>::type first;
	typedef end<m>::type last;

	BOOST_CHECK((                            \
		std::is_same<deref<first>::type, \
		pair<int, unsigned>>::value      \
	));
	BOOST_CHECK((std::is_same<next<first>::type, last>::value));

	typedef insert<m, pair<char, long>>::type m2;

	BOOST_CHECK_EQUAL((size<m2>::value), 2);
	BOOST_CHECK(!(empty<m2>::value));
	BOOST_CHECK((std::is_same<clear<m2>::type, map<>>::value));
	BOOST_CHECK((std::is_same<at<m2, int>::type, unsigned>::value));
	BOOST_CHECK((std::is_same<at<m2, char>::type, long>::value));

	BOOST_CHECK((contains<m2, pair<int, unsigned>>::value));
	BOOST_CHECK(!(contains<m2, pair<int, int>>::value));
	BOOST_CHECK(!(contains<m2, pair<char, unsigned char>>::value));
	BOOST_CHECK((contains<m2, pair<char, long>>::value));

	BOOST_CHECK((has_key<m2, char>::value));
	BOOST_CHECK(!(has_key<m2, long>::value));
	BOOST_CHECK(!(std::is_same<order<m2, int>::type, void_>::value));
	BOOST_CHECK(!(std::is_same<order<m2, char>::type, void_>::value));
	BOOST_CHECK(!(std::is_same<order<m2, char>::type,\
				   order<m2, int>::type>::value));

	typedef begin<m2>::type first2;
	typedef end<m2>::type last2;

	BOOST_CHECK((std::is_same<deref<first2>::type,\
				  pair<int, unsigned>>::value));

	typedef next<first2>::type iter;

	BOOST_CHECK((std::is_same<deref<iter>::type, pair<char, long>>::value));
	BOOST_CHECK((std::is_same<next<iter>::type, last2>::value));

	typedef insert<m2, pair<int, unsigned>>::type s2_1;

	BOOST_CHECK((std::is_same<m2, s2_1>::value));

	typedef insert<m2, pair<long, unsigned>>::type m3;

	BOOST_CHECK_EQUAL((size<m3>::value), 3);
	BOOST_CHECK((has_key<m3, long>::value));
	BOOST_CHECK((has_key<m3, int>::value));
	BOOST_CHECK((has_key<m3, char>::value));
	BOOST_CHECK((contains<m3, pair<long, unsigned>>::value));
	BOOST_CHECK((contains<m3, pair<int, unsigned>>::value));

	typedef insert<m, pair<char, long>>::type m1;

	BOOST_CHECK_EQUAL((size<m1>::value), 2);
	BOOST_CHECK((std::is_same<at<m1, int>::type, unsigned>::value));
	BOOST_CHECK((std::is_same<at<m1, char>::type, long>::value));

	BOOST_CHECK((contains<m1, pair<int, unsigned>>::value));
	BOOST_CHECK(!(contains<m1, pair<int, int>>::value));
	BOOST_CHECK(!(contains<m1, pair<char, unsigned char>>::value));
	BOOST_CHECK((contains<m1, pair<char, long>>::value));

	BOOST_CHECK((std::is_same<m1, m2>::value));

	typedef erase_key<m1, char>::type m_1;

	BOOST_CHECK((std::is_same<m, m_1>::value));
	BOOST_CHECK_EQUAL((size<m_1>::value), 1);
	BOOST_CHECK((std::is_same<at<m_1, char>::type, void_>::value));
	BOOST_CHECK((std::is_same<at<m_1, int>::type, unsigned>::value));

	typedef erase_key<m3, char>::type m2_1;

	BOOST_CHECK_EQUAL((size<m2_1>::type::value), 2);
	BOOST_CHECK((std::is_same<at<m2_1, char>::type, void_>::value));
	BOOST_CHECK((std::is_same<at<m2_1, int>::type, unsigned>::value));
	BOOST_CHECK((std::is_same<at<m2_1, long>::type, unsigned>::value));
}

BOOST_AUTO_TEST_CASE(map_1)
{
	typedef map<> m;
	struct UDT {};
	struct incomplete;

	BOOST_CHECK_EQUAL((size<m>::value), 0);
	BOOST_CHECK((empty<m>::value));

	BOOST_CHECK((std::is_same<clear<m>::type, map<>>::value));
	BOOST_CHECK((std::is_same<at<m, char>::type, void_>::value));

	BOOST_CHECK(!(has_key<m, char>::value));
	BOOST_CHECK(!(has_key<m, int>::value));
	BOOST_CHECK(!(has_key<m, UDT>::value));
	BOOST_CHECK(!(has_key<m, incomplete>::value));

	BOOST_CHECK(!(has_key<m, char const>::value));
	BOOST_CHECK(!(has_key<m, int const>::value));
	BOOST_CHECK(!(has_key<m, UDT const>::value));
	BOOST_CHECK(!(has_key<m, incomplete const>::value));

	BOOST_CHECK(!(has_key<m, int *>::value));
	BOOST_CHECK(!(has_key<m, UDT *>::value));
	BOOST_CHECK(!(has_key<m, incomplete *>::value));

	BOOST_CHECK(!(has_key<m, int &>::value));
	BOOST_CHECK(!(has_key<m, UDT &>::value));
	BOOST_CHECK(!(has_key<m, incomplete &>::value));

	typedef insert<m, pair<char, int>>::type m1;

	BOOST_CHECK_EQUAL((size<m1>::value), 1);
	BOOST_CHECK((std::is_same<at<m1, char>::type, int>::value));

	typedef erase_key<m, char>::type m0_1;

	BOOST_CHECK_EQUAL((size<m0_1>::value), 0);
	BOOST_CHECK((std::is_same<at<m0_1, char>::type, void_>::value));
}

namespace test {

template <typename M>
void t() {
	BOOST_CHECK_EQUAL((size<M>::value), 3);

	typedef typename end<M>::type not_found;

	BOOST_CHECK(!(                                             \
		std::is_same<                                      \
			typename find<M, pair<int, int*>>::type,   \
			not_found                                  \
		>::value                                           \
	));
	BOOST_CHECK(!(                                             \
		std::is_same<                                      \
			typename find<M, pair<long, long*>>::type, \
			not_found                                  \
		>::value                                           \
	));
	BOOST_CHECK(!(                                             \
		std::is_same<                                      \
			typename find<M, pair<char, char*>>::type, \
			not_found                                  \
		>::value                                           \
	));
	BOOST_CHECK((                                              \
		std::is_same<                                      \
			typename find<M, int>::type,               \
			not_found                                  \
		>::value                                           \
	));
};

}

BOOST_AUTO_TEST_CASE(map_2)
{

	typedef map<pair<int, int*>> map_of_1_pair;
	typedef begin<map_of_1_pair>::type iter_to_1_pair;

	BOOST_CHECK((                                     \
		std::is_same<deref<iter_to_1_pair>::type, \
		pair<int, int*>>::value                   \
	));

	typedef map<
		pair<int, int*>,
		pair<long, long*>,
		pair<char, char*>
	> mymap;

	test::t<mymap>();
	test::t<mymap::type>();
}

BOOST_AUTO_TEST_CASE(map_3)
{
	typedef erase_key<
		map<pair<char, double>, pair<int, float>>, char
	>::type int_to_float_map;

	typedef insert<
		int_to_float_map, pair<char, long>
	>::type with_char_too;

	BOOST_CHECK((std::is_same<at<with_char_too, char>::type, long>::value));
}
}}}
