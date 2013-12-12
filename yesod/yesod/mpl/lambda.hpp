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

#if !defined(UCPF_YESOD_MPL_LAMBDA_DEC_12_2013_1820)
#define UCPF_YESOD_MPL_LAMBDA_DEC_12_2013_1820

#include <yesod/mpl/bind.hpp>
#include <yesod/mpl/logical.hpp>
#include <yesod/mpl/identity.hpp>
#include <yesod/mpl/pack_inserter.hpp>
#include <yesod/mpl/transform_pack.hpp>

namespace ucpf { namespace yesod { namespace mpl {
namespace detail {

template <typename LeMaybe>
struct is_le {
	typedef typename LeMaybe::is_le type;
};

template <typename IsLE, typename Tag, template <typename...> class F>
struct le_resultv {
	template <typename... Tn>
	struct _ {
		typedef F<typename Tn::type...> result_;

		typedef result_ type;
	};
};

template <typename Tag, template <typename...> class F>
struct le_resultv<true_type, Tag, F> {
	template <typename... Tn>
	struct _ {
		typedef bind<quote<F, Tag>, typename Tn::result_...> result_;

		typedef protect<result_> type;
	};
};

}

template <>
struct lambda<> {
	template <typename T0, typename T1, typename... Tn> struct apply
	: lambda<T0, T1> {};
};

template <typename Tag>
struct lambda<lambda<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef lambda<> result_;
	typedef lambda<> type;
};

template <template <typename...> class F, typename... Tn, typename Tag>
struct lambda<F<Tn...>, Tag, long_<sizeof...(Tn)>> {
	typedef package<lambda<Tn, Tag>...> ls;
	typedef typename transform1_pack<
		ls, detail::is_le<arg<0>>
	>::type is_les;

	typedef typename apply_pack<is_les, or_>::type is_le;

	typedef apply_pack<
		ls, detail::le_resultv<is_le, Tag, F>::template _
	> le_result_;

	typedef typename le_result_::result_ result_;
	typedef typename le_result_::type type;
};

}}}

#endif
