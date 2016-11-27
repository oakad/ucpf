/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

/*=============================================================================
    Based on implementation of boost::mpl library:

    Copyright (c) 2001-2004 Aleksey Gurtovoy
    Copyright (c) 2001      Peter Dimov

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

/*=============================================================================
    Based on extensions to boost::mpl library:

    Copyright (c) 2006-2010 Larry Evans

    Permission to copy, use, modify, sell and distribute this software is
    granted provided this copyright notice appears in all copies. This software
    is provided "as is" without express or implied warranty, and with no claim
    as to its suitability for any purpose.
==============================================================================*/

#if !defined(UCPF_YESOD_MPL_BIND_DEC_14_2013_2300)
#define UCPF_YESOD_MPL_BIND_DEC_14_2013_2300

#include <yesod/mpl/if.hpp>
#include <yesod/mpl/arg.hpp>
#include <yesod/mpl/quote.hpp>
#include <yesod/mpl/package.hpp>
#include <yesod/mpl/push_pop.hpp>
#include <yesod/mpl/front_back.hpp>
#include <yesod/mpl/apply_wrap.hpp>
#include <yesod/mpl/fold_assoc_pack.hpp>

namespace ucpf { namespace yesod { namespace mpl {

template <typename...>
struct bind;

namespace detail {

template <typename T, typename Arg>
struct replace_unnamed_arg {
	typedef Arg next;
	typedef T type;
};

template <typename Arg>
struct replace_unnamed_arg<arg<-1l>, Arg> {
	typedef typename Arg::next next;
	typedef Arg type;
};

template <typename...>
struct resolve_bind_arg;

template <typename T, typename... Un>
struct resolve_bind_arg<T, Un...> {
	typedef T type;
};

template <typename F, typename... Tn, typename... Un>
struct resolve_bind_arg<bind<F, Tn...>, Un...> {
	typedef bind<F, Tn...> f_;
	typedef typename apply_wrap<f_, Un...>::type type;
};

template <long N, typename... Un>
struct resolve_bind_arg<arg<N>, Un...> {
	typedef typename apply_wrap<arg<N>, Un...>::type type;
};

template <typename... Un>
struct resolve_bind {
	template <typename NTs, typename A>
	struct op;

	template <typename N, typename... Tn, typename A>
	struct op<package<N, Tn...>, A> {
		typedef replace_unnamed_arg<A, N> r;
		typedef typename r::type a;
		typedef typename r::next n;
		typedef typename resolve_bind_arg<a, Un...>::type t;
		typedef package<n, Tn..., t> type;
	};
};

}

template <typename F, typename... Tn> 
struct bind<F, Tn...> {
	template <typename... Un>
	struct apply {
		typedef detail::resolve_bind<Un...> resolve_bind_us;
		typedef typename fold_assoc_pack<
			typename detail::op_assoc::left,
			resolve_bind_us::template op, package<arg<0>>,
			F, Tn...
		>::type resolveds;

		typedef typename pop_front<resolveds>::type fts;
		typedef typename front<fts>::type f_;
		typedef typename pop_front<fts>::type ts;

		template <typename... Vn>
		struct apply_wrap_f : apply_wrap<
			f_, Vn...
		>{};

		typedef typename apply_pack<ts, apply_wrap_f>::type type;
	};
};

template <typename Tag, typename T0, typename T1, typename T2>
struct bind<quote<if_, Tag>, T0, T1, T2> {
	typedef arg<0> r0;
	typedef detail::replace_unnamed_arg<T0, typename r0::next> r1;
	typedef detail::replace_unnamed_arg<T1, typename r1::next> r2;
	typedef detail::replace_unnamed_arg<T2, typename r2::next> r3;

	template <typename... Un>
	struct apply {
		typedef typename detail::resolve_bind_arg<
			typename r1::type, Un...
		> t0;

		typedef typename detail::resolve_bind_arg<
			typename r2::type, Un...
		> t1;

		typedef typename detail::resolve_bind_arg<
			typename r3::type, Un...
		> t2;

		typedef typename if_<typename t0::type, t1, t2>::type f_;

		typedef typename f_::type type;
	};
};

template <typename Tag, typename T0, typename T1, typename T2>
struct bind<quote<eval_if, Tag>, T0, T1, T2> {
	typedef arg<0> r0;
	typedef detail::replace_unnamed_arg<T0, typename r0::next> r1;
	typedef detail::replace_unnamed_arg<T1, typename r1::next> r2;
	typedef detail::replace_unnamed_arg<T2, typename r2::next> r3;

	template <typename... Un>
	struct apply {
		typedef typename detail::resolve_bind_arg<
			typename r1::type, Un...
		> t0;

		typedef typename detail::resolve_bind_arg<
			typename r2::type, Un...
		> t1;

		typedef typename detail::resolve_bind_arg<
			typename r3::type, Un...
		> t2;

		typedef typename eval_if<typename t0::type, t1, t2>::type f_;

		typedef typename f_::type type;
	};
};

}}}

#endif
