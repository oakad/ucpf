#include <mina/to_ascii_decimal.hpp>
#include <test/float_generator.hpp>

#include <iostream>

using ucpf::mina::to_ascii_decimal;
using ucpf::mina::test::float_generator_r;
using ucpf::mina::detail::to_ascii_decimal_f;
using ucpf::yesod::float128;

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
#if 0
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
#endif
void print_flt128(float128 v)
{
	char buf[120] = {0};
	char *ptr(buf);
	to_ascii_decimal_f<float128>(ptr, v, std::allocator<void>());
	auto xv(strtoflt128(buf, nullptr));
	std::cout << "--- std: " << v << '(' << xv << "), "
		  << (v == xv) << '\n';
	printf("^^^ imp: %s\n", buf);
}

void print_flt128_s(float128 v)
{
	char buf[120] = {0};
	char *ptr(buf);
	to_ascii_decimal_f<float128>::bigint_convert(
		ptr, v, std::allocator<void>()
	);
	auto xv(strtoflt128(buf, nullptr));
	std::cout << "--- std: " << v << '(' << xv << "), "
		  << (v == xv) << '\n';
	printf("^^^ imp: %s\n", buf);
}
/*
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
*/
int main(int argc, char **argv)
{
/*
	print_flt_s(3.15102898e+20f);
	print_flt(3.15102898e+20f);
	print_flt_s(2.20260073e+12);
	print_flt(2.20260073e+12);
	print_dbl(2.20260073e+12);
*/
//	print_dbl(8.517281238612595e+37);
//	print_dbl_s(8.517281238612595e+37);
	uint8_t rr[] = {
		0xaa, 0x34, 0x72, 0x1a, 0x08, 0x3b, 0x2b, 0x84,
		0xf8, 0x8d, 0x58, 0xb0, 0x6d, 0xb1, 0x4b, 0x51
	};
//	auto x(reinterpret_cast<float128 *>(rr));
//	std::cout << *x << '\n';
//	print_flt128_s(*x);
//	print_flt128_s(1.299156547977642741607182298748798609224e-372Q);
//	print_flt128(1.299156547977642741607182298748798609224e-372Q);
//	print_flt128_s(53681914061667066325079.40807933397445595Q);
//	print_flt128(53681914061667066325079.40807933397445595Q);
//	print_flt128_s(5.489832816835470272485139686132842651587e+724Q);
//	print_flt128(5.489832816835470272485139686132842651587e+724Q);
	print_flt128_s(4.9406564584124654e-324Q);
	print_flt128(4.9406564584124654e-324Q);
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
