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
struct float_generator {
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

}}}
#endif

