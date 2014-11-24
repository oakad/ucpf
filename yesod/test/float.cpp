/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#define BOOST_TEST_MODULE yesod
#include <boost/test/unit_test.hpp>

#include <yesod/float.hpp>

namespace ucpf { namespace yesod {
namespace test {

struct s0 {};

template <typename T>
float to_float32(T v)
{
	return float(v);
}

}

BOOST_AUTO_TEST_CASE(float_0)
{
	BOOST_CHECK(is_floating_point<float>::value);
	BOOST_CHECK(is_floating_point<double>::value);
	BOOST_CHECK(is_floating_point<float_t<32>>::value);
	BOOST_CHECK(is_floating_point<float_t<64>>::value);
	BOOST_CHECK(!is_floating_point<test::s0>::value);
	BOOST_CHECK((std::is_same<                                 \
		typename fp_adapter_type<float>::type, float_t<32> \
	>::value));
	BOOST_CHECK((std::is_same<                                  \
		typename fp_adapter_type<double>::type, float_t<64> \
	>::value));
	BOOST_CHECK((std::is_same<                       \
		typename fp_adapter_type<int>::type, int \
	>::value));
}

BOOST_AUTO_TEST_CASE(float_1)
{
	std::array<float8, 15> a0 = {{
		0x00, 0x80, 0x01, 0x02, 0x07,
		0x08, 0x09, 0x10, 0x11, 0x70, 0x71,
		0x76, 0x77, 0x78, 0xf8
	}};
	std::array<float, 15> a1 = {{
		0.0f, -0.0f, 0.001953125f, 0.00390625f, 0.013671875f,
		0.015625f, 0.017578125f, 0.03125f, 0.03515625f, 128.0f, 144.0f,
		224.0f, 240.0f,
		std::numeric_limits<float>::infinity(),
		-std::numeric_limits<float>::infinity()
	}};
	std::array<float, 15> a2;

	std::transform(
		a0.begin(), a0.end(), a2.begin(), test::to_float32<float8>
	);
	BOOST_CHECK(a1 == a2);
}

BOOST_AUTO_TEST_CASE(float_2)
{
	std::array<float16, 15> a0 = {{
		0x0000, 0x8000, 0x0001, 0x00f0,
		0x03ff, 0x0400,
		0x1800, 0x2c00, 0x4000, 0x5355, 0x66aa,
		0x7a00, 0x7bff , 0x7c00, 0xfc00
	}};
	std::array<float, 15> a1 = {{
		0.0f, -0.0f, 0.59604644775390625e-7f, 0.1430511474609375e-4f,
		0.60975551605224609375e-4f, 0.6103515625e-4f,
		0.001953125f, 0.0625f, 2.0f, 58.65625f, 1706.0f,
		49152.0f, 65504.0f,
		std::numeric_limits<float>::infinity(),
		-std::numeric_limits<float>::infinity()
	}};
	std::array<float, 15> a2;

	std::transform(
		a0.begin(), a0.end(), a2.begin(), test::to_float32<float16>
	);
	BOOST_CHECK(a1 == a2);
}

}}
