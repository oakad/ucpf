/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_YESOD_MPL_VALUE_CAST_DEC_17_2013_1730)
#define UCPF_YESOD_MPL_VALUE_CAST_DEC_17_2013_1730

#include <yesod/mpl/unpack_args.hpp>

namespace ucpf { namespace yesod { namespace mpl {
namespace detail {

template <typename ValueType>
struct value_cast_impl {
	template <typename...>
	struct apply;

	template <typename T0>
	struct apply<T0> {
		typedef apply type;

		static ValueType &push_back(ValueType &v)
		{
			v.push_back(T0::value);
			return v;
		}
	};

	template <typename T0, typename... Tn>
	struct apply<T0, Tn...> {
		typedef apply type;

		static ValueType &push_back(ValueType &v)
		{
			v.push_back(T0::value);
			return apply<Tn...>::push_back(v);
		}
	};
};

}

template <typename Sequence, typename ValueType>
ValueType value_cast()
{
	ValueType rv;
	typedef typename apply_wrap<
		unpack_args<detail::value_cast_impl<ValueType>>,
		Sequence
	>::type v;

	return v::push_back(rv);
}

}}}

#endif
