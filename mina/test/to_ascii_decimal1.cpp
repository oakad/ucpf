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

BOOST_AUTO_TEST_CASE(to_ascii_decimal1_0)
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

BOOST_AUTO_TEST_CASE(to_ascii_decimal1_1)
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

BOOST_AUTO_TEST_CASE(to_ascii_decimal1_2)
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

BOOST_AUTO_TEST_CASE(to_ascii_decimal1_3)
{
	std::array<uint64_t, 9> n0{
		0, 709, 7709, 77095, 77095615, 4670770956ull, 467077095615ull,
		18446744077095615ull, 18446744073709551615ull
	};
	std::string s0(
		"0, 709, 7709, 77095, 77095615, 4670770956, 467077095615, "
		"18446744077095615, 18446744073709551615"
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

	std::array<int64_t, 9> n1{
		-9223372036854775808ll, -92233720854778ll, -92230368808ll,
		-37203685ll, 0, 23368557ll, 72036854757ll, 72036854775807ll,
		9223372036854775807ll
	};
	std::string s1(
		"-9223372036854775808, -92233720854778, -92230368808, "
		"-37203685, +0, +23368557, +72036854757, +72036854775807, "
		"+9223372036854775807"
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

BOOST_AUTO_TEST_CASE(to_ascii_decimal1_4)
{
	std::array<unsigned __int128, 9> n0{
		0, 709, 7709, 77095, 77095615, 4670770956ull, 467077095615ull,
		18446744077095615ull, 18446744073709551615ull
	};
	std::for_each(n0.begin(), n0.end(), [](unsigned __int128 &v) {
		v = (v << 64) + v;
	});
	std::string s0(
		"0, 13078741548260072096453, 142205950064226933415453, "
		"1422151734362637881912615, 1422163079110243213286859455, "
		"86160516452247696872466435852, "
		"8616051645501470848352286859455, "
		"340282366983400308143828261286859455, "
		"340282366920938463463374607431768211455"
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

	std::array<__int128, 9> n1{
		-9223372036854775808ll, -92233720854778ll, -92230368808ll,
		-37203685ll, 0, 23368557ll, 72036854757ll, 72036854775807ll,
		9223372036854775807ll
	};
	std::for_each(n1.begin(), n1.end(), [](__int128 &v) {
		v = v << 64;
	});
	std::string s1(
		"-170141183460469231731687303715884105728, "
		"-1701411843574057150912994631221248, "
		"-1701350009225020282215992393728, "
		"-686286855793906939812904960, "
		"+0, "
		"+431073790350893858382938112, "
		"+1328845423577365471965386637312, "
		"+1328845423924293387759642174554112, "
		"+170141183460469231713240559642174554112"
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
