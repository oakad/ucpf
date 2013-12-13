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

struct is_le {
	template <typename T>
	struct apply {
		typedef typename T::is_le type;
	};
};

template <
	typename IsLE, typename Tag,
	template <typename...> class F, typename... Tn
> struct le_result {
	typedef F<typename Tn::type...> result_;
	typedef result_ type;
};

template <typename Tag, template <typename...> class F, typename... Tn>
struct le_result<true_type, Tag, F, Tn...> {
	typedef bind<quote<F, Tag>, typename Tn::result_...> result_;
	typedef protect<result_> type;
};

template <typename IsLE, typename Tag, template <typename...> class F>
struct le_resultv {
	template <typename... Tn>
	struct apply {
		typedef F<typename Tn::type...> result_;
		typedef result_ type;
	};
};

template <typename Tag, template <typename...> class F>
struct le_resultv<true_type, Tag, F> {
	template <typename... Tn>
	struct apply {
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

template <long N, typename Tag>
struct lambda<arg<N>, Tag, long_<0>> {
	typedef true_type is_le;
	typedef arg<N> result_;
	typedef protect<result_> type;
};

template <typename T, typename Tag, typename Arity>
struct lambda<T, Tag, Arity> {
	typedef false_type is_le;
	typedef T result_;
	typedef T type;
};

template <typename T, typename Tag>
struct lambda<protect<T>, Tag, long_<1>> {
	typedef false_type is_le;
	typedef protect<T> result_;
	typedef result_ type;
};

template <typename F, typename... Tn, typename Tag>
struct lambda<bind<F, Tn...>, Tag> {
	typedef false_type is_le;
	typedef bind<F, Tn...> result_;
	typedef result_ type;
};

template <typename F, typename Tag0, typename Tag1, typename Arity>
struct lambda<lambda<F, Tag0, Arity>, Tag1, long_<3>> {
	typedef lambda<F, Tag1> l0;
	typedef lambda<Tag0, Tag1> l1;
	typedef typename l0::is_le is_le;

	typedef bind<
		quote<detail::template_arity>, typename l0::result_
	> arity_;

	typedef lambda<typename if_<is_le, arity_, Arity>::type, Tag1> l2;

	typedef detail::le_result<
		is_le, Tag1, mpl::lambda, l0, l1, l2
	> le_result_;

	typedef typename le_result_::result_ result_;
	typedef typename le_result_::type type;
};


template <template <typename...> class F, typename... Tn, typename Tag>
struct lambda<F<Tn...>, Tag, long_<sizeof...(Tn)>> {
	typedef package<lambda<Tn, Tag>...> ls;

	typedef typename transform1_pack<
		ls, detail::is_le
	>::type is_les;

	typedef typename apply_pack<is_les, or_>::type is_le;

	typedef apply_pack<
		ls, detail::le_resultv<is_le, Tag, F>::template apply
	> le_result_;

	typedef typename le_result_::result_ result_;
	typedef typename le_result_::type type;
};

template <typename...>
struct is_lambda_expression;

template <>
struct is_lambda_expression<> {
	template <typename T0, typename... Tn>
	struct apply : is_lambda_expression<T0> {};
};

template <typename Tag>
struct lambda<is_lambda_expression<>, Tag, long_<-1>> {
	typedef false_type is_le;
	typedef is_lambda_expression<> result_;
	typedef is_lambda_expression<> type;
};

template <typename T>
struct is_lambda_expression<T> {
	typedef typename lambda<T>::is_le type;
};

}}}

#endif
