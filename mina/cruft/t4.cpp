#include <mina/to_ascii_decimal.hpp>

using ucpf::mina::to_ascii_decimal;

void print_dbl(double v)
{
	char buf[80] = {0};
	char *ptr(buf);
	to_ascii_decimal(ptr, v);
	printf("--- std: %f\n", v);
	printf("^^^ imp: %s\n", buf);
}

int main(int argc, char **argv)
{
/*
	print_dbl(1.23);
	print_dbl(1230);
	print_dbl(1230e100);
	print_dbl(0.000123);
*/
	ucpf::yesod::float_t<64> x(std::numeric_limits<double>::epsilon());
	printf("m: %zx, e: %zx, sign %d\n", x.get_mantissa(), x.get_exponent(), x.get_sign());
	printf("aa %d\n", std::numeric_limits<double>::min_exponent);
	printf("aa %d\n", std::numeric_limits<double>::min_exponent10);
	printf("aa %d\n", std::numeric_limits<double>::max_exponent);
	printf("aa %d\n", std::numeric_limits<double>::max_exponent10);
	return 0;
}
