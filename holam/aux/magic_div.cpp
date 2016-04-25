/*=============================================================================
    Copyright (c) 2013      Alex Dubov <oakad@yahoo.com>

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

    Based on algorithm from section 10-10 of Hacker's Delight by H. Warren to
    produce multiplicative coefficients for constant division (as used in
    section 10-8 thereof).
==============================================================================*/

#include <tuple>
#include <limits>
#include <cstdio>
#include <cstdint>

template <typename T>
std::tuple<T, int, bool> generate_mul(T d)
{
	constexpr static T base_ff(std::numeric_limits<T>::max());
	constexpr static T base_7f(base_ff >> 1);
	constexpr static T base_80(base_7f + 1);

	bool adj(false);
	auto p(std::numeric_limits<T>::digits - 1);
	T delta, nc(base_ff);

	nc -= ((~d) + 1) % d;

	T q1(base_80 / nc);
	T q2(base_7f / d);

	T r1(base_80 - (q1 * nc));
	T r2(base_7f - (q2 * d));

	do {
		++p;

		if (r1 >= (nc - r1)) {
			q1 = (q1 << 1) + 1;
			r1 = (r1 << 1) - nc;
		} else {
			q1 <<= 1;
			r1 <<= 1;
		}

		if ((r2 + 1) >= (d - r2)) {
			if (q2 >= base_7f)
				adj = true;

			q2 = (q2 << 1) + 1;
			r2 = (r2 << 1) + 1 - d;
		} else {
			if (q2 >= base_80)
				adj = true;

			q2 <<= 1;
			r2 = (r2 << 1) + 1;
		}

		delta = d - 1 - r2;
	} while (
		(p < 2 * std::numeric_limits<T>::digits)
		&& (q1 < delta || (q1 == delta && r1 == 0))
	);

	++q2;

	return std::make_tuple(q2, p - std::numeric_limits<T>::digits, adj);
}

template <typename ValueType, typename AccType>
void gen_mul_small(uint64_t dr, uint64_t tv, bool test)
{
	auto rv(generate_mul<ValueType>(dr));
	printf("adj %d, ", std::get<2>(rv));
	printf("shift %d, ", std::get<1>(rv));
	printf("M 0x%x\n", std::get<0>(rv));

	if (test) {
		auto M(std::get<0>(rv));
		auto s(std::get<1>(rv));

		ValueType v1(tv);
		AccType v2(M);
		v2 *= v1;
		v2 = v2 >> (sizeof(ValueType) * 8);

		if (std::get<2>(rv))
			v2 += v1;

		ValueType v3(v2 >> s);
		printf("Divide %zu by %zu: %u\n", tv, dr, v3);
	}
}

void gen_mul_64(uint64_t dr, uint64_t tv, bool test)
{
	auto rv(generate_mul<uint64_t>(dr));
	printf("adj %d, ", std::get<2>(rv));
	printf("shift %d, ", std::get<1>(rv));
	printf("M 0x%zx\n", std::get<0>(rv));

	if (test) {
		auto M(std::get<0>(rv));
		auto s(std::get<1>(rv));

		uint64_t v1(tv);
		unsigned __int128 v2(M);
		v2 *= v1;
		v2 = v2 >> 64;

		if (std::get<2>(rv))
			v2 += v1;

		uint64_t v3(v2 >> s);
		printf("Divide %zu by %zu: %zu\n", tv, dr, v3);
	}
}

void gen_mul_128(uint64_t dr, uint64_t tv, bool test)
{
	auto rv(generate_mul<unsigned __int128>(dr));
	printf("adj %d, ", std::get<2>(rv));
	printf("shift %d, ", std::get<1>(rv));
	printf(
		"M 0x%zx, 0x%zx\n",
		uint64_t(std::get<0>(rv) >> 64), uint64_t(std::get<0>(rv))
	);
}

int main(int argc, char **argv)
{
	int bits;
	uint64_t dr(0);
	uint64_t tv(0);
	bool test(false);

	if (argc > 1) {
		sscanf(argv[1], "%u", &bits);
	}

	if (argc > 2) {
		sscanf(argv[2], "%lu", &dr);
	}

	if (argc > 3) {
		sscanf(argv[3], "%lu", &tv);
		test = true;
	}

	if (!dr)
		return -1;

	printf("Div %lu, ", dr);

	switch (bits) {
	case 8:
		gen_mul_small<uint8_t, uint16_t>(dr, tv, test);
		break;
	case 16:
		gen_mul_small<uint16_t, uint32_t>(dr, tv, test);
		break;
	case 32:
		gen_mul_small<uint32_t, uint64_t>(dr, tv, test);
		break;
	case 64:
		gen_mul_64(dr, tv, test);
		break;
	case 128:
		gen_mul_128(dr, tv, test);
		break;
	}

	return 0;
}
