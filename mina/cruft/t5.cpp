#include <cstdio>
#include <cstdint>
#include <vector>
#include <mina/detail/bigint.hpp>

using namespace ucpf::mina::detail;

int main(void)
{
	typedef std::vector<bigint::limb_type> bigint_type;
	bigint_type x, y, z;
	bigint::assign_scalar(x, 3294793275u, 111);
	bigint::assign_pow10(y, 71);
	bigint::assign_pow10(z, 101);
	bigint::multiply(y, x);
	auto d(bigint::divide_near(y, z));
	printf("xx %s\n", bigint::to_ascii_hex(x).data());
	printf("yy %zX, %s\n", d, bigint::to_ascii_hex(y).data());
	return 0;
}
