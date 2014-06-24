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

namespace ucpf { namespace mina {

BOOST_AUTO_TEST_CASE(to_ascii_decimal_0)
{
	std::array<uint8_t, 9> n0{
		0, 7, 24, 91, 113, 139, 200, 243, 255
	};
	std::string s0("0, 7, 24, 91, 113, 139, 200, 243, 255");
	std::string ref;
	auto iter0(n0.begin());
	auto sink(std::back_inserter(ref));

	while (true) {
		to_ascii_decimal(sink, *iter0);
		++iter0;
		if (iter0 == n0.end())
			break;
		*sink++ = ',';
		*sink++ = ' ';
	};

	BOOST_CHECK_EQUAL(ref, s0);
	ref.clear();

	std::array<int8_t, 9> n1{
		-128, -101, -53, -5, 0, 9, 61, 111, 127
	};
	std::string s1("-128, -101, -53, -5, +0, +9, +61, +111, +127");
	auto iter1(n1.begin());

	while (true) {
		to_ascii_decimal(sink, *iter1);
		++iter1;
		if (iter1 == n1.end())
			break;
		*sink++ = ',';
		*sink++ = ' ';
	};
	BOOST_CHECK_EQUAL(ref, s1);
}

BOOST_AUTO_TEST_CASE(to_ascii_decimal_1)
{
	std::array<uint16_t, 9> n0{
		0, 7, 24, 910, 1130, 14739, 27485, 43204, 65535
	};
	std::string s0("0, 7, 24, 910, 1130, 14739, 27485, 43204, 65535");
	std::string ref;
	auto iter0(n0.begin());
	auto sink(std::back_inserter(ref));

	while (true) {
		to_ascii_decimal(sink, *iter0);
		++iter0;
		if (iter0 == n0.end())
			break;
		*sink++ = ',';
		*sink++ = ' ';
	};

	BOOST_CHECK_EQUAL(ref, s0);
	ref.clear();

	std::array<int16_t, 9> n1{
		-32768, -19157, -5743, -55, 0, 945, 6132, 21111, 32767
	};
	std::string s1(
		"-32768, -19157, -5743, -55, +0, +945, +6132, +21111, +32767"
	);
	auto iter1(n1.begin());

	while (true) {
		to_ascii_decimal(sink, *iter1);
		++iter1;
		if (iter1 == n1.end())
			break;
		*sink++ = ',';
		*sink++ = ' ';
	};
	BOOST_CHECK_EQUAL(ref, s1);
}

BOOST_AUTO_TEST_CASE(to_ascii_decimal_2)
{
	std::array<uint32_t, 9> n0{
		0, 76, 243, 9100, 63192, 6399192, 576495799, 2104957993,
		4294967295
	};
	std::string s0(
		"0, 76, 243, 9100, 63192, 6399192, 576495799, "
		"2104957993, 4294967295"
	);
	std::string ref;
	auto iter0(n0.begin());
	auto sink(std::back_inserter(ref));

	while (true) {
		to_ascii_decimal(sink, *iter0);
		++iter0;
		if (iter0 == n0.end())
			break;
		*sink++ = ',';
		*sink++ = ' ';
	};

	BOOST_CHECK_EQUAL(ref, s0);
	ref.clear();

	std::array<int32_t, 9> n1{
		-2147483648, -191575439, -5744323, -5542, 0, 9445, 614332,
		111105854, 2147483647
	};
	std::string s1(
		"-2147483648, -191575439, -5744323, -5542, +0, +9445, "
		"+614332, +111105854, +2147483647"
	);
	auto iter1(n1.begin());

	while (true) {
		to_ascii_decimal(sink, *iter1);
		++iter1;
		if (iter1 == n1.end())
			break;
		*sink++ = ',';
		*sink++ = ' ';
	};
	BOOST_CHECK_EQUAL(ref, s1);
}

}}
