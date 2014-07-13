/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#define BOOST_TEST_MODULE mina
#include <boost/test/included/unit_test.hpp>

#include <string>
#include <mina/to_ascii_decimal.hpp>
#include "float_generator.hpp"

#define CASE_COUNT 100000

namespace std {

template <typename CharType, typename TraitsType>
std::basic_ostream<CharType, TraitsType> &operator<<(
	std::basic_ostream<CharType, TraitsType> &os, ucpf::yesod::float128 x
)
{
	auto sz(quadmath_snprintf(nullptr, 0, "%Qg", x));
	if (sz > 0) {
		char str[sz + 1];
		str[sz] = 0;
		quadmath_snprintf(str, sz + 1, "%Qg", x);
		for (decltype(sz) c(0); c <= sz; ++c)
			os << os.widen(str[c]);
	}
	return os;
}

}

namespace ucpf { namespace mina {
namespace test {

struct null_sink {
	null_sink &operator=(bool const &value)
	{
		BOOST_CHECK(value);
		return *this;
	}

	null_sink &operator=(bool &&value)
	{
		BOOST_CHECK(value);
		return *this;
	}

	null_sink &operator*()
	{
		return *this;
	}

	null_sink &operator++()
	{
		return *this;
	}

	null_sink &operator++(int)
	{
		return *this;
	}

};

}

BOOST_AUTO_TEST_CASE(to_ascii_decimal2_2)
{
	test::float_generator<32> fg;
	{
		char buf[40] = {0};
		char *ptr(buf);
		to_ascii_decimal(ptr, 0.0);
		BOOST_CHECK_EQUAL(buf, "+0.0");
	}
	{
		char buf[40] = {0};
		char *ptr(buf);
		to_ascii_decimal(ptr, std::numeric_limits<float>::infinity());
		BOOST_CHECK_EQUAL(buf, "+1.#inf");
	}
	{
		char buf[40] = {0};
		char *ptr(buf);
		to_ascii_decimal(ptr, std::numeric_limits<float>::quiet_NaN());
		BOOST_CHECK_EQUAL(buf, "+1.#q(0)");
	}
	{
		char buf[40] = {0};
		char *ptr(buf);
		to_ascii_decimal(
			ptr, std::numeric_limits<float>::signaling_NaN()
		);
		BOOST_CHECK_EQUAL(buf, "+1.#s(2097152)");
	}

	std::generate_n(test::null_sink(), CASE_COUNT, [&fg]() -> bool {
		return fg([](float v) -> bool {
			char buf[40] = {0};
			char *ptr(buf);
			to_ascii_decimal(ptr, v);
			auto xv(strtof(buf, nullptr));
			BOOST_CHECK_EQUAL(v, xv);
			return v == xv;
		});
	});
}

BOOST_AUTO_TEST_CASE(to_ascii_decimal2_3)
{
	test::float_generator<64> fg;
	{
		char buf[40] = {0};
		char *ptr(buf);
		to_ascii_decimal(ptr, 0.0);
		BOOST_CHECK_EQUAL(buf, "+0.0");
	}
	{
		char buf[40] = {0};
		char *ptr(buf);
		to_ascii_decimal(ptr, std::numeric_limits<double>::infinity());
		BOOST_CHECK_EQUAL(buf, "+1.#inf");
	}
	{
		char buf[40] = {0};
		char *ptr(buf);
		to_ascii_decimal(ptr, std::numeric_limits<double>::quiet_NaN());
		BOOST_CHECK_EQUAL(buf, "+1.#q(0)");
	}
	{
		char buf[40] = {0};
		char *ptr(buf);
		to_ascii_decimal(
			ptr, std::numeric_limits<double>::signaling_NaN()
		);
		BOOST_CHECK_EQUAL(buf, "+1.#s(1125899906842624)");
	}

	std::generate_n(test::null_sink(), CASE_COUNT, [&fg]() -> bool {
		return fg([](double v) -> bool {
			char buf[40] = {0};
			char *ptr(buf);
			to_ascii_decimal(ptr, v);
			auto xv(strtod(buf, nullptr));
			BOOST_CHECK_EQUAL(v, xv);
			return v == xv;
		});
	});
}
#if 0
BOOST_AUTO_TEST_CASE(to_ascii_decimal2_4)
{
	test::float_generator<128> fg;
	{
		char buf[40] = {0};
		char *ptr(buf);
		to_ascii_decimal(ptr, 0.0Q);
		BOOST_CHECK_EQUAL(buf, "+0.0");
	}
	{
		char buf[40] = {0};
		char *ptr(buf);
		to_ascii_decimal(
			ptr, std::numeric_limits<yesod::float128>::infinity()
		);
		BOOST_CHECK_EQUAL(buf, "+1.#inf");
	}
	{
		char buf[40] = {0};
		char *ptr(buf);
		to_ascii_decimal(ptr, std::numeric_limits<
			yesod::float128
		>::quiet_NaN());
		BOOST_CHECK_EQUAL(buf, "+1.#q(0)");
	}
	{
		char buf[50] = {0};
		char *ptr(buf);
		to_ascii_decimal(ptr, std::numeric_limits<
			yesod::float128
		>::signaling_NaN());
		BOOST_CHECK_EQUAL(
			buf, "+1.#s(1298074214633706907132624082305024)"
		);
	}

	std::generate_n(test::null_sink(), CASE_COUNT, [&fg]() -> bool {
		return fg([](yesod::float128 v) -> bool {
			char buf[80] = {0};
			char *ptr(buf);
			to_ascii_decimal(ptr, v);
			auto xv(strtoflt128(buf, nullptr));
			BOOST_CHECK_EQUAL(v, xv);
			return v == xv;
		});
	});
}
#endif
}}
