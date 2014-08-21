/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_MINA_TEST_FLOAT_GENERATOR_20140704T2300)
#define UCPF_MINA_TEST_FLOAT_GENERATOR_20140704T2300

#include <random>
#include <yesod/float.hpp>

namespace ucpf { namespace mina { namespace test {

template <size_t N>
struct float_generator_r {
	typedef yesod::float_t<N> wrapper_type;
	typedef typename wrapper_type::storage_type gen_type;
	typedef typename wrapper_type::machine_type value_type;

	template <typename Func>
	auto operator()(Func &&f)
	-> typename std::result_of<Func(value_type)>::type
	{
		
		while (true) {
			wrapper_type w(dist(dev));
			if (!w.is_special())
				return f(w.get());
		}
	}

	std::random_device dev;
	std::uniform_int_distribution<gen_type> dist;
};

template <size_t N>
struct float_generator_e {
	typedef yesod::float_t<N> wrapper_type;
	typedef typename wrapper_type::storage_type gen_type;
	typedef typename wrapper_type::machine_type value_type;
	constexpr static uint32_t mantissa_bits
	= wrapper_type::traits_type::mantissa_bits;
	constexpr static uint32_t exponent_bits
	= wrapper_type::traits_type::exponent_bits;

	template <typename Func>
	auto operator()(Func &&f)
	-> typename std::result_of<Func(value_type)>::type
	{
		while (true) {
			auto x(wrapper_type(dist(dev)).get_mantissa());
			x |= gen_type(last_exponent) << (mantissa_bits - 1);
			++last_exponent;
			wrapper_type w(x);
			if (w.is_special()) {
				last_exponent = 0;
				continue;
			}
			return f(w.get());
		}
	}

	std::random_device dev;
	std::uniform_int_distribution<gen_type> dist;
	uint32_t last_exponent = 0;
};

template <size_t MaxDigits, uint32_t MaxNegExponent, uint32_t MaxPosExponent>
struct dec_float_generator {
	constexpr static size_t exponent_digits = (yesod::order_base_2(
		MaxNegExponent > MaxPosExponent
		? MaxNegExponent : MaxPosExponent
	) * 1233) >> 12;
	constexpr static size_t buffer_size
	= 5 + MaxDigits + exponent_digits;
	constexpr static size_t digits_per_word = 19;

	dec_float_generator()
	: max_exponent_div(10)
	{
		for (size_t c(1); c < exponent_digits; ++c)
			max_exponent_div *= 10;
	}

	template <typename Func>
	auto operator()(Func &&f)
	-> typename std::result_of<Func(char *, char *)>::type
	{
		size_t i_cnt(0), f_cnt(0);
		bool m_sign(false), e_exp(false), e_sign(false);

		while (true) {
			auto r(dist(dev));
			m_sign = r & 1;
			e_exp = r & 2;
			e_sign = r & 4;
			r >>= 3;
			i_cnt = r % (MaxDigits + 1);
			if (i_cnt) {
				r /= (MaxDigits + 1);
				f_cnt = r % (i_cnt + 1);
				i_cnt -= f_cnt;
				break;
			}
		}

		auto last(buf);
		*last++ = m_sign ? '-' : '+';
		auto r(dist(dev));
		auto r_digits(digits_per_word);

		while (i_cnt) {
			if (!r_digits) {
				r = dist(dev);
				r_digits = digits_per_word;
			}
			*last++ = '0' + (r % 10);
			r /= 10;
			--r_digits;
			--i_cnt;
		}

		if (f_cnt)
			*last++ = '.';

		while (f_cnt) {
			if (!r_digits) {
				r = dist(dev);
				r_digits = digits_per_word;
			}
			*last++ = '0' + (r % 10);
			r /= 10;
			--r_digits;
			--f_cnt;
		}

		if (e_exp) {
			*last++ = 'e';
			*last++ = e_sign ? '-' : '+';

			int32_t exp(dist(dev) % (
				e_sign ? MaxNegExponent : MaxPosExponent
			));

			if (exp) {
				auto exp_div(max_exponent_div);
				while (exp_div > exp)
					exp_div /= 10;

				while (exp_div) {
					*last++ = '0' + (exp / exp_div);
					exp %= exp_div;
					exp_div /= 10;
				}
			} else
				*last++ = '0';
		}

		*last = 0;
		return f(buf, last);
	}

	std::random_device dev;
	std::uniform_int_distribution<uint64_t> dist;
	char buf[buffer_size];
	int32_t max_exponent_div;
};

#if !defined(_GLIBCXX_USE_INT128)

template <>
struct float_generator_r<128> {
	typedef yesod::float_t<128> wrapper_type;
	typedef uint64_t gen_type;
	typedef typename wrapper_type::machine_type value_type;

	template <typename Func>
	auto operator()(Func &&f)
	-> typename std::result_of<Func(value_type)>::type
	{
		
		while (true) {
			auto wl(dist(dev));
			auto wh(dist(dev));
			wrapper_type w(uint128_t(wl, wh));
			if (!w.is_special())
				return f(w.get());
		}
	}

	std::random_device dev;
	std::uniform_int_distribution<gen_type> dist;
};

template <>
struct float_generator_e<128> {
	typedef yesod::float_t<128> wrapper_type;
	typedef uint64_t gen_type;
	typedef typename wrapper_type::machine_type value_type;
	constexpr static uint32_t mantissa_bits
	= wrapper_type::traits_type::mantissa_bits;
	constexpr static uint32_t exponent_bits
	= wrapper_type::traits_type::exponent_bits;

	template <typename Func>
	auto operator()(Func &&f)
	-> typename std::result_of<Func(value_type)>::type
	{
		while (true) {
			auto xl(dist(dev));
			auto xh(dist(dev));
			auto x(wrapper_type(uint128_t(xl, xh)).get_mantissa());
			x |= gen_type(last_exponent) << (mantissa_bits - 1);
			++last_exponent;
			wrapper_type w(x);
			if (w.is_special()) {
				last_exponent = 0;
				continue;
			}
			return f(w.get());
		}
	}

	std::random_device dev;
	std::uniform_int_distribution<gen_type> dist;
	uint32_t last_exponent = 0;
};

#endif

}}}
#endif
