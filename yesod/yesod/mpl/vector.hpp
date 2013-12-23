/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

/*=============================================================================
    Based on implementation of boost::mpl library:

    Copyright (c) 2000-2004 Aleksey Gurtovoy

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

#if !defined(UCPF_YESOD_MPL_VECTOR_DEC_16_2013_1720)
#define UCPF_YESOD_MPL_VECTOR_DEC_16_2013_1720

#include <yesod/mpl/if.hpp>
#include <yesod/mpl/pair.hpp>
#include <yesod/mpl/base.hpp>
#include <yesod/mpl/logical.hpp>
#include <yesod/mpl/identity.hpp>
#include <yesod/mpl/next_prior.hpp>
#include <yesod/mpl/iterator_tags.hpp>
#include <yesod/mpl/fold_assoc_pack.hpp>

#include <yesod/mpl/detail/at.hpp>
#include <yesod/mpl/detail/size.hpp>
#include <yesod/mpl/detail/clear.hpp>

namespace ucpf { namespace yesod { namespace mpl {
namespace detail {

struct vector_tag {};
struct vector_iterator_tag {};

template <typename T, typename Base, bool at_front = false>
struct vector_item : Base {
	typedef typename Base::upper_bound_ index_;
	typedef typename increment<index_>::type upper_bound_;
	typedef typename increment<typename Base::size>::type size;
	typedef Base base;
	typedef vector_item type;

	static type_wrapper<T> item_(index_);
	using Base::item_;
};

template <typename T, typename Base>
struct vector_item<T, Base, true> : Base {
	typedef typename decrement<typename Base::lower_bound_>::type index_;
	typedef index_ lower_bound_;
	typedef typename increment<typename Base::size>::type size;
	typedef Base base;
	typedef vector_item type;

	static type_wrapper<T> item_(index_);
	using Base::item_;
};

template <typename T, typename Base>
struct vector_item_fold {
	typedef vector_item<T, Base, true> type;
};

template <typename Base, bool at_front>
struct vector_mask : Base {
	typedef typename decrement<typename Base::upper_bound_>::type index_;
	typedef index_ upper_bound_;
	typedef typename decrement<typename Base::size>::type size;
	typedef Base base;
	typedef vector_mask type;

	static type_wrapper<void_> item_(index_);
	using Base::item_;
};

template <typename Base>
struct vector_mask<Base, true> : Base {
	typedef typename Base::lower_bound_ index_;
	typedef typename increment<index_>::type lower_bound_;
	typedef typename decrement<typename Base::size>::type size;
	typedef Base base;
	typedef vector_mask type;

	static type_wrapper<void_> item_(index_);
	using Base::item_;
};

template <typename Vector, long n_>
struct vector_at_impl {
	typedef long_<(Vector::lower_bound_::value + n_)> index_;
	typedef decltype(Vector::item_(index_())) type;
};

template <typename Vector, long n_>
struct vector_at : wrapped_type<typename vector_at_impl<Vector, n_>::type> {};

template <typename Vector, long n_>
struct vector_iterator {
	typedef vector_iterator_tag tag;
	typedef random_access_iterator_tag category;
	typedef typename vector_at<Vector, n_>::type type;

	typedef Vector vector_;
	typedef long_<n_> pos;
};

}

template <typename Vector, long n_>
struct next<detail::vector_iterator<Vector, n_>> {
	typedef detail::vector_iterator<Vector, (n_ + 1)> type;
};

template <typename Vector, long n_>
struct prior<detail::vector_iterator<Vector, n_>> {
	typedef detail::vector_iterator<Vector, (n_ - 1)> type;
};

template <typename Vector, long n_, typename Distance>
struct advance<detail::vector_iterator<Vector, n_>, Distance> {
	typedef detail::vector_iterator<Vector, (n_ + Distance::value)> type;
};

template <typename Vector, long n_, long m_>
struct distance<
	detail::vector_iterator<Vector, n_>,
	detail::vector_iterator<Vector, m_>
> : long_<(m_ - n_)> {};

template <typename...>
struct vector;

template <>
struct vector<> {
	typedef detail::vector_tag tag;
	typedef vector type;
	typedef long_<32768> lower_bound_;
	typedef lower_bound_ upper_bound_;
	typedef long_<0> size;

	static detail::type_wrapper<void_> item_(...);
};

template <typename... Tn>
struct vector : fold_assoc_pack<
	typename detail::op_assoc::right, detail::vector_item_fold,
	vector<>, Tn...
>::type {
	typedef vector type;
};

template <typename T, T... Cn>
struct vector_c : vector<integral_constant<T, Cn>...> {
	typedef vector_c type;
	typedef T value_type;
};

template <typename T>
struct vector_c<T> : vector<> {
	typedef vector_c type;
	typedef T value_type;
};

namespace detail {

template <>
struct at_impl<vector_tag> {
	template <typename Vector, typename N>
	struct apply : vector_at<Vector, N::value>
	{};
};

template <>
struct back_impl<vector_tag> {
	template <typename Vector>
	struct apply : vector_at<
		Vector, decrement<typename Vector::size>::type::value
	> {};
};

template <>
struct begin_impl<vector_tag> {
	template <typename Vector>
	struct apply {
		typedef vector_iterator<Vector, 0> type;
	};
};

template <>
struct clear_impl<vector_tag> {
	template <typename Vector>
	struct apply {
		typedef vector<> type;
	};
};

template <>
struct empty_impl<vector_tag> {
	template <typename Vector>
	struct apply : std::is_same<
		typename Vector::lower_bound_, typename Vector::upper_bound_
	> {};
};

template <>
struct end_impl<vector_tag> {
	template <typename Vector>
	struct apply {
		typedef vector_iterator<Vector, Vector::size::value> type;
	};
};

template <>
struct front_impl<vector_tag> {
	template <typename Vector>
	struct apply : vector_at<Vector, 0>
	{};
};

template <>
struct pop_back_impl<vector_tag> {
	template <typename Vector>
	struct apply {
		typedef vector_mask<Vector, false> type;
	};
};

template <>
struct pop_front_impl<vector_tag> {
	template <typename Vector>
	struct apply {
		typedef vector_mask<Vector, true> type;
	};
};

template <>
struct push_back_impl<vector_tag> {
	template <typename Vector, typename T>
	struct apply {
		typedef vector_item<T, Vector, false> type;
	};
};

template <>
struct push_front_impl<vector_tag> {
	template <typename Vector, typename T>
	struct apply {
		typedef vector_item<T, Vector, true> type;
	};
};

template <>
struct O1_size_impl<vector_tag> {
	template <typename Vector>
	struct apply : Vector::size {};
};

template <>
struct size_impl<vector_tag>
{
	template <typename Vector>
	struct apply : Vector::size {};
};

}
}}}

#endif
