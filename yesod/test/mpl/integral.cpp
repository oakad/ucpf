/*=============================================================================
    Copyright (c) 2001-2006 Aleksey Gurtovoy
    Copyright (c) 2008      Eric Niebler
    Copyright (c) 2013      Alex Dubov <oakad@yahoo.com>

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE yesod_mpl
#include <boost/test/unit_test.hpp>
#include <boost/preprocessor/repeat.hpp>

#include <yesod/mpl/sizeof.hpp>
#include <yesod/mpl/integral.hpp>
#include <yesod/mpl/next_prior.hpp>

#define INTEGRAL_WRAPPER_TEST(unused1, i, T)                  \
	{ BOOST_CHECK((std::is_same<                          \
		WRAPPER(T,i)::value_type, T                   \
	>::value)); }                                         \
	{ BOOST_CHECK((std::is_same<                          \
		WRAPPER(T,i)::type, WRAPPER(T, i)             \
	>::value)); }                                         \
	{ BOOST_CHECK((std::is_same<                          \
		next<WRAPPER(T, i)>::type, WRAPPER(T, i + 1)  \
	>::value)); }                                         \
	{ BOOST_CHECK((std::is_same<                          \
		prior<WRAPPER(T, i)>::type, WRAPPER(T, i - 1) \
	>::value)); }                                         \
	{ BOOST_CHECK_EQUAL((WRAPPER(T,i)::value), i); }      \
	{ BOOST_CHECK_EQUAL((WRAPPER(T,i)()), i); }           \
	{ BOOST_CHECK_EQUAL((WRAPPER(T,i)::value), i); }


namespace ucpf { namespace yesod { namespace mpl {

BOOST_AUTO_TEST_CASE(integral_0)
{
#undef WRAPPER
#define WRAPPER(T, i) integral_constant<T, i>
	BOOST_PP_REPEAT(10, INTEGRAL_WRAPPER_TEST, char);
	BOOST_PP_REPEAT(10, INTEGRAL_WRAPPER_TEST, short);
	BOOST_PP_REPEAT(10, INTEGRAL_WRAPPER_TEST, int);
	BOOST_PP_REPEAT(10, INTEGRAL_WRAPPER_TEST, long);
}

BOOST_AUTO_TEST_CASE(integral_1)
{
#undef WRAPPER
#define WRAPPER(T, i) char_<i>
	BOOST_PP_REPEAT(10, INTEGRAL_WRAPPER_TEST, char);
}

BOOST_AUTO_TEST_CASE(integral_2)
{
#undef WRAPPER
#define WRAPPER(T, i) int_<i>
	BOOST_PP_REPEAT(10, INTEGRAL_WRAPPER_TEST, int);
}

BOOST_AUTO_TEST_CASE(integral_3)
{
#undef WRAPPER
#define WRAPPER(T, i) size_t<i>
	BOOST_PP_REPEAT_FROM_TO(1, 11, INTEGRAL_WRAPPER_TEST, std::size_t);
}

}}}
