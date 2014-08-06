/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#include <cstring>
#include <iostream>
#include <algorithm>
#include <mina/detail/from_ascii_decimal_f.hpp>
#include "../test/float_generator.hpp"

#define CASE_COUNT 5000

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

#if 0
void test_float32()
{
	test::float_generator_r<32> fg_r;
	test::float_generator_e<32> fg_e;
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

#endif

#define XS 0
#define XM 1

void test_float64()
{
#if XS
	//char const *v = "2622520828.242490722767e-121";
	//char const *v = "776937685588284223.984378491277888300952";
	//char const *v = "603467951.059192303526e-239";
	//char const *v = "3218558146625396.392625539973627e-275";
	//char const *v = "5.1e-323";
	//char const *v = "5.053507028435470510603e+52";
	//-.025814278560944524802074798530038
	char const *v = "-9646109091079555";
	//+0471921094.28134313030e+207
	//+.81207619318252449741596744461043602986
	//+34.476159569225171
	//+3487479.067913e-211
	//-.35138377441158921712925843161842
	//-944795830346.32262411e-316
	char const *xv(v);

	detail::from_ascii_decimal_f<double> cv(
		xv, v + std::strlen(v), std::allocator<void>()
	);
	double rv(strtod(v, nullptr));
	printf("v (%d) %.40g, eq %d\n", cv.valid, cv.value, cv.value == rv);
	printf("rv    %.40g\n", rv);
#endif
	test::dec_float_generator<40, 325, 310> fg_r;
#if 0
	test::float_generator_e<64> fg_e;
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
#if XM
	printf("-- random\n");
	std::generate_n(test::null_sink(), CASE_COUNT, [&fg_r]() -> bool {
		return fg_r([](char *first, char *last) -> bool {
			printf("---- %s\n", first);
			auto xv(strtod(
				const_cast<char const *>(first), nullptr
			));
			detail::from_ascii_decimal_f<double> cv(
				first, last, std::allocator<void>()
			);
			CHECK_EQUAL(cv.value, xv);
			return cv.valid && (cv.value == xv);
		});
	});
#endif
#if 0
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
#endif
}

#if 0
void test_float128()
{
	test::float_generator_r<128> fg_r;
	test::float_generator_e<128> fg_e;
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
#endif

}}

int main()
{
	//printf("test float32\n");
	//ucpf::mina::test_float32();
	printf("test float64\n");
	ucpf::mina::test_float64();
	//printf("test float128\n");
	//ucpf::mina::test_float128();
	return 0;
}
