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

template <unsigned int N>
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

template <unsigned int N>
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
