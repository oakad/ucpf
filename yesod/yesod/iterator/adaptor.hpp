/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

/*=============================================================================
    Based on implementation of boost::iterator library:

    Copyright (c) 2002 David Abrahams
    Copyright (c) 2002 Jeremy Siek
    Copyright (c) 2002 Thomas Witt

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#if !defined(UCPF_YESOD_ITERATOR_ADAPTOR_20140914T2300)
#define UCPF_YESOD_ITERATOR_ADAPTOR_20140914T2300

#include <yesod/iterator/facade.hpp>

namespace ucpf { namespace yesod { namespace iterator {
namespace detail {

template <typename... Tn>
struct adaptor_base {
	static_assert(
		sizeof...(Tn) > 1,
		"At least the derived type for CRTP and base iterator type "
		"must be specified."
	);

	typedef mpl::apply_wrap<mpl::arg<0>, Tn...>::type derived_type;
	typedef mpl::apply_wrap<mpl::arg<1>, Tn...>::type iterator_type;
	typedef std::iterator_traits<iterator_type> traits_type;

	typedef typename facade<
		derived_type,
		typename std::conditional<
			sizeof...(Tn) > 2,
			typename mpl::apply_wrap<mpl::arg<2>, Tn...>::type,
			typename traits_type::value_type
		>::type,
		typename std::conditional<
			sizeof...(Tn) > 3,
			typename mpl::apply_wrap<mpl::arg<3>, Tn...>::type,
			typename traits_type::iterator_category
		>::type,
		typename std::conditional<
			sizeof...(Tn) > 4,
			typename mpl::apply_wrap<mpl::arg<4>, Tn...>::type,
			typename traits_type::reference
		>::type,
		typename std::conditional<
			sizeof...(Tn) > 5,
			typename mpl::apply_wrap<mpl::arg<5>, Tn...>::type,
			typename traits_type::difference_type
		>::type
	>::type type;
};

}

/* Expected arguments in order:
 * 1. Derived type for CRTP
 * 2. Base iterator type
 * 3. Value type
 * 4. Iterator category or traversal type
 * 5. Reference type
 * 6. Difference type
 */
template <typename... Tn>
struct adaptor : adaptor_base<Tn...>::type
{
	typedef typename adaptor_base<Tn...> base_type;
	typedef typename base_type::iterator_type base_iterator_type;
	typedef adaptor<Tn...> adaptor_type;

	adaptor()
	{}

	explicit adaptor(base_iterator_type const &iter_)
	: iter(iter_)
	{}

	base_iterator_type const &base() const
	{
		return iter;
	}

protected:
	base_iterator_type const &base_reference() const
	{
		return iter;
	}

	base_iterator_type &base_reference()
	{
		return iter;
	}

private:
	friend struct core_access;

	typename base_type::type::reference dereference() const
	{
		return *iter;
	}

	template <typename... Un>
	bool equal(adaptor<Un...> const &other) const
	{
		return iter == other.base();
	}

	void advance(typename base_type::type::difference_type n)
	{
		std::advance(iter, n);
	}

	void increment()
	{
		++iter;
	}

	void decrement()
	{
		--iter;
	}

	template <typename... Un>
	typename base_type::type::difference_type distance_to(
		adaptor<Un...> const &other
	) const
	{
		return std::distance(iter, other.base());
	}

	base_iterator_type iter;
};

}}}

#endif
