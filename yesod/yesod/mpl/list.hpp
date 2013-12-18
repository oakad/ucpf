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

#if !defined(UCPF_YESOD_MPL_LIST_DEC_17_2013_1320)
#define UCPF_YESOD_MPL_LIST_DEC_17_2013_1320

#include <yesod/mpl/arithmetic.hpp>
#include <yesod/mpl/iterator_tags.hpp>
#include <yesod/mpl/fold_assoc_pack.hpp>

#include <yesod/mpl/detail/size.hpp>
#include <yesod/mpl/detail/clear.hpp>

namespace ucpf { namespace yesod { namespace mpl {
namespace detail {

struct list_tag {};
struct list_iterator_tag {};

template <typename T, typename Next>
struct list_item {
	typedef list_tag tag;
	typedef list_item type;
	typedef typename increment<typename Next::size>::type size;
	typedef T item;
	typedef Next next;
};

template <typename T, typename Next>
struct list_item_fold {
	typedef list_item<T, Next> type;
};

template <typename Node>
struct list_iterator {
	typedef list_iterator_tag tag;
	typedef forward_iterator_tag category;
};

}

template <typename Node>
struct deref<detail::list_iterator<Node>> {
	typedef typename Node::item type;
};

template <typename Node>
struct next<detail::list_iterator<Node>> {
	typedef detail::list_iterator<typename Node::next> type;
};

template <typename T, typename Tag>
struct lambda<detail::list_iterator<T>, Tag, long_<1>> {
	typedef false_type is_le;
	typedef detail::list_iterator<T> result_;
	typedef detail::list_iterator<T> type;
};

template <typename...>
struct list;

template <>
struct list<> {
	typedef detail::list_tag tag;
	typedef long_<0> size;
	typedef list type;
};

template <typename... Tn>
struct list : fold_assoc_pack<
	typename detail::op_assoc::right, detail::list_item_fold,
	list<>, Tn...
>::type {
	typedef list type;
};

template <typename T, T... Cn>
struct list_c : list<integral_constant<T, Cn>...> {
	typedef list_c type;
	typedef T value_type;
};

template <typename T>
struct list_c<T> : list<> {
	typedef list<> type;
	typedef T value_type;
};

namespace detail {

template <>
struct begin_impl<list_tag> {
	template <typename List>
	struct apply {
		typedef list_iterator<typename List::type> type;
	};
};

template <>
struct clear_impl<list_tag> {
	template <typename List> struct apply {
		typedef list<> type;
	};
};

template <>
struct empty_impl<list_tag> {
	template <typename List>
	struct apply : not_<typename List::size> {};
};

template <>
struct end_impl<list_tag> {
	template <typename List>
	struct apply {
		typedef list_iterator<list<>> type;
        };
};

template <>
struct front_impl<list_tag> {
	template <typename List>
	struct apply {
		typedef typename List::item type;
	};
};

template <>
struct has_push_back_impl<list_tag> {
	template <typename Seq>
	struct apply : false_type {};
};

template <>
struct pop_front_impl<list_tag> {
	template <typename List>
	struct apply {
		typedef typename next<List>::type type;
	};
};

template <>
struct push_front_impl<list_tag> {
	template<typename List, typename T>
	struct apply {
		typedef list_item<T, typename List::type> type;
	};
};

template <>
struct O1_size_impl<list_tag> {
	template <typename List>
	struct apply : List::size {};
};

template <>
struct size_impl<list_tag> {
	template <typename List>
	struct apply : List::size {};
};

}
}}}

#endif
