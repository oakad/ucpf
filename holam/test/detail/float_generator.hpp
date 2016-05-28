/*
 * Copyright (c) 2014-2016 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_128FAA27E23AB25653EE9972C7EC76E2)
#define HPP_128FAA27E23AB25653EE9972C7EC76E2

#include <random>
#include <holam/detail/floating_point_traits.hpp>

namespace ucpf { namespace holam { namespace detail { namespace test {

void expand_bcd(char *out, uint8_t *in, std::size_t len)
{
	for (std::size_t pos(0); pos < len; ++pos) {
		out[pos] = 0x30 + (pos & 1 ? (
			in[pos / 2] >> 4
		) : (in[pos / 2] & 0xf));
	}
}

template <typename T>
struct read_float {
};

template <>
struct read_float<float> {
	static float apply(char const *str)
	{
		return strtof(str, nullptr);
	}
};

template <>
struct read_float<double> {
	static double apply(char const *str)
	{
		return strtod(str, nullptr);
	}
};

template <typename T>
struct float_generator_r {
	typedef fp_value_traits<T> value_traits;
	typedef typename value_traits::mantissa_type mantissa_type;

	template <typename Func>
	auto operator()(Func &&f)
	{
		union {
			T fp;
			mantissa_type m;
		} val;
		while (true) {
			val.m = dist(dev);
			if (std::isnormal(val.fp))
				return f(val.fp);
		}
	}

	std::random_device dev;
	std::uniform_int_distribution<mantissa_type> dist;
};

template <typename T>
struct float_generator_e {
	typedef fp_value_traits<T> value_traits;
	typedef typename value_traits::mantissa_type mantissa_type;
	constexpr static auto mantissa_bits = value_traits::mantissa_bits;
	constexpr static auto exponent_bits = value_traits::exponent_bits;

	template <typename Func>
	auto operator()(Func &&f)
	{
		union {
			T fp;
			mantissa_type m;
		} val;
		while (true) {
			val.m = dist(dev);
			val.m &= (mantissa_type(1) << mantissa_bits) - 1;
			val.m |= mantissa_type(last_exponent) << mantissa_bits;
			++last_exponent;

			if (last_exponent > 2 * value_traits::exponent_bias)
				last_exponent = 0;

			return f(val.fp);
		}
	}

	std::random_device dev;
	std::uniform_int_distribution<mantissa_type> dist;
	uint32_t last_exponent = 0;
};

template <typename T>
struct float_generator_z {
	typedef fp_value_traits<T> value_traits;
	typedef typename value_traits::mantissa_type mantissa_type;
	constexpr static auto mantissa_bits = value_traits::mantissa_bits;
	constexpr static auto exponent_bits = value_traits::exponent_bits;

	template <typename Func>
	auto operator()(Func &&f)
	{
		union {
			T fp;
			mantissa_type m;
		} val;
		for (
			uint32_t exp(0);
			exp <= 2 * value_traits::exponent_bias;
			++exp
		) {
			val.m = 0;
			val.m |= mantissa_type(exp) << mantissa_bits;
			f(val.fp);
		}
	}

	std::random_device dev;
	std::uniform_int_distribution<mantissa_type> dist;
};

#if !defined(_GLIBCXX_USE_FLOAT128)

template <>
struct float_generator_r<float128> {
	typedef fp_value_traits<float128> value_traits;
	typedef typename value_traits::mantissa_type mantissa_type;

	template <typename Func>
	auto operator()(Func &&f)
	{
		float128 val;
		while (true) {
			val.low = dist(dev);
			val.high = dist(dev);
			if (std::isnormal(val))
				return f(val);
		}
	}

	std::random_device dev;
	std::uniform_int_distribution<uint64_t> dist;
};

template <>
struct float_generator_e<float128> {
	typedef fp_value_traits<T> value_traits;
	typedef typename value_traits::mantissa_type mantissa_type;
	constexpr static auto mantissa_bits = value_traits::mantissa_bits;
	constexpr static auto exponent_bits = value_traits::exponent_bits;

	template <typename Func>
	auto operator()(Func &&f)
	{
		float128 val;
		while (true) {
			val.low= dist(dev);
			val.high = dist(dev);
			val.high &= (uint64_t(1) << (
				mantissa_bits - uint64_t
			)) - 1;
			val.high |= uint64_t(last_exponent) << (
				mantissa_bits - 64
			);
			++last_exponent;

			if (last_exponent > 2 * value_traits::exponent_bias)
				last_exponent = 0;

			return f(val);
		}
	}

	std::random_device dev;
	std::uniform_int_distribution<mantissa_type> dist;
	uint32_t last_exponent = 0;
};

template <>
struct float_generator_z<float128> {
	typedef fp_value_traits<T> value_traits;
	typedef typename value_traits::mantissa_type mantissa_type;
	constexpr static auto mantissa_bits = value_traits::mantissa_bits;
	constexpr static auto exponent_bits = value_traits::exponent_bits;

	template <typename Func>
	auto operator()(Func &&f)
	{
		float128 val;
		for (
			uint32_t exp(0);
			exp <= 2 * value_traits::exponent_bias;
			++exp
		) {
			val.low = 0;
			val.high = 0;
			val.high |= uint64_t(exp) << (mantissa_bits - 64);

			return f(val);
		}
	}

	std::random_device dev;
	std::uniform_int_distribution<mantissa_type> dist;
};
#endif

#if 0
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

template <size_t MaxDigits, uint32_t MaxNegExponent, uint32_t MaxPosExponent>
struct hex_float_generator {
	constexpr static size_t exponent_digits = (yesod::order_base_2(
		MaxNegExponent > MaxPosExponent
		? MaxNegExponent : MaxPosExponent
	) * 1233) >> 12;
	constexpr static size_t buffer_size
	= 7 + MaxDigits + exponent_digits;
	constexpr static size_t digits_per_word = 16;

	hex_float_generator()
	: max_exponent_div(10)
	{
		for (size_t c(1); c < exponent_digits; ++c)
			max_exponent_div *= 10;
	}

	template <typename Func>
	auto operator()(Func &&f)
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
		*last++ = '0';
		*last++ = 'x';
		auto r(dist(dev));
		auto r_digits(digits_per_word);

		if (!i_cnt)
			*last++ = '0';

		while (i_cnt) {
			if (!r_digits) {
				r = dist(dev);
				r_digits = digits_per_word;
			}
			*last = '0' + (r & 0xf);
			if (*last > '9')
				*last += 39;
			last++;
			r >>= 4;
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
			*last = '0' + (r & 0xf);
			if (*last > '9')
				*last += 39;
			++last;
			r >>= 4;
			--r_digits;
			--f_cnt;
		}

		if (e_exp) {
			*last++ = 'p';
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

#endif
}}}}
#endif
