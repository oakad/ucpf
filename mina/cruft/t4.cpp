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

void test_mul()
{
	uint64_t xm(3287498753947549ull);
	uint64_t ym(2305743987457ull);
	uint64_t xo(12432543544564ull);
	uint64_t yo(9534946465766ull);
	uint128_t m(xm);
	m *= ym;
	struct {
		uint128_t m;
	} other;

	other.m = xo;
	other.m *= yo;

	printf("m (%zd * %zd) * ", xm, ym);
	printf("(%zd * %zd)\n", xo, yo);
	uint64_t mh(m >> 64), ml(m);
	uint64_t oh(other.m >> 64), ol(other.m);
	uint128_t acc_h(mh);
	acc_h *= oh;
	uint128_t acc_l(ml);
	acc_l *= ol;
	uint128_t acc_m(mh >= ml ? mh - ml : ml - mh);
	acc_m *= oh >= ol ? oh - ol : ol - oh;

	if ((mh >= ml) != (oh >= ol))
		acc_m += acc_h + acc_l;
	else
		acc_m = acc_h + acc_l - acc_m;

	uint64_t mv[4];

	mv[0] = acc_l;
	acc_m += acc_l >> 64;

	mv[1] = acc_m;
	acc_h += acc_m >> 64;

	mv[2] = acc_h;
	mv[3] = acc_h >> 64;

	printf("aa %016zx%016zx%016zx%016zx\n",
		mv[3], mv[2], mv[1], mv[0]
	);
};

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
	print_dbl(8.2897362787489375e+96);
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
