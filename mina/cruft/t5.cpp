#include <cstdio>
#include <cstdint>
#include <vector>
#include <mina/detail/bigint.hpp>

using namespace ucpf::mina::detail;

int main(void)
{
/*
	typedef std::vector<bigint::limb_type> bigint_type;
	bigint_type x, y, z;
	bigint::assign_scalar(x, 3294793275u, 111);
	bigint::assign_pow10(y, 71);
	bigint::assign_pow10(z, 101);
	bigint::multiply(y, x);
	auto d(bigint::divide_near(y, z));
	printf("xx %s\n", bigint::to_ascii_hex(x).data());
	printf("yy %zX, %s\n", d, bigint::to_ascii_hex(y).data());
*/

	uint128_t x1(0x9843FECAB359A625, 0x213656);
	uint128_t x2(0x9843FECAB359A625, 0x43957);
	printf("yy01 %d\n", x2 < x2);
	printf("yy02 %d\n", x2 <= x2);
	printf("yy03 %d\n", x2 > x2);
	printf("yy04 %d\n", x2 >= x2);

	printf("xx01 %016llX%016llX\n", x1.v[1], x1.v[0]);
	++x1;
	printf("xx02 %016llX%016llX\n", x1.v[1], x1.v[0]);
	--x1;
	printf("xx03 %016llX%016llX\n", x1.v[1], x1.v[0]);
	x1 += 0x985743ull;
	printf("xx04 %016llX%016llX\n", x1.v[1], x1.v[0]);
	x1 += x2;
	printf("xx05 %016llX%016llX\n", x1.v[1], x1.v[0]);
	x1 -= 0x985743ull;
	printf("xx06 %016llX%016llX\n", x1.v[1], x1.v[0]);
	x1 <<= 15u;
	printf("xx07 %016llX%016llX\n", x1.v[1], x1.v[0]);
	x1 >>= 14u;
	printf("xx08 %016llX%016llX\n", x1.v[1], x1.v[0]);
	auto x3(x1 + x2);
	printf("xx09 %016llX%016llX\n", x3.v[1], x3.v[0]);
	auto x4(x3 - 0x985743ull);
	printf("xx10 %016llX%016llX\n", x4.v[1], x4.v[0]);
	auto x5(x4 - x2);
	printf("xx11 %016llX%016llX\n", x5.v[1], x5.v[0]);
	auto x6(x5 * 0x985743ull);
	printf("xx12 %016llX%016llX\n", x6.v[1], x6.v[0]);
	auto x7(x6 * x2);
	printf("xx13 %016llX%016llX\n", x7.v[1], x7.v[0]);
	return 0;
}
