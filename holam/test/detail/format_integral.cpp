/*
 * Copyright (c) 2014-2016 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */


#define BOOST_TEST_MODULE holam
#include <boost/test/unit_test.hpp>

#include <holam/detail/format_integral.hpp>

#define CASE_COUNT 1000000

namespace std {

template <typename CharType, typename TraitsType>
std::basic_ostream<CharType, TraitsType> &operator<<(
        std::basic_ostream<CharType, TraitsType> &os, uint128_t const &val
)
{
	typedef ucpf::holam::detail::bcd_converter<uint128_t> cnv_type;
	uint8_t bcd_value[cnv_type::bcd_value_size];

	cnv_type::to_bcd(bcd_value, val);
	auto m_size(ucpf::holam::detail::bcd_count_digits(
		bcd_value, cnv_type::bcd_value_size
	));
	auto pos(2 * cnv_type::bcd_value_size - m_size);

	if (pos & 1) {
		os << os.widen(0x30 | (bcd_value[pos >> 1] & 0xf));
		++pos;
		--m_size;
	}

	while (m_size >= 2) {
		os << os.widen(0x30 | (bcd_value[pos >> 1] >> 4));
		os << os.widen(0x30 | (bcd_value[pos >> 1] & 0xf));
		pos -= 2;
		m_size -= 2;
	}

	if (m_size)
		os << os.widen(0x30 | (bcd_value[pos >> 1] >> 4));

        return os;
}

}

namespace ucpf { namespace holam { namespace detail {
namespace test {

template <typename T>
T from_bcd(uint8_t *in, std::size_t sz)
{
	T rv(0);
	for (std::size_t c(0); c < sz; ++c) {
		rv *= 10;
		rv += in[c] >> 4;
		rv *= 10;
		rv += in[c] & 0xf;
	}
	return rv;
}

}

BOOST_AUTO_TEST_CASE(format_integral_0)
{
	typedef bcd_converter<uint8_t> cnv_type;
	uint8_t bcd_value[cnv_type::bcd_value_size];

	for (uint16_t c(0); c < (uint8_t(1) << 8); ++c) {
		cnv_type::to_bcd(bcd_value, uint8_t(c));
		auto out(test::from_bcd<uint8_t>(
			bcd_value, cnv_type::bcd_value_size
		));
		BOOST_REQUIRE_EQUAL(out, uint8_t(c));
	}
}

BOOST_AUTO_TEST_CASE(format_integral_1)
{
	typedef bcd_converter<uint16_t> cnv_type;
	uint8_t bcd_value[cnv_type::bcd_value_size];

	for (uint32_t c(0); c < (uint32_t(1) << 16); ++c) {
		cnv_type::to_bcd(bcd_value, uint16_t(c));
		auto out(test::from_bcd<uint16_t>(
			bcd_value, cnv_type::bcd_value_size
		));
		BOOST_REQUIRE_EQUAL(out, uint16_t(c));
	}
}

BOOST_AUTO_TEST_CASE(format_integral_2)
{
	typedef bcd_converter<uint32_t> cnv_type;
	uint8_t bcd_value[cnv_type::bcd_value_size];

	std::random_device dev;
	std::uniform_int_distribution<uint32_t> dist;

	for (auto c(0); c < CASE_COUNT; ++c) {
		auto v_in(dist(dev));
		cnv_type::to_bcd(bcd_value, v_in);
		auto v_out(test::from_bcd<uint32_t>(
			bcd_value, cnv_type::bcd_value_size
		));
		BOOST_REQUIRE_EQUAL(v_out, v_in);
	}
}

BOOST_AUTO_TEST_CASE(format_integral_3)
{
	typedef bcd_converter<uint64_t> cnv_type;
	uint8_t bcd_value[cnv_type::bcd_value_size];

	std::random_device dev;
	std::uniform_int_distribution<uint64_t> dist;

	for (auto c(0); c < CASE_COUNT; ++c) {
		auto v_in(dist(dev));
		cnv_type::to_bcd(bcd_value, v_in);
		auto v_out(test::from_bcd<uint64_t>(
			bcd_value, cnv_type::bcd_value_size
		));
		BOOST_REQUIRE_EQUAL(v_out, v_in);
	}
}

BOOST_AUTO_TEST_CASE(format_integral_4)
{
	typedef bcd_converter<uint128_t> cnv_type;
	uint8_t bcd_value[cnv_type::bcd_value_size];

	std::random_device dev;
	std::uniform_int_distribution<uint64_t> dist;

	for (auto c(0); c < CASE_COUNT; ++c) {
		uint128_t v_in(dist(dev));
		v_in <<= 64;
		v_in |= dist(dev);
		cnv_type::to_bcd(bcd_value, v_in);
		auto v_out(test::from_bcd<uint128_t>(
			bcd_value, cnv_type::bcd_value_size
		));
		BOOST_REQUIRE_EQUAL(v_out, v_in);
	}
}

}}}
