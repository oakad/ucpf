#include <mina/to_ascii_decimal.hpp>
#include <test/float_generator.hpp>

#include <gmp.h>

using ucpf::mina::to_ascii_decimal;
using ucpf::mina::test::float_generator;
using ucpf::mina::detail::to_ascii_decimal_f;

void print_flt(float v)
{
	char buf[80] = {0};
	char *ptr(buf);
	to_ascii_decimal_f<float>(ptr, v, std::allocator<void>());
	auto xv(strtof(buf, nullptr));
	printf("--- std: %f (%f), %d\n", v, xv, v == xv);
	printf("^^^ imp: %s\n", buf);
}

void print_flt_s(float v)
{
	char buf[80] = {0};
	char *ptr(buf);
	to_ascii_decimal_f<float>::bigint_convert(
		ptr, v, std::allocator<void>()
	);
	auto xv(strtof(buf, nullptr));
	printf("--- std: %f (%f), %d\n", v, xv, v == xv);
	printf("^^^ imp: %s\n", buf);
}


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
	printf("--- std: %e (%e), %d\n", v, xv, v == xv);
	printf("^^^ imp: %s\n", buf);
}

void test_mul()
{
	uint64_t xm(3298753947549ull);
	uint64_t ym(2376705743987457ull);
	uint64_t xo(12432658543544564ull);
	uint64_t yo(953494648365766ull);
	uint128_t m(xm);
	m *= ym;
	struct {
		uint128_t m;
	} other;

	other.m = xo;
	other.m *= yo;

	printf("m (%lld * %lld) * ", xm, ym);
	printf("(%lld * %lld)\n", xo, yo);

	auto rv(ucpf::yesod::multiply(m, other.m));
	uint64_t mv[4];

	mv[0] = rv.first;
	mv[1] = rv.first >> 64;
	mv[2] = rv.second;
	mv[3] = rv.second >> 64;;

	printf("aa %016llx%016llx%016llx%016llx\n",
		mv[3], mv[2], mv[1], mv[0]
	);
};

int main(int argc, char **argv)
{
/*
	print_flt_s(3.15102898e+20f);
	print_flt(3.15102898e+20f);
	print_flt_s(2.20260073e+12);
	print_flt(2.20260073e+12);
	print_dbl(2.20260073e+12);
*/
	print_dbl(8.517281238612595e+37);
	print_dbl_s(8.517281238612595e+37);
/*
	float_generator<64> fg;
	fg([](double v) -> bool {
		print_dbl(v);
		return true;
	});
*/
//	test_mul();
	return 0;
}
