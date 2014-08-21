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
#include <mina/from_ascii_decimal.hpp>
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

#define XS 0
#define XM 1

void test_float32()
{
#if XM
	test::dec_float_generator<20, 45, 38> fg_r;

	printf("-- random\n");
	std::generate_n(test::null_sink(), CASE_COUNT, [&fg_r]() -> bool {
		return fg_r([](char *first, char *last) -> bool {
			auto x_first(first);
			auto xv(strtof(
				const_cast<char const *>(first), nullptr
			));
			detail::from_ascii_decimal_f<float> cv(
				x_first, last, std::allocator<void>()
			);
			printf("-fi- %s\n", first);
			printf("-fo- %.40g\n", cv.value);
			CHECK_EQUAL(cv.value, xv);
			return cv.valid && (cv.value == xv);
		});
	});
#endif
}

void test_float64()
{
#if XS

	{
		char const *v = "-.082738e-14";
		char const *xv(v);
		double zv;

		auto valid(from_ascii_decimal(xv, v + std::strlen(v), zv));

		double rv(strtod(v, nullptr));
		printf("v (%d) %.40g, eq %d\n", valid, zv, zv == rv);
		printf("rv    %.40g\n", rv);
	}
#endif

#if XM
	test::dec_float_generator<40, 325, 310> fg_r;

	printf("-- random\n");
	std::generate_n(test::null_sink(), CASE_COUNT, [&fg_r]() -> bool {
		return fg_r([](char *first, char *last) -> bool {
			auto x_first(first);
			auto xv(strtod(
				const_cast<char const *>(first), nullptr
			));
			detail::from_ascii_decimal_f<double> cv(
				x_first, last, std::allocator<void>()
			);
			printf("-di- %s\n", first);
			printf("-do- %.40g\n", cv.value);
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

void test_float128()
{
#if XS
	{
		char const *v = "7157015999103799151002151499469595.5";
		char const *xv(v);
		yesod::float128 zv;

		auto valid(from_ascii_decimal(xv, v + std::strlen(v), zv));

		yesod::float128 rv(strtoflt128(v, nullptr));
		printf("v (%d) %.40Qg, eq %d\n", valid, zv, zv == rv);
 		printf("rv    %.40Qg\n", rv);
	}
#endif
#if XM
	test::dec_float_generator<80, 4966, 4933> fg_r;

	printf("-- random\n");
	std::generate_n(test::null_sink(), CASE_COUNT, [&fg_r]() -> bool {
		return fg_r([](char *first, char *last) -> bool {
			auto x_first(first);
			auto xv(strtoflt128(
				const_cast<char const *>(first), nullptr
			));
			detail::from_ascii_decimal_f<yesod::float128> cv(
				x_first, last, std::allocator<void>()
			);
			printf("-qi- %s\n", first);
			printf("-qo- %.40Qg\n", cv.value);
			CHECK_EQUAL(cv.value, xv);
			return cv.valid && (cv.value == xv);
		});
	});

#endif
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
