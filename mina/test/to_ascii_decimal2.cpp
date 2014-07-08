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

#define CASE_COUNT 1000

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

BOOST_AUTO_TEST_CASE(to_ascii_decimal2_0)
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

}}
