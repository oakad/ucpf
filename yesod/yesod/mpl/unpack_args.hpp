/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

/*=============================================================================
    Based on extensions to boost::mpl library:

    Copyright (c) 2006-2010 Larry Evans

    Permission to copy, use, modify, sell and distribute this software is
    granted provided this copyright notice appears in all copies. This software
    is provided "as is" without express or implied warranty, and with no claim
    as to its suitability for any purpose.
==============================================================================*/

#if !defined(UCPF_YESOD_MPL_UNPACK_ARGS_DEC_16_2013_1800)
#define UCPF_YESOD_MPL_UNPACK_ARGS_DEC_16_2013_1800

#include <yesod/mpl/at.hpp>
#include <yesod/mpl/arg.hpp>
#include <yesod/mpl/size.hpp>
#include <yesod/mpl/apply.hpp>
#include <yesod/mpl/lambda.hpp>
#include <yesod/mpl/package_range_c.hpp>

namespace ucpf { namespace yesod { namespace mpl {
namespace detail {

template <typename Tpack, typename F, typename Tn>
struct unpack_args_impl;

template <long... Cn, typename F, typename Tn>
struct unpack_args_impl<package_c<long, Cn...>, F, Tn> : apply<
	F, typename at_c<Tn, Cn>::type...
> {};

}

template <typename F>
struct unpack_args {
	template <typename Tn>
	struct apply : detail::unpack_args_impl<
		typename package_range_c<
			long, long(0), long(size<Tn>::value)
		>::type, F, Tn
	> {};
};

template <typename T0, typename Tag>
struct lambda<unpack_args<T0>, Tag, long_<1>> {
	typedef false_type is_le;
	typedef unpack_args<T0> result_;
	typedef result_ type;
};

}}}

#endif
