/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_YESOD_MPL_IF_DEC_11_2013_1540)
#define UCPF_YESOD_MPL_IF_DEC_11_2013_1540

#include <type_traits>

namespace ucpf { namespace yesod { namespace mpl {

template <bool C, typename T0, typename T1>
using if_c = std::conditional<C, T0, T1>;

template <typename Tc, typename T0, typename T1>
using if_ = std::conditional<static_cast<bool>(Tc::value), T0, T1>;

template <bool C, typename T0, typename T1>
struct eval_if_c {
	typedef typename if_c<C, T0, T1>::type f_;
	typedef typename f_::type type;
};

template <typename Tc, typename T0, typename T1>
struct eval_if {
	typedef typename if_<Tc, T0, T1>::type f_;
	typedef typename f_::type type;
};

}}}

#endif
