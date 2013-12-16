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
    Copyright (c) 2003-2004 David Abrahams

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

#if !defined(UCPF_YESOD_MPL_SET_DEC_16_2013_1840)
#define UCPF_YESOD_MPL_SET_DEC_16_2013_1840

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
#include <yesod/mpl/detail/order.hpp>

namespace ucpf { namespace yesod { namespace mpl {
namespace detail {

struct set_tag {};

template <typename T, typename Base>
struct set_item : Base {
	typedef set_item<T, Base> item_;
	typedef void_ last_masked_;
	typedef T item_type_;
	typedef Base base;

	typedef typename increment<typename Base::size>::type size;
	typedef typename increment<typename Base::order>::type order;

	typedef char (&order_tag_)[order::value];

	static order_tag_ order_by_key_(set_item const&, type_wrapper<T>*);
	using Base::order_by_key_;

	static true_type is_masked_(set_item const&, type_wrapper<T>*);
	using Base::is_masked_;
};

template <typename T, typename Base>
struct set_item_fold {
	typedef set_item<T, Base> type;
};

template <typename T, typename Base>
struct set_mask : Base {
	typedef set_mask<T, Base> item_;
	typedef T last_masked_;
	typedef void_ item_type_;
	typedef Base base;
	typedef typename decrement<typename Base::size>::type size;

	static false_type is_masked_(set_mask const&, type_wrapper<T>*);
	using Base::is_masked_;
};

template <typename T, typename Base>
struct set_unmask : Base {
	typedef set_unmask<T, Base> item_;
	typedef void_ last_masked_;
	typedef T item_type_;
	typedef Base base;
	typedef typename increment<typename Base::size>::type size;

	static true_type is_masked_(set_unmask const&, type_wrapper<T>*);
	using Base::is_masked_;
};

template <typename Set, typename Tail>
struct set_iter;

template <typename Set, typename Tail>
struct set_iter_get : eval_if<
	mpl::has_key<Set, typename Tail::item_type_>,
	identity<set_iter<Set, Tail>>,
	next<set_iter<Set, Tail>>
> {};

template <typename Set, typename Tail>
struct set_iter_impl {
	typedef Tail tail_;
	typedef forward_iterator_tag category;
	typedef typename Tail::item_type_ type;
};

}

template <typename...>
struct set;

template <>
struct set<> {
	typedef set<> item_;
	typedef item_ type;
	typedef detail::set_tag tag;
	typedef void_ last_masked_;
	typedef void_ item_type_;
	typedef long_<0> size;
	typedef long_<1> order;

	typedef char (&order_tag_)[1];
 
	static order_tag_ order_by_key_(set<> const &, void const volatile *);
	static false_type is_masked_(set<> const &, void const volatile *);
};

template <typename... Tn>
struct set : fold_assoc_pack<
	typename detail::op_assoc::right, detail::set_item_fold,
	set<>, Tn...
>::type {
	typedef set type;
};

template <typename T, T... Cn>
struct set_c : set<integral_constant<T, Cn>...> {
	typedef set_c type;
	typedef T value_type;
};

template <typename T>
struct set_c<T> : set<> {
	typedef set_c type;
	typedef T value_type;
};

template <typename Set, typename Tail>
struct next<detail::set_iter<Set, Tail>> : detail::set_iter_get<
	Set, typename Tail::base
> {};

template <typename Set>
struct next<detail::set_iter<Set, set<>>> {
	typedef detail::set_iter<Set, set<>> type;
};

namespace detail {

template <typename Set, typename Tail>
struct set_iter : set_iter_impl<Set, Tail> {};

template <typename Set>
struct set_iter<Set, set<>> {
	typedef forward_iterator_tag category;
};

template <>
struct has_key_impl<set_tag> {
	template <typename Set, typename T>
	struct apply : decltype(Set::is_masked_(
		ptr_to_ref(static_cast<Set *>(0)),
		static_cast<type_wrapper<T> *>(0)
	)) {};
};

template <>
struct at_impl<set_tag> {
	template <typename Set, typename T>
	struct apply {
		typedef typename if_<
			has_key_impl<set_tag>::apply<Set,T>, T, void_
		>::type type;
	};
};

template <>
struct begin_impl<set_tag> {
	template <typename Set>
	struct apply : set_iter_get<Set, typename Set::item_> {};
};

template <>
struct clear_impl<set_tag> {
	template <typename Set>
	struct apply {
		typedef set<> type;
	};
};

template <>
struct empty_impl<set_tag> {
	template <typename Set>
	struct apply : not_<typename Set::size> {};
};

template <>
struct end_impl<set_tag> {
	template <typename Set>
	struct apply {
		typedef set_iter<Set, set<>> type;
	};
};

template <>
struct erase_key_impl<set_tag> {
	template <typename Set, typename T>
	struct apply : eval_if<
		has_key_impl<set_tag>::apply<Set,T>,
		eval_if<
			std::is_same<T, typename Set::item_type_>,
			base<Set>,
			identity<set_mask<T, typename Set::item_>>
		>, identity<Set>
	> {};
};

template <>
struct erase_impl<set_tag> {
	template <typename Set, typename Pos, typename unused_>
	struct apply : erase_key_impl<set_tag>::apply<
		Set, typename Pos::type
	> {};
};

template <>
struct insert_impl<set_tag>
{
	template <typename Set, typename T>
	struct apply_impl : eval_if<
		has_key_impl<set_tag>::apply<Set,T>,
		identity<Set>,
		eval_if<
			std::is_same<T, typename Set::last_masked_>,
			base<Set>,
			identity<set_item<T, typename Set::item_>>
		>
	> {};

	template <typename...>
	struct apply;

	template <typename Set, typename Pos>
	struct apply<Set, Pos> : apply_impl<Set, Pos> {};

	template <typename Set, typename Pos, typename T>
	struct apply<Set, Pos, T> : apply_impl<Set, T> {};
};

template <>
struct key_type_impl<set_tag> {
	template <typename Set, typename T>
	struct apply {
		typedef T type;
	};
};

template <>
struct size_impl<set_tag> {
	template <typename Set>
	struct apply : Set::size {};
};

template <>
struct value_type_impl<set_tag> {
	template <typename Set, typename T>
	struct apply {
		typedef T type;
	};
};

}
}}}

#endif
