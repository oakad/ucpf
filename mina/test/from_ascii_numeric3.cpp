/*
 * Copyright (c) 2015 Alex Dubov <oakad@yahoo.com>
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

//#define CASE_COUNT 1000000
#define CASE_COUNT 1000

namespace std {

template <typename CharType, typename TraitsType>
std::basic_ostream<CharType, TraitsType> &operator<<(
	std::basic_ostream<CharType, TraitsType> &os, ucpf::yesod::float128 x
)
{
	auto sz(quadmath_snprintf(nullptr, 0, "%.40Qa", x));
	if (sz > 0) {
		char str[sz + 1];
		str[sz] = 0;
		quadmath_snprintf(str, sz + 1, "%.40Qa", x);
		for (decltype(sz) c(0); c <= sz; ++c)
			os << os.widen(str[c]);
	}
	return os;
}

}

namespace ucpf { namespace mina {

BOOST_AUTO_TEST_CASE(from_ascii_numeric3_2)
{
	{
		char const *first = "+0x0.0";
		char const *last = first + strlen(first);
		float v;
		BOOST_CHECK(from_ascii_numeric(v, first, last));
		BOOST_CHECK_EQUAL(v, 0.0);
	}
	{
		char const *first = "-0x0p34";
		char const *last = first + strlen(first);
		float v;
		BOOST_CHECK(from_ascii_numeric(v, first, last));
		BOOST_CHECK_EQUAL(v, 0.0);
	}
	{
		char const *first = "0x234p-160";
		char const *last = first + strlen(first);
		float v;
		BOOST_CHECK(from_ascii_numeric(v, first, last));
		BOOST_CHECK_EQUAL(v, 0.0);
	}
	{
		char const *first = "0x123p160";
		char const *last = first + strlen(first);
		float v;
		BOOST_CHECK(from_ascii_numeric(v, first, last));
		BOOST_CHECK_EQUAL(v, std::numeric_limits<float>::infinity());
	}

	test::hex_float_generator<20, 151, 127> fg_r;

	for (int c(0); c < CASE_COUNT; ++c) {
		fg_r([](char *first, char *last) -> bool {
			auto xv(strtof(
				const_cast<char const *>(first), nullptr
			));
			printf("-xv- %s\n", first);
			float v;
			BOOST_CHECK(from_ascii_numeric(v, first, last));
			BOOST_WARN_EQUAL(v, xv);
			if (v != xv) {
				if (v >= 0)
					v -= std::numeric_limits<
						float
					>::denorm_min();
				else
					v += std::numeric_limits<
						float
					>::denorm_min();

				BOOST_CHECK_EQUAL(v, xv);
			}

			return true;
		});
	};
}
#if 0
BOOST_AUTO_TEST_CASE(from_ascii_numeric3_3)
{
	{
		char const *first = "+0x0.0";
		char const *last = first + strlen(first);
		double v;
		BOOST_CHECK(from_ascii_numeric(v, first, last));
		BOOST_CHECK_EQUAL(v, 0.0);
	}
	{
		char const *first = "-0xp34";
		char const *last = first + strlen(first);
		double v;
		BOOST_CHECK(from_ascii_numeric(v, first, last));
		BOOST_CHECK_EQUAL(v, 0.0);
	}
	{
		char const *first = "0x234p-1340";
		char const *last = first + strlen(first);
		double v;
		BOOST_CHECK(from_ascii_numeric(v, first, last));
		BOOST_CHECK_EQUAL(v, 0.0);
	}
	{
		char const *first = "0x123p1340";
		char const *last = first + strlen(first);
		double v;
		BOOST_CHECK(from_ascii_numeric(v, first, last));
		BOOST_CHECK_EQUAL(v, std::numeric_limits<double>::infinity());
	}

	test::hex_float_generator<40, 1079, 1023> fg_r;

	for (int c(0); c < CASE_COUNT; ++c) {
		fg_r([](char *first, char *last) -> bool {
			auto xv(strtod(
				const_cast<char const *>(first), nullptr
			));
			double v;
			BOOST_CHECK(from_ascii_numeric(v, first, last));
			BOOST_WARN_EQUAL(v, xv);
			if (v != xv) {
				if (v >= 0)
					v -= std::numeric_limits<
						double
					>::denorm_min();
				else
					v += std::numeric_limits<
						double
					>::denorm_min();

				BOOST_CHECK_EQUAL(v, xv);
			}

			return true;
		});
	};
}

BOOST_AUTO_TEST_CASE(from_ascii_numeric3_4)
{
	{
		char const *first = "+0x0.0";
		char const *last = first + strlen(first);
		yesod::float128 v;
		BOOST_CHECK(from_ascii_numeric(v, first, last));
		BOOST_CHECK_EQUAL(v, 0.0);
	}
	{
		char const *first = "-0x0p34";
		char const *last = first + strlen(first);
		yesod::float128 v;
		BOOST_CHECK(from_ascii_numeric(v, first, last));
		BOOST_CHECK_EQUAL(v, 0.0);
	}
	{
		char const *first = "0x234p-20100";
		char const *last = first + strlen(first);
		yesod::float128 v;
		BOOST_CHECK(from_ascii_numeric(v, first, last));
		BOOST_CHECK_EQUAL(v, 0.0);
	}
	{
		char const *first = "0x123p2050";
		char const *last = first + strlen(first);
		yesod::float128 v;
		BOOST_CHECK(from_ascii_numeric(v, first, last));
		BOOST_CHECK_EQUAL(
			v, std::numeric_limits<yesod::float128>::infinity()
		);
	}

	test::hex_float_generator<80, 16480, 16383> fg_r;

	for (int c(0); c < CASE_COUNT; ++c) {
		fg_r([](char *first, char *last) -> bool {
			auto xv(strtoflt128(
				const_cast<char const *>(first), nullptr
			));
			yesod::float128 v;
			BOOST_CHECK(from_ascii_numeric(v, first, last));
			BOOST_WARN_EQUAL(v, xv);
			if (v != xv) {
				if (v >= 0)
					v -= std::numeric_limits<
						yesod::float128
					>::denorm_min();
				else
					v += std::numeric_limits<
						yesod::float128
					>::denorm_min();

				BOOST_CHECK_EQUAL(v, xv);
			}

			return true;
		});
	};
}
#endif
}}
