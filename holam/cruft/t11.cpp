#include <holam/detail/floating_point_to_bcd_bigint.hpp>
#include <holam/detail/floating_point_to_bcd_grisu.hpp>
#include <test/detail/float_generator.hpp>
#include <cstdio>

namespace h = ucpf::holam::detail;
namespace ht = ucpf::holam::detail::test;

std::size_t max_len = 5;
std::size_t grisu_true = 0;
std::size_t grisu_false = 0;
template <typename T>
void verify(T v)
{
	uint8_t bcd_val[h::fp_value_traits<T>::bcd_storage_size];
	char c_val[30] = {0};
	std::size_t val_len;
	int32_t exp10;
	h::fp_value_t<T> vv(v);

	if (h::floating_point_to_bcd_grisu<T>::apply(
		bcd_val, val_len, exp10, vv
	))
		++grisu_true;
	else {
		++grisu_false;
		return;
	}

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
	double v(4.99239550226548449897e+281);
	verify(v);
#else
	ht::float_generator_r<float> gen;

	for (auto c(0); c < 1000000; ++c)
		gen(verify<float>);
#endif
	printf(
		"max len %zd, true %zd, false %zd, %d\n",
		max_len, grisu_true, grisu_false, h::floating_point_to_bcd_grisu<float>::max_exp10
	);
}
