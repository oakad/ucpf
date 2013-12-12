/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

/*=============================================================================
    Based on extensions to boost::mpl library:

    Copyright (c) 2009 Larry Evans

    Permission to copy, use, modify, sell and distribute this software is
    granted provided this copyright notice appears in all copies. This software
    is provided "as is" without express or implied warranty, and with no claim
    as to its suitability for any purpose.
==============================================================================*/

#if !defined(UCPF_YESOD_MPL_FOLD_NULL_UNIT_DEC_12_2013_1140)
#define UCPF_YESOD_MPL_FOLD_NULL_UNIT_DEC_12_2013_1140

namespace ucpf { namespace yesod { namespace mpl {
namespace detail {

template <
	typename Null, typename Unit, template <typename C> class Converter,
	typename T0, typename T1
> struct null_unit_impl : Converter<T1>::type {};

template <
	typename Null, typename Unit, template <typename C> class Converter,
	typename T
> struct null_unit_impl<Null, Unit, Converter, Null, T> : Null {};

}

template <typename Null, typename Unit, template <typename C> class Converter>
struct null_unit {
	template <typename T0, typename T1>
	struct op : detail::null_unit_impl<
		Null, Unit, Converter, typename Converter<T0>::type, T1
	> {};
};

template <
	typename Null, typename Unit, template <typename C> class Converter,
	typename... Tn
> struct fold_null_unit;

template <typename Null, typename Unit, template <typename C> class Converter>
struct fold_null_unit<Null, Unit, Converter> : Unit {
	template <typename... Un>
	struct apply : fold_null_unit<Null, Unit, Converter, Un...> {};
};

template <
	typename Null, typename Unit, template <typename C> class Converter,
	typename T0, typename... Tn
> struct fold_null_unit<
	Null, Unit, Converter, T0, Tn...
> : detail::null_unit_impl<
	Null, Unit, Converter, typename Converter<T0>::type,
	fold_null_unit<Null, Unit, Converter, Tn...>
> {
	template <typename... Un>
	struct apply : fold_null_unit<Null, Unit, Converter, Un...>{};
};

}}}

#endif
