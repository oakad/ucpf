/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_MINA_DETAIL_CLASSIFY_20141113T2300)
#define UCPF_MINA_DETAIL_CLASSIFY_20141113T2300

#include <yesod/float.hpp>
#include <yesod/is_sequence.hpp>
#include <yesod/compose_bool.hpp>

namespace ucpf { namespace mina { namespace detail {

struct kind_flags {
	enum {
		integral = 1,
		float_t  = 2,
		sequence = 4,
		integral_sequence = sequence | integral,
		float_sequence = sequence | float_t
	};
};

template <typename T>
struct classify {
	template <typename U, int Kind, bool IsSequence = false>
	struct apply {
		constexpr static int value = ucpf::yesod::compose_bool<
			int, Kind, yesod::is_floating_point<U>::value,
			std::is_integral<U>::value
		>::value;
	};

	template <typename U, int Kind>
	struct apply<U, Kind, true> {
		constexpr static int value = apply<
			typename U::value_type, Kind | kind_flags::sequence,
			false
		>::value;
	};

	constexpr static int value = apply<
		T, 0, yesod::is_sequence<T>::value
	>::value;

	typedef typename std::integral_constant<int, value> type;
};

}}}
#endif
