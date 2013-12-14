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

#if !defined(UCPF_YESOD_MPL_MAP_DEC_11_2013_1210)
#define UCPF_YESOD_MPL_MAP_DEC_11_2013_1210

#include <yesod/mpl/if.hpp>
#include <yesod/mpl/pair.hpp>
#include <yesod/mpl/base.hpp>
#include <yesod/mpl/identity.hpp>
#include <yesod/mpl/next_prior.hpp>
#include <yesod/mpl/fold_assoc_pack.hpp>

#include <yesod/mpl/detail/at.hpp>
#include <yesod/mpl/detail/size.hpp>
#include <yesod/mpl/detail/clear.hpp>
#include <yesod/mpl/detail/order.hpp>
#include <yesod/mpl/detail/has_key.hpp>
#include <yesod/mpl/detail/erase_key.hpp>

namespace ucpf { namespace yesod { namespace mpl {
namespace detail {

struct map_tag {};

template <typename Key, typename T, typename Base>
struct map_item : Base {
	typedef Key key_;
	typedef pair<Key, T> item;
	typedef Base base;

	typedef typename next<typename Base::size>::type size;
	typedef typename next<typename Base::order>::type order;

	typedef char (&order_tag_)[order::value];

	static type_wrapper<T> value_by_key_(
		map_item const &, type_wrapper<Key> *
	);
	using Base::value_by_key_;

	static type_wrapper<item> item_by_order_(map_item const &, order *);
	using Base::item_by_order_;

	static order_tag_ order_by_key_(map_item const &, type_wrapper<Key> *);
	using Base::order_by_key_;
};

template <typename Key, typename Base>
struct map_mask : Base {
	typedef void_ key_;
	typedef Base base;

	typedef typename prior<typename Base::size>::type size;
	typedef typename x_order_impl<Base, Key>::type key_order_;

	static type_wrapper<void_> value_by_key_(
		map_mask const &, type_wrapper<Key> *
	);
	using Base::value_by_key_;

	static type_wrapper<void_> item_by_order_(
		map_mask const &, key_order_ *
	);
	using Base::item_by_order_;
};

template <typename P, typename Base>
struct map_item_fold {
	typedef map_item<typename P::first, typename P::second, Base> type;
};

template <typename Map, typename Key>
struct map_at {
	typedef type_wrapper<Key> key_;

	typedef decltype(Map::value_by_key_(
		ptr_to_ref(static_cast<Map *>(nullptr)),
		static_cast<key_ *>(nullptr)
	)) type;
};

}

template <typename...>
struct map;

template <>
struct map<> {
        typedef map type;
        typedef detail::map_tag tag;
        typedef void_ key_;
        typedef long_<1> order;
        typedef long_<0> size;

        static detail::type_wrapper<void_> value_by_key_(
                map<> const &, void const volatile *
        );

        static detail::type_wrapper<void_> item_by_order_(
                map<> const &, long_<1> *
        );

        static false_type order_by_key_(
                map<> const &, void const volatile *
        );
};

template <typename... Pn>
struct map : fold_assoc_pack<
	typename detail::op_assoc::right, detail::map_item_fold, map<>, Pn...
>::type {};

namespace detail {

template <>
struct at_impl<map_tag> {
	template <typename Map, typename Key>
	struct apply : wrapped_type<
		typename map_at<Map, Key>::type
	> {};
};

template <>
struct clear_impl<map_tag> {
	template <typename Map> struct apply {
		typedef map<> type;
	};
};

template <>
struct empty_impl<map_tag> {
	template <typename Map>
	struct apply : not_<typename Map::size> {};
};

template <>
struct has_key_impl<map_tag> {
	template <typename Map, typename Key>
	struct apply : is_not_void_<
		typename at_impl<map_tag>::apply<Map, Key>::type
	> {};
};

template <>
struct erase_key_impl<map_tag> {
	template<typename Map, typename Key>
	struct apply : eval_if<
		has_key_impl<map_tag>::apply<Map, Key>,
		eval_if<
			std::is_same<Key, typename Map::key_>,
			base<Map>, identity<map_mask<Key, Map>>
		>, identity<Map>
	> {};
};

template <>
struct O1_size_impl<map_tag> {
	template <typename Map>
	struct apply : Map::size {};
};

template <>
struct size_impl<map_tag> {
	template <typename Map>
	struct apply : Map::size {};
};

}

}}}

#endif
