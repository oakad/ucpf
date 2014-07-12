#include <mina/to_ascii_decimal.hpp>
#include <test/float_generator.hpp>

#include <gmp.h>

using ucpf::mina::to_ascii_decimal;
using ucpf::mina::test::float_generator;
using ucpf::mina::detail::to_ascii_decimal_f;

void print_dbl(double v)
{
	char buf[80] = {0};
	char *ptr(buf);
	to_ascii_decimal_f<double>(ptr, v, std::allocator<void>());
	auto xv(strtod(buf, nullptr));
	printf("--- std: %f (%f), %d\n", v, xv, v == xv);
	printf("^^^ imp: %s\n", buf);
}

void print_dbl_s(double v)
{
	char buf[80] = {0};
	char *ptr(buf);
	to_ascii_decimal_f<double>::bigint_convert(
		ptr, v, std::allocator<void>()
	);
	auto xv(strtod(buf, nullptr));
	printf("--- std: %f (%f), %d\n", v, xv, v == xv);
	printf("^^^ imp: %s\n", buf);
}

int main(int argc, char **argv)
{
/*
	print_dbl(1.23);
	print_dbl(1230);
	print_dbl(1230e100);
	print_dbl(0.000123);
	print_dbl(3.5386175273211743e-15);
	print_dbl(2.7521330385015478e-205);
	print_dbl(25.779390132673132);
	print_dbl(1.9534695983725164e-308);
	print_dbl_s(1.9534695983725164e-308);
*/
//	print_dbl(8.2897362787489375e+96);
	print_dbl_s(8.2897362787489375e+96);
//	print_dbl_s(25.779390132673132);
/*
	print_dbl(-3.8501120564961564e+71);

	float_generator<64> fg;
	fg([](double v) -> bool {
		print_dbl(v);
		return true;
	});
*/
	return 0;
}
