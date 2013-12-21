/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

/*=============================================================================
    Based on implementation of boost::iterator library:

    Copyright (c) 2002 Jeremy Siek

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#if !defined(UCPF_YESOD_ITERATOR_CATEGORIES_DEC_20_2013_2300)
#define UCPF_YESOD_ITERATOR_CATEGORIES_DEC_20_2013_2300

#include <yesod/mpl/pair.hpp>
#include <yesod/mpl/list.hpp>
#include <yesod/mpl/find.hpp>

namespace ucpf { namespace yesod { namespace iterator {

struct no_traversal_tag {};

struct incrementable_traversal_tag : no_traversal_tag {};

struct single_pass_traversal_tag : incrementable_traversal_tag {};

struct forward_traversal_tag : single_pass_traversal_tag {};

struct bidirectional_traversal_tag : forward_traversal_tag {};

struct random_access_traversal_tag : bidirectional_traversal_tag {};

namespace detail {

typedef mpl::list<
	mpl::pair<std::random_access_iterator_tag, random_access_traversal_tag>,
	mpl::pair<std::bidirectional_iterator_tag, bidirectional_traversal_tag>,
	mpl::pair<std::forward_iterator_tag, forward_traversal_tag>,
	mpl::pair<std::input_iterator_tag, single_pass_traversal_tag>,
	mpl::pair<std::output_iterator_tag, incrementable_traversal_tag>
> traversal_of_category;

template <typename T>
struct is_reference_to_const : std::false_type
{};

template <typename T>
struct is_reference_to_const<T const &> : std::true_type
{};

template <typename ValueParam, typename Reference>
using writability_disabled = mpl::or_<
	std::is_const<Reference>,
	is_reference_to_const<Reference>,
	std::is_const<ValueParam>
>;

template <typename T>
using is_category = mpl::or_<
	std::is_convertible<T, std::input_iterator_tag>,
	std::is_convertible<T, std::output_iterator_tag>
>;

template <typename T>
using is_traversal = std::is_convertible<
	T, incrementable_traversal_tag
>;


template <typename Category>
struct category_to_traversal {
	template <typename T>
	struct has_category {
		template <typename U>
		using apply = std::is_convertible<
			T,
			typename mpl::first<U>::type
		>;
	};

	typedef typename mpl::if_<
		std::is_convertible<Category, incrementable_traversal_tag>,
		Category,
		typename mpl::second<typename mpl::deref<typename mpl::find_if<
			traversal_of_category, has_category<Category>
		>::type>::type>::type
	>::type type;
};

template <typename Category, typename Traversal>
struct category_with_traversal : Category, Traversal {
	static_assert(
		std::is_convertible<
			typename category_to_traversal<Category>::type,
			Traversal
		>::value,
		"iterator category and traversal types do not match"
	);
	static_assert(
		is_category<Category>::value,
	       "is_category<Category>::value"
	);
	static_assert(
		!is_category<Traversal>::value,
		"!is_category<Traversal>::value"
	);
	static_assert(
		!is_traversal<Category>::value,
		"!is_traversal<Category>::value"
	);
	static_assert(
		is_traversal<Traversal>::value,
		"is_traversal<Traversal>::value"
	);
};

}
}}}
#endif
