/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_YESOD_MPL_DETAIL_TEMPLATE_ARITY_MAP_DEC_11_2013_1430)
#define UCPF_YESOD_MPL_DETAIL_TEMPLATE_ARITY_MAP_DEC_11_2013_1430

#include <yesod/mpl/integral.hpp>

namespace ucpf { namespace yesod { namespace mpl { namespace detail {

template <typename...>
struct template_arity;

template <typename T>
struct template_arity<T> : long_<0> {};

template <template <typename...> class F, typename... Tn>
struct template_arity<F<Tn...>> : long_<sizeof...(Tn)> {};

}}}}

#endif
