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

template <size_t MaxDigits, size_t MaxExpDigits>
struct dec_float_generator {
	constexpr static size_t buffer_size
	= 5 + MaxDigits + MaxExpDigits;
	constexpr static size_t digits_per_word = 19;

	static_assert(buffer_size <= 64, "buffer_size < 64");

	template <typename Func>
	auto operator()(Func &&f)
	-> typename std::result_of<Func(char *, char *)>::type
	{
		size_t i_cnt(0), f_cnt(0), e_cnt(0);
		bool m_sign(false), e_sign(false);

		while (true) {
			auto r(dist(dev));
			m_sign = r & 1;
			e_sign = r & 2;
			r >>= 2;
			e_cnt = r % (MaxExpDigits + 1);
			r /= (MaxExpDigits + 1);
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

		if (e_cnt) {
			*last++ = 'e';
			*last++ = e_sign ? '-' : '+';
		}

		while (e_cnt) {
			if (!r_digits) {
				r = dist(dev);
				r_digits = digits_per_word;
			}
			*last++ = '0' + (r % 10);
			r /= 10;
			--r_digits;
			--e_cnt;
		}

		*last = 0;
		return f(buf, last);
	}

	std::random_device dev;
	std::uniform_int_distribution<uint64_t> dist;
	char buf[buffer_size];
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
