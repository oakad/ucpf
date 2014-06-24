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

int main(int argc, char **argv)
{
	unsigned long dr(0);
	unsigned long tv(0);
	bool test(false);
	if (argc > 1) {
		sscanf(argv[1], "%lu", &dr);
	}

	if (argc > 2) {
		sscanf(argv[2], "%lu", &tv);
		test = true;
	}

	if (!dr)
		return -1;

	printf("Div %lu, ", dr);

	auto rv(generate_mul<uint32_t>(dr));
	printf("adj %d, ", std::get<2>(rv));
	printf("shift %d, ", std::get<1>(rv));
	printf("M %x\n", std::get<0>(rv));

	if (test) {
		auto M(std::get<0>(rv));
		auto s(std::get<1>(rv));

		uint32_t v1(tv);
		uint64_t v2(M);
		v2 *= v1;
		v2 = (v2 >> 32) & 0xffffffff;

		if (std::get<2>(rv))
			v2 += v1;

		uint16_t v3(v2 >> s);
		printf("Divide %lu by %lu: %lu\n", tv, dr, (unsigned long)v3);
	}
	

	return 0;
}
