/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#include <cstring>
#include <iostream>
#include <mina/to_ascii_decimal.hpp>
#include "../test/float_generator.hpp"

#define CASE_COUNT 50000

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
		quadmath_snprintf(str, sz + 1, "%.40Qg", x);
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
		if (!value)
			printf("Broken\n");
		return *this;
	}

	null_sink &operator=(bool &&value)
	{
		if (!value)
			printf("Broken\n");
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

#define CHECK_EQUAL(x, y) do {                                     \
	if (!(x == y))                                             \
		std::cout << "failed at " << __LINE__ << ": " << x \
			  << " not equal " << y << '\n';           \
} while(0)

#define CHECK_EQUAL_STR(x, y) do {                                 \
	if (strcmp(x, y))                                          \
		std::cout << "failed at " << __LINE__ << ": " << x \
			  << " not equal " << y << '\n';           \
} while(0)

void test_float32()
{
	test::float_generator_r<32> fg_r;
	test::float_generator_e<32> fg_e;
#if 0
	{
		char buf[40] = {0};
		char *ptr(buf);
		to_ascii_decimal(ptr, 0.0);
		CHECK_EQUAL_STR(buf, "+0.0");
	}
	{
		char buf[40] = {0};
		char *ptr(buf);
		to_ascii_decimal(ptr, std::numeric_limits<float>::infinity());
		CHECK_EQUAL_STR(buf, "+1.#inf");
	}
	{
		char buf[40] = {0};
		char *ptr(buf);
		to_ascii_decimal(ptr, std::numeric_limits<float>::quiet_NaN());
		CHECK_EQUAL_STR(buf, "+1.#q(0)");
	}
	{
		char buf[40] = {0};
		char *ptr(buf);
		to_ascii_decimal(
			ptr, std::numeric_limits<float>::signaling_NaN()
		);
		CHECK_EQUAL_STR(buf, "+1.#s(2097152)");
	}
#endif
	printf("-- random\n");
	std::generate_n(test::null_sink(), CASE_COUNT, [&fg_r]() -> bool {
		return fg_r([](float v) -> bool {
			char buf[40] = {0};
			char *ptr(buf);
			to_ascii_decimal(ptr, v);
			auto xv(strtof(buf, nullptr));
			CHECK_EQUAL(v, xv);
			return v == xv;
		});
	});
	printf("-- exponent\n");
	std::generate_n(test::null_sink(), CASE_COUNT, [&fg_e]() -> bool {
		return fg_e([](float v) -> bool {
			char buf[40] = {0};
			char *ptr(buf);
			to_ascii_decimal(ptr, v);
			auto xv(strtof(buf, nullptr));
			CHECK_EQUAL(v, xv);
			return v == xv;
		});
	});
}

void test_float64()
{
	test::float_generator_r<64> fg_r;
	test::float_generator_e<64> fg_e;
#if 0
	{
		char buf[40] = {0};
		char *ptr(buf);
		to_ascii_decimal(ptr, 0.0);
		CHECK_EQUAL_STR(buf, "+0.0");
	}
	{
		char buf[40] = {0};
		char *ptr(buf);
		to_ascii_decimal(ptr, std::numeric_limits<double>::infinity());
		CHECK_EQUAL_STR(buf, "+1.#inf");
	}
	{
		char buf[40] = {0};
		char *ptr(buf);
		to_ascii_decimal(ptr, std::numeric_limits<double>::quiet_NaN());
		CHECK_EQUAL_STR(buf, "+1.#q(0)");
	}
	{
		char buf[40] = {0};
		char *ptr(buf);
		to_ascii_decimal(
			ptr, std::numeric_limits<double>::signaling_NaN()
		);
		CHECK_EQUAL_STR(buf, "+1.#s(1125899906842624)");
	}
#endif
	printf("-- random\n");
	std::generate_n(test::null_sink(), CASE_COUNT, [&fg_r]() -> bool {
		return fg_r([](double v) -> bool {
			char buf[80] = {0};
			char *ptr(buf);
			to_ascii_decimal(ptr, v);
			auto xv(strtod(buf, nullptr));
			CHECK_EQUAL(v, xv);
			return v == xv;
		});
	});
	printf("-- exponent\n");
	std::generate_n(test::null_sink(), CASE_COUNT, [&fg_e]() -> bool {
		return fg_e([](double v) -> bool {
			char buf[40] = {0};
			char *ptr(buf);
			to_ascii_decimal(ptr, v);
			auto xv(strtod(buf, nullptr));
			CHECK_EQUAL(v, xv);
			return v == xv;
		});
	});
}

void test_float128()
{
	test::float_generator_r<128> fg_r;
	test::float_generator_e<128> fg_e;
#if 0
	{
		char buf[40] = {0};
		char *ptr(buf);
		to_ascii_decimal(ptr, 0.0Q);
		CHECK_EQUAL_STR(buf, "+0.0");
	}
	{
		char buf[40] = {0};
		char *ptr(buf);
		to_ascii_decimal(
			ptr, std::numeric_limits<yesod::float128>::infinity()
		);
		CHECK_EQUAL_STR(buf, "+1.#inf");
	}
	{
		char buf[40] = {0};
		char *ptr(buf);
		to_ascii_decimal(ptr, std::numeric_limits<
			yesod::float128
		>::quiet_NaN());
		CHECK_EQUAL_STR(buf, "+1.#q(0)");
	}
	{
		char buf[50] = {0};
		char *ptr(buf);
		to_ascii_decimal(ptr, std::numeric_limits<
			yesod::float128
		>::signaling_NaN());
		CHECK_EQUAL_STR(
			buf, "+1.#s(1298074214633706907132624082305024)"
		);
	}
#endif
	printf("-- random\n");
	std::generate_n(test::null_sink(), CASE_COUNT, [&fg_r]() -> bool {
		return fg_r([](yesod::float128 v) -> bool {
			char buf[80] = {0};
			char *ptr(buf);
			to_ascii_decimal(ptr, v);
			auto xv(strtoflt128(buf, nullptr));
			CHECK_EQUAL(v, xv);
			return v == xv;
		});
	});

	printf("-- exponent\n");
	std::generate_n(test::null_sink(), CASE_COUNT, [&fg_e]() -> bool {
		return fg_e([](yesod::float128 v) -> bool {
			char buf[80] = {0};
			char *ptr(buf);
			to_ascii_decimal(ptr, v);
			auto xv(strtoflt128(buf, nullptr));
			CHECK_EQUAL(v, xv);
			return v == xv;
		});
	});

}

}}

int main()
{
	printf("test float32\n");
	ucpf::mina::test_float32();
	printf("test float64\n");
	ucpf::mina::test_float64();
	printf("test float128\n");
	ucpf::mina::test_float128();
	return 0;
}
