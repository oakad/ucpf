/*==============================================================================
    Copyright (c) 2003-2004 Aleksey Gurtovoy
    Copyright (c) 2013      Alex Dubov <oakad@yahoo.com>

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE yesod_mpl
#include <boost/test/unit_test.hpp>

#include <yesod/mpl/logical.hpp>
#include <yesod/mpl/comparison.hpp>
#include <yesod/mpl/arithmetic.hpp>

namespace ucpf { namespace yesod { namespace mpl {
namespace test {

struct complex_tag : int_<10> {};

template <typename Re, typename Im>
struct complex {
	typedef complex_tag tag;
	typedef complex type;
	typedef Re real;
	typedef Im imag;
};

template <typename C>
struct real : C::real {};

template <typename C>
struct imag : C::imag {};

}

template <>
struct numeric_cast<detail::integral_c_tag, test::complex_tag> {
	template <typename N>
	struct apply : test::complex<
		N, integral_constant<typename N::value_type, 0>
	> {};
};

namespace detail {

template <>
struct plus_impl<test::complex_tag, test::complex_tag> {
	template <typename N0, typename N1>
	struct apply : test::complex<
		plus<typename N0::real, typename N1::real>,
		plus<typename N0::imag, typename N1::imag>
	> {};
};

template <>
struct times_impl<test::complex_tag, test::complex_tag> {
	template <typename N0, typename N1>
	struct apply : test::complex<
		minus<
			times<typename N0::real, typename N1::real>,
			times<typename N0::imag, typename N1::imag>
		>, plus<
			times<typename N0::real, typename N1::imag>,
			times<typename N0::imag, typename N1::real>
		>
	> {};
};

template <>
struct equal_to_impl<test::complex_tag, test::complex_tag> {
	template<typename N0, typename N1>
	struct apply : and_<
		equal_to<typename N0::real, typename N1::real>,
		equal_to<typename N0::imag, typename N1::imag>
	> {};
};

}

namespace test {

typedef int_<2> i;
typedef complex<int_<5>, int_<-1>> c1;
typedef complex<int_<-5>, int_<1>> c2;

}

BOOST_AUTO_TEST_CASE(numeric_ops_0)
{
	typedef plus<test::c1, test::c2>::type r1;
	BOOST_CHECK_EQUAL((test::real<r1>::value), 0);
	BOOST_CHECK_EQUAL((test::imag<r1>::value), 0);

	typedef plus<test::c1, test::c1>::type r2;
	BOOST_CHECK_EQUAL((test::real<r2>::value), 10);
	BOOST_CHECK_EQUAL((test::imag<r2>::value), -2);

	typedef plus<test::c2, test::c2>::type r3;
	BOOST_CHECK_EQUAL((test::real<r3>::value), -10);
	BOOST_CHECK_EQUAL((test::imag<r3>::value), 2);

	typedef plus<test::c1, test::i>::type r4;
	BOOST_CHECK_EQUAL((test::real<r4>::value), 7);
	BOOST_CHECK_EQUAL((test::imag<r4>::value), -1);

	typedef plus<test::i, test::c2>::type r5;
	BOOST_CHECK_EQUAL((test::real<r5>::value), -3);
	BOOST_CHECK_EQUAL((test::imag<r5>::value), 1);
}

BOOST_AUTO_TEST_CASE(numeric_ops_1)
{
	typedef times<test::c1, test::c2>::type r1;
	BOOST_CHECK_EQUAL((test::real<r1>::value), -24);
	BOOST_CHECK_EQUAL((test::imag<r1>::value), 10);

	typedef times<test::c1, test::c1>::type r2;
	BOOST_CHECK_EQUAL((test::real<r2>::value), 24);
	BOOST_CHECK_EQUAL((test::imag<r2>::value), -10);

	typedef times<test::c2, test::c2>::type r3;
	BOOST_CHECK_EQUAL((test::real<r3>::value), 24);
	BOOST_CHECK_EQUAL((test::imag<r3>::value), -10);

	typedef times<test::c1, test::i>::type r4;
	BOOST_CHECK_EQUAL((test::real<r4>::value), 10);
	BOOST_CHECK_EQUAL((test::imag<r4>::value), -2);

	typedef times<test::i, test::c2>::type r5;
	BOOST_CHECK_EQUAL((test::real<r5>::value), -10 );
	BOOST_CHECK_EQUAL((test::imag<r5>::value), 2 );
}

BOOST_AUTO_TEST_CASE(numeric_ops_2)
{
	BOOST_CHECK((equal_to<test::c1, test::c1>::value));
	BOOST_CHECK((equal_to<test::c2, test::c2>::value));
	BOOST_CHECK(!(equal_to<test::c1, test::c2>::value));

	BOOST_CHECK((equal_to<                               \
		test::c1, test::complex<long_<5>, long_<-1>> \
	>::value));

	BOOST_CHECK(!(equal_to<test::c1, test::i>::value));
	BOOST_CHECK(!(equal_to<test::i, test::c2>::value));
}

}}}
