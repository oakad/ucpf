#include <holam/detail/floating_point_to_bcd_bigint.hpp>
#include <test/detail/float_generator.hpp>
#include <cstdio>

namespace h = ucpf::holam::detail;
namespace ht = ucpf::holam::detail::test;

std::size_t max_len = 5;
template <typename T>
void verify(T v)
{
	uint8_t bcd_val[h::fp_value_traits<T>::bcd_storage_size];
	char c_val[30] = {0};
	std::size_t val_len;
	int32_t exp10;
	h::fp_value_t<T> vv(v);

	h::floating_point_to_bcd_bigint<T>::apply(
		bcd_val, val_len, exp10, vv
	);
	if (val_len > max_len)
		max_len = val_len;

	c_val[0] = std::signbit(v) ? '-' : '+';
	ht::expand_bcd(c_val + 1, bcd_val, val_len);
	val_len++;
	c_val[val_len++] = 'e';
	sprintf(c_val + val_len, "%d", exp10);
	T other(ht::read_float<T>::apply(c_val));
	if (other != v) {
		printf("in %0.20e out %s eq %d\n", v, c_val, other == v);
	}
}

int main(int argc, char **argv)
{
#if 0
	double v(2.2250738585072014e-308);
	verify(v);
#else
	ht::float_generator_r<double> gen;

	for (auto c(0); c < 100000; ++c)
		gen(verify<double>);

	printf("max len %zd\n", max_len);
#endif
}
