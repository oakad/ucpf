#include <cstdint>
#include <cstdio>
#include <cmath>
#include <mpfr.h>


int main(int argc, char **argv)
{
	mpfr_t x2, x10;
	mpfr_t l2, l10;
	mpfr_t inv_log2_10;
	mpfr_inits2(256, x2, x10, l2, l10, inv_log2_10, nullptr);
	mpfr_set_ui(x2, 2, MPFR_RNDN);
	mpfr_set_ui(x10, 10, MPFR_RNDN);
	mpfr_log(l2, x2, MPFR_RNDN);
	mpfr_log(l10, x10, MPFR_RNDN);
	mpfr_div(inv_log2_10, l2, l10, MPFR_RNDN);

	constexpr double d_inv_log2_10 = 0.30102999566398114;

	constexpr int32_t i_inv_log2_10 = 646456993;

	for (long c = -(1 << 14); c < (1 << 14); ++c) {
		mpfr_mul_si(x10, inv_log2_10, c, MPFR_RNDN);
		auto pow_10(mpfr_get_si(x10, MPFR_RNDU));

		auto d_pow_10(std::lround(std::ceil(
			c * d_inv_log2_10 - 1e-10
		)));

		int64_t acc(c);
		acc *= i_inv_log2_10;
		auto i_pow_10(acc ? (acc >> 31) + 1 : 0);

		if (pow_10 != d_pow_10) {
			printf(
				"xx pow_2 %ld, pow_10 %ld, diff %ld\n",
				c, pow_10, pow_10 - d_pow_10
			);
		}
		if (pow_10 != i_pow_10) {
			printf(
				"yy pow_2 %ld, pow_10 %ld, diff %ld\n",
				c, pow_10, pow_10 - i_pow_10
			);
		}
	}
	return 0;
}
