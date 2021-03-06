/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#define BOOST_TEST_MODULE mina
#include <boost/test/unit_test.hpp>

#include <string>
#include <mina/from_ascii_numeric.hpp>
#include "float_generator.hpp"

#define CASE_COUNT 1000000

namespace std {

template <typename CharType, typename TraitsType>
std::basic_ostream<CharType, TraitsType> &operator<<(
	std::basic_ostream<CharType, TraitsType> &os, ucpf::yesod::float128 x
)
{
	auto sz(quadmath_snprintf(nullptr, 0, "%.40Qg", x));
	if (sz > 0) {
		char str[sz + 1];
		str[sz] = 0;
		quadmath_snprintf(str, sz, "%.40Qg", x);
		for (decltype(sz) c(0); str[c] && (c < sz); ++c)
			os << os.widen(str[c]);
	}
	return os;
}

}

namespace ucpf { namespace mina {

BOOST_AUTO_TEST_CASE(from_ascii_numeric2_2)
{
	{
		char const *first = "+0.0";
		char const *last = first + strlen(first);
		float v;
		BOOST_CHECK(from_ascii_numeric(v, first, last));
		BOOST_CHECK_EQUAL(v, 0.0);
	}
	{
		char const *first = "-0e34";
		char const *last = first + strlen(first);
		float v;
		BOOST_CHECK(from_ascii_numeric(v, first, last));
		BOOST_CHECK_EQUAL(v, 0.0);
	}
	{
		char const *first = "234e-60";
		char const *last = first + strlen(first);
		float v;
		BOOST_CHECK(from_ascii_numeric(v, first, last));
		BOOST_CHECK_EQUAL(v, 0.0);
	}
	{
		char const *first = "123e60";
		char const *last = first + strlen(first);
		float v;
		BOOST_CHECK(from_ascii_numeric(v, first, last));
		BOOST_CHECK_EQUAL(v, std::numeric_limits<float>::infinity());
	}

	test::dec_float_generator<20, 45, 38> fg_r;

	for (int c(0); c < CASE_COUNT; ++c) {
		fg_r([](char *first, char *last) -> bool {
			auto xv(strtof(
				const_cast<char const *>(first), nullptr
			));
			float v;
			BOOST_CHECK(from_ascii_numeric(v, first, last));
			BOOST_CHECK_EQUAL(v, xv);
			return true;
		});
	};
}

BOOST_AUTO_TEST_CASE(from_ascii_numeric2_3)
{
	{
		char const *first = "+0.0";
		char const *last = first + strlen(first);
		double v;
		BOOST_CHECK(from_ascii_numeric(v, first, last));
		BOOST_CHECK_EQUAL(v, 0.0);
	}
	{
		char const *first = "-0e34";
		char const *last = first + strlen(first);
		double v;
		BOOST_CHECK(from_ascii_numeric(v, first, last));
		BOOST_CHECK_EQUAL(v, 0.0);
	}
	{
		char const *first = "234e-340";
		char const *last = first + strlen(first);
		double v;
		BOOST_CHECK(from_ascii_numeric(v, first, last));
		BOOST_CHECK_EQUAL(v, 0.0);
	}
	{
		char const *first = "123e340";
		char const *last = first + strlen(first);
		double v;
		BOOST_CHECK(from_ascii_numeric(v, first, last));
		BOOST_CHECK_EQUAL(v, std::numeric_limits<double>::infinity());
	}

	test::dec_float_generator<40, 325, 310> fg_r;

	for (int c(0); c < CASE_COUNT; ++c) {
		fg_r([](char *first, char *last) -> bool {
			auto xv(strtod(
				const_cast<char const *>(first), nullptr
			));
			double v;
			BOOST_CHECK(from_ascii_numeric(v, first, last));
			BOOST_CHECK_EQUAL(v, xv);
			return true;
		});
	};
}

BOOST_AUTO_TEST_CASE(from_ascii_numeric2_4)
{
	{
		char const *first = "+0.0";
		char const *last = first + strlen(first);
		yesod::float128 v;
		BOOST_CHECK(from_ascii_numeric(v, first, last));
		BOOST_CHECK_EQUAL(v, 0.0);
	}
	{
		char const *first = "-0e34";
		char const *last = first + strlen(first);
		yesod::float128 v;
		BOOST_CHECK(from_ascii_numeric(v, first, last));
		BOOST_CHECK_EQUAL(v, 0.0);
	}
	{
		char const *first = "234e-5100";
		char const *last = first + strlen(first);
		yesod::float128 v;
		BOOST_CHECK(from_ascii_numeric(v, first, last));
		BOOST_CHECK_EQUAL(v, 0.0);
	}
	{
		char const *first = "123e5050";
		char const *last = first + strlen(first);
		yesod::float128 v;
		BOOST_CHECK(from_ascii_numeric(v, first, last));
		BOOST_CHECK_EQUAL(
			v, std::numeric_limits<yesod::float128>::infinity()
		);
	}

	test::dec_float_generator<80, 4966, 4933> fg_r;

	for (int c(0); c < CASE_COUNT; ++c) {
		fg_r([](char *first, char *last) -> bool {
			auto xv(strtoflt128(
				const_cast<char const *>(first), nullptr
			));
			yesod::float128 v;
			BOOST_CHECK(from_ascii_numeric(v, first, last));
			BOOST_CHECK_EQUAL(v, xv);
			return true;
		});
	};
}

}}
