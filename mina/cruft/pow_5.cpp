#include <mpfr.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

bool round_up_hex(char *s, int last)
{
	constexpr static char const *digits = "0123456789abcdef";

	for (int k(last); k >= 0; --k) {
		int digit(0);
		if (s[k] > 0x60)
			digit = s[k] - 0x61 + 10;
		else
			digit = s[k] - 0x30;

		++digit;
		if (digit > 15)
			s[k] = '0';
		else {
			s[k] = digits[digit];
			return false;
		}
	}
	return true;
}

int main(int argc, char **argv)
{
	//int min(-348 - 672), max(340 + 672), step(8);
	//int min(-10), max(10), step(1);
	int min(-344 - 4660), max(340 + 4676), step(8);
	mpfr_t x;
	mpfr_t y;
	mpz_t z;
	mpfr_init2(x, 8192);
	mpfr_init2(y, 8192);
	mpz_init(z);
	char c_high[17], c_low[17];

	for (int c(min); c <= max; c += step) {
		mpfr_set_ui(x, 5, MPFR_RNDN);
		mpfr_pow_si(y, x, c, MPFR_RNDN);
		auto e(mpfr_get_z_2exp(z, y));
		char *s(mpz_get_str(nullptr, 16, z));
		memcpy(c_high, s, 16);
		c_high[16] = 0;
		memcpy(c_low, s + 16, 16);
		c_low[16] = 0;
		if (s[33] > '7') {
			if (round_up_hex(c_low, 15))
				round_up_hex(c_high, 15);
		}

		mpfr_mul_2si(x, y, c, MPFR_RNDN);
		mpfr_log2(y, x, MPFR_RNDN);
		auto exp_2(mpfr_get_si(y, MPFR_RNDN));

		mpfr_set_ui_2exp(y, 1, exp_2, MPFR_RNDN);
		if (mpfr_cmp(x, y) > 0)
			++exp_2;

		//exp_2 -= 64;
		printf("{0x%sull, 0x%sull, %ld, %d},\n", c_high, c_low, exp_2, c);
		free(s);
	}

	mpfr_clear(x);
	mpfr_clear(y);
	mpz_clear(z);
	return 0;
}
