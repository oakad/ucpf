#include <holam/support/int128.hpp>
#include <holam/detail/binary_pow_10.hpp>
#include <random>
#define CASE_COUNT 10

namespace ucpf {

void print(soft_uint128_t v)
{
	printf(
		"%llX%016llX",
		(unsigned long long)(v.high),
		(unsigned long long)(v.low)
	);
}

void print(soft_uint256_t v)
{
	printf(
		"%llX%016llX%016llX%016llX",
		(unsigned long long)(v.high.high),
		(unsigned long long)(v.high.low),
		(unsigned long long)(v.low.high),
		(unsigned long long)(v.low.low)
	);
}

/*
void print_128(uint128_t v)
{
	printf(
		"%llX%016llX",
		(unsigned long long)(v >> 64),
		(unsigned long long)(v)
	);
}
*/
void t0()
{
	std::random_device dev;
	std::uniform_int_distribution<uint64_t> dist;

	for (int c(0); c < CASE_COUNT; ++c) {
		auto v0(dist(dev));
		auto v1(dist(dev));
		auto v2(holam::support::multiply(v0, v1));
		printf(
			"%llX * %llX\n", (unsigned long long)v0,
			(unsigned long long)v1
		);
		print(v2);
		printf("\n");
	}
}
/*
void t1()
{
	std::random_device dev;
	std::uniform_int_distribution<uint64_t> dist;

	for (int c(0); c < CASE_COUNT; ++c) {
		uint128_t v0{
			.low = dist(dev),
			.high = dist(dev)
		};
		uint64_t v1(dist(dev));
		printf("%llX%016llX - %llX\n", v0.high, v0.low, v1);
		v0 -= v1;
		printf("%llX%016llX\n", v0.high, v0.low);
	}
}
*/

void t2()
{
	std::random_device dev;
	std::uniform_int_distribution<uint64_t> dist;

	for (int c(0); c < CASE_COUNT; ++c) {
		uint128_t v0(dist(dev));
		v0 <<= 64;
		v0 |= dist(dev);
		uint128_t v1(dist(dev));
		v1 <<= 64;
		v1 |= dist(dev);
		soft_uint256_t v2(holam::support::multiply(v0, v1));

		print(v0);
		printf(" * ");
		print(v1);
		printf("\n");
		print(v2);
		printf("\n");
	}
}

void t3()
{
	int pos(0);
	for (auto v: holam::detail::binary_pow_10<>::pow_10_list) {
		uint128_t v0(v.m_high);
		v0 <<= 64;
		v0 |= v.m_low;
		print(v0);
		printf(": %d\n", pos++);
	}
}

}

int main(int argc, char **argv)
{
	//printf("obase = 16\nibase = 16\n");
	//ucpf::t2();
	//ucpf::t1();
	ucpf::t3();
}
