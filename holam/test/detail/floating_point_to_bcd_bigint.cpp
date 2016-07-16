/*
 * Copyright (c) 2016 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */


#define BOOST_TEST_MODULE holam
#include <boost/test/unit_test.hpp>

#include <holam/detail/floating_point_to_bcd_bigint.hpp>
#include "float_generator.hpp"

#define CASE_COUNT 100000
namespace ucpf { namespace holam { namespace detail {
namespace test {

template <typename T>
void verify(T v)
{
	uint8_t bcd_val[fp_value_traits<T>::bcd_storage_size];
	char c_val[fp_value_traits<T>::bcd_storage_size * 2 + 10] = {0};
	std::size_t val_len;
	int32_t exp10;
	fp_value_t<T> vv(v);

	floating_point_to_bcd_bigint<T>::apply(
		bcd_val, val_len, exp10, vv
	);
	c_val[0] = std::signbit(v) ? '-' : '+';
	expand_bcd(c_val + 1, bcd_val, val_len);
	val_len++;
	c_val[val_len++] = 'e';
	sprintf(c_val + val_len, "%d", exp10);
	T other(read_float<T>::apply(c_val));
	BOOST_REQUIRE_EQUAL(v, other);
}

}

BOOST_AUTO_TEST_CASE(floating_point_to_bcd_bigint_2)
{
	{
		test::float_generator_z<float> gen_0;
		gen_0(test::verify<float>);
	}
	{
		test::float_generator_e<float> gen_1;
		for (auto c(0); c < CASE_COUNT; ++c)
			gen_1(test::verify<float>);
	}
	{
		test::float_generator_r<float> gen_2;
		for (auto c(0); c < CASE_COUNT; ++c)
			gen_2(test::verify<float>);
	}
}

BOOST_AUTO_TEST_CASE(floating_point_to_bcd_bigint_3)
{
	{
		test::float_generator_z<double> gen_0;
		gen_0(test::verify<double>);
	}
	{
		test::float_generator_e<double> gen_1;
		for (auto c(0); c < CASE_COUNT; ++c)
			gen_1(test::verify<double>);
	}
	{
		test::float_generator_r<double> gen_2;
		for (auto c(0); c < CASE_COUNT; ++c)
			gen_2(test::verify<double>);
	}
}

BOOST_AUTO_TEST_CASE(floating_point_to_bcd_bigint_4)
{
	{
		test::float_generator_z<float128> gen_0;
		gen_0(test::verify<float128>);
	}
	{
		test::float_generator_e<float128> gen_1;
		for (auto c(0); c < CASE_COUNT; ++c)
			gen_1(test::verify<float128>);
	}
	{
		test::float_generator_r<float128> gen_2;
		for (auto c(0); c < CASE_COUNT; ++c)
			gen_2(test::verify<float128>);
	}
}

}}}
