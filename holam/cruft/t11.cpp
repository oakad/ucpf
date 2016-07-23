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
	char c_val[h::fp_value_traits<T>::bcd_storage_size * 2 + 10] = {0};
	std::size_t val_len;
	int32_t exp10;
	h::fp_value_t<T> vv(v);
#if 1
	if (h::floating_point_to_bcd_grisu<T>::apply(
		bcd_val, val_len, exp10, vv
	))
		++grisu_true;
	else {
		++grisu_false;
		return;
	}
#else
	h::floating_point_to_bcd_bigint<T>::apply(
		bcd_val, val_len, exp10, vv
	);
#endif
	if (val_len > max_len)
		max_len = val_len;

	c_val[0] = std::signbit(v) ? '-' : '+';
	ht::expand_bcd(c_val + 1, bcd_val, val_len);
	val_len++;
	c_val[val_len++] = 'e';
	sprintf(c_val + val_len, "%d", exp10);
	T other(ht::read_float<T>::apply(c_val));
	if (other != v) {
		std::cout << "in " << v << " out " << c_val << " eq " << (other == v) << '\n';
		//printf("in %0.20e out %s eq %d\n", v, c_val, other == v);
	}
}

int main(int argc, char **argv)
{
#if 1
	//float v0(64);
	//verify(v0);
	//double v1(5.7089907708238395e+45);
	//verify(v1);
	ucpf::float128 v2(3.076044018662234017587819121941070509464e-4932Q);
	verify(v2);
#else
	ht::float_generator_r<float> gen;

	for (auto c(0); c < 1000000; ++c)
		gen(verify<float>);
#endif
	printf(
		"max len %zd, true %zd, false %zd\n",
		max_len, grisu_true, grisu_false
	);
}
