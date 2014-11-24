/*=============================================================================
    Copyright (c) 2001-2004 Aleksey Gurtovoy
    Copyright (c) 2013      Alex Dubov <oakad@yahoo.com>

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#define BOOST_TEST_MODULE yesod_mpl
#include <boost/test/unit_test.hpp>

#include <yesod/mpl/apply.hpp>
#include <yesod/mpl/lambda.hpp>
#include <yesod/mpl/sizeof.hpp>
#include <yesod/mpl/logical.hpp>
#include <yesod/mpl/comparison.hpp>

#include "data_types.hpp"

namespace ucpf { namespace yesod { namespace mpl {
namespace test {

struct my {
	char a[100];
};

}

BOOST_AUTO_TEST_CASE(lambda_0)
{
	// !(x == char) && !(x == double) || sizeof(x) > 8
	typedef lambda<or_<
		and_<
			not_<std::is_same<arg<0>, char>>,
			not_<std::is_floating_point<arg<0>>>
		>,
		greater<sizeof_<arg<0>>, mpl::size_t<8>>
	>>::type f;

	BOOST_CHECK(!(apply_wrap<f, char>::type::value));
	BOOST_CHECK(!(apply_wrap<f, double>::type::value));
	BOOST_CHECK((apply_wrap<f, long>::type::value));
	BOOST_CHECK((apply_wrap<f, test::my>::type::value));
}

BOOST_AUTO_TEST_CASE(lambda_1)
{
	// x == y || x == my || sizeof(x) == sizeof(y)
	typedef lambda<or_<
		std::is_same<arg<0>, arg<1>>,
		std::is_same<arg<1>, test::my>,
		equal_to<sizeof_<arg<0>>, sizeof_<arg<1>>>
	>>::type f;

	BOOST_CHECK(!(apply_wrap<f, double, char>::type::value));
	BOOST_CHECK(!(apply_wrap<f, test::my, int>::type::value));
	BOOST_CHECK(!(apply_wrap<f, test::my, char[99]>::type::value));
	BOOST_CHECK((apply_wrap<f, int, int>::type::value));
	BOOST_CHECK((apply_wrap<f, test::my, test::my>::type::value));
	BOOST_CHECK((apply_wrap<f, signed long, unsigned long>::type::value));
}

BOOST_AUTO_TEST_CASE(lambda_2)
{
	// bind <-> lambda interaction
	typedef lambda<less<arg<0>, arg<1>>>::type pred;
	typedef bind<pred, arg<0>, int_<4>> f;

	BOOST_CHECK((apply_wrap<f, int_<3>>::type::value ));
}

BOOST_AUTO_TEST_CASE(lambda_3)
{
#define AUX_LAMBDA_TEST(T)                                                    \
	{ BOOST_CHECK((                                                       \
		apply<lambda<std::is_same<arg<-1>, T>>::type, T>::type::value \
	)); }                                                                 \
	{ BOOST_CHECK((                                                       \
		apply<lambda<std::is_same<T, arg<-1>>>::type, T>::type::value \
	)); }                                                                 \
	{ BOOST_CHECK((                                                       \
		apply<lambda<                                                 \
			std::is_same<arg<-1>, arg<-1>>>::type, T, T           \
		>::type::value)); }

	AUX_LAMBDA_TEST(test::UDT);
	AUX_LAMBDA_TEST(test::abstract);
	AUX_LAMBDA_TEST(test::noncopyable);
	AUX_LAMBDA_TEST(test::incomplete);
	AUX_LAMBDA_TEST(int);
	AUX_LAMBDA_TEST(void);
	AUX_LAMBDA_TEST(double);
	AUX_LAMBDA_TEST(int &);
	AUX_LAMBDA_TEST(int *);

	AUX_LAMBDA_TEST(int[]);

	AUX_LAMBDA_TEST(int[10]);
	AUX_LAMBDA_TEST(int (*)())
	AUX_LAMBDA_TEST(test::mem_ptr);
	AUX_LAMBDA_TEST(test::mem_fun_ptr);
}
}}}
