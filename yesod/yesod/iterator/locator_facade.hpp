/*
 * Copyright (c) 2017 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

/*=============================================================================
    Based on implementation of boost::iterator library:

    Copyright (c) 2002-2003 David Abrahams
    Copyright (c) 2002      Jeremy Siek
    Copyright (c) 2002      Thomas Witt
    Copyright (c) 2012      Jeffrey Lee Hellrung, Jr.

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#if !defined(HPP_6597A132A7845DBE352B25D5E77831DB)
#define HPP_6597A132A7845DBE352B25D5E77831DB

#include <iterator>
#include <yesod/iterator/is_interoperable.hpp>

namespace ucpf::yesod::iterator {

template <typename... Args>
struct locator_facade;

struct locator_core_access {
	template <typename... Args>
	friend struct locator_facade;

	template <typename T0, typename T1>
	using choose_difference_type = std::conditional_t<
		std::is_convertible<T1, T0>::value,
		typename T0::difference_type,
		typename T1::difference_type
	>;

	template <typename... Tn, typename... Un>
	friend enable_if_interoperable_t<
		typename locator_facade<Tn...>::derived_type,
		typename locator_facade<Un...>::derived_type,
		bool
	> operator==(
		locator_facade<Tn...> const &lhs,
		locator_facade<Un...> const &rhs
	);

	template <typename... Tn, typename... Un>
	friend enable_if_interoperable_t<
		typename locator_facade<Tn...>::derived_type,
		typename locator_facade<Un...>::derived_type,
		bool
	> operator!=(
		locator_facade<Tn...> const &lhs,
		locator_facade<Un...> const &rhs
	);

	template <typename... Tn, typename... Un>
	friend enable_if_interoperable_t<
		typename locator_facade<Tn...>::derived_type,
		typename locator_facade<Un...>::derived_type,
		bool
	> operator<(
		locator_facade<Tn...> const &lhs,
		locator_facade<Un...> const &rhs
	);

	template <typename... Tn, typename... Un>
	friend enable_if_interoperable_t<
		typename locator_facade<Tn...>::derived_type,
		typename locator_facade<Un...>::derived_type,
		bool
	> operator>(
		locator_facade<Tn...> const &lhs,
		locator_facade<Un...> const &rhs
	);

	template <typename... Tn, typename... Un>
	friend enable_if_interoperable_t<
		typename locator_facade<Tn...>::derived_type,
		typename locator_facade<Un...>::derived_type,
		bool
	> operator<=(
		locator_facade<Tn...> const &lhs,
		locator_facade<Un...> const &rhs
	);

	template <typename... Tn, typename... Un>
	friend enable_if_interoperable_t<
		typename locator_facade<Tn...>::derived_type,
		typename locator_facade<Un...>::derived_type,
		bool
	> operator>=(
		locator_facade<Tn...> const &lhs,
		locator_facade<Un...> const &rhs
	);

	template <typename... Tn, typename... Un>
	friend enable_if_interoperable_t<
		typename locator_facade<Tn...>::derived_type,
		typename locator_facade<Un...>::derived_type,
		choose_difference_type<
			typename locator_facade<Tn...>::derived_type,
			typename locator_facade<Un...>::derived_type
		>
	> operator-(
		locator_facade<Tn...> const &lhs,
		locator_facade<Un...> const &rhs
	);

	template <typename... Tn>
	friend auto operator+(
		locator_facade<Tn...>const &lhs,
		typename locator_facade<
			Tn...
		>::derived_type::difference_type rhs
	);

	template <typename... Tn>
	friend auto operator+(
		typename locator_facade<
			Tn...
		>::derived_type::difference_type lhs,
		locator_facade<Tn...> const &rhs
	);

	template <typename Facade>
	static void increment(Facade &f)
	{
		f.increment();
	}

	template <typename Facade>
	static void decrement(Facade &f)
	{
		f.decrement();
	}

	template <typename Facade0, typename Facade1>
	static bool equal(Facade0 const &f0, Facade1 const &f1, std::true_type)
	{
		return f0.equal(f1);
	}

	template <typename Facade0, typename Facade1>
	static bool equal(Facade0 const &f0, Facade1 const &f1, std::false_type)
	{
		return f1.equal(f0);
	}

	template <typename Facade>
	static void advance(Facade &f, typename Facade::difference_type n)
	{
		f.advance(n);
	}

	template <typename Facade0, typename Facade1>
	static auto distance_from(
		Facade0 const &f0, Facade1 const &f1, std::true_type
	)
	{
		return -f0.distance_to(f1);
	}

	template <typename Facade0, typename Facade1>
	static auto distance_from(
		Facade0 const &f0, Facade1 const &f1, std::false_type
	)
	{
		return f1.distance_to(f0);
	}

	template <typename... Tn>
	static auto &derived(locator_facade<Tn...> &f)
	{
		return *static_cast<
			typename locator_facade<Tn...>::derived_type *
		>(&f);
	}

	template <typename... Tn>
	static auto const &derived(locator_facade<Tn...> &f)
	{
		return *static_cast<
			typename locator_facade<Tn...>::derived_type const *
		>(&f);
	}

	locator_core_access() = delete;
};

template <typename Derived, typename Category, typename Difference>
struct locator_facade<Derived, Category, Difference> {
	typedef Derived derived_type;
	typedef Category locator_category;
	typedef Difference difference_type;

	derived_type &derived()
	{
		return *static_cast<derived_type *>(this);
	}

	derived_type const &derived() const
	{
		return *static_cast<derived_type const *>(this);
	}

	derived_type &operator++()
	{
		locator_core_access::increment(this->derived());
		return this->derived();
	}

	derived_type operator++(int)
	{
		derived_type tmp(this->derived());
		++*this;
		return tmp;
	}

	derived_type &operator--()
	{
		locator_core_access::decrement(this->derived());
		return this->derived();
	}

	derived_type operator--(int)
	{
		derived_type tmp(this->derived());
		--*this;
		return tmp;
	}

	derived_type &operator+=(difference_type n)
	{
		locator_core_access::advance(this->derived(), n);
		return this->derived();
	}

	derived_type &operator-=(difference_type n)
	{
		locator_core_access::advance(this->derived(), -n);
		return this->derived();
	}

	derived_type operator-(difference_type x) const
	{
		derived_type result(this->derived());
		return result -= x;
	}
};

template <typename... Tn, typename... Un>
enable_if_interoperable_t<
	typename locator_facade<Tn...>::derived_type,
	typename locator_facade<Un...>::derived_type,
	bool
> operator==(
	locator_facade<Tn...> const &lhs,
	locator_facade<Un...> const &rhs
)
{
	typedef typename locator_facade<Tn...>::derived_type lh_derived;
	typedef typename locator_facade<Un...>::derived_type rh_derived;

	static_assert(is_interoperable<lh_derived, rh_derived>::value);

	return locator_core_access::equal(
		*static_cast<lh_derived const *>(&lhs),
		*static_cast<rh_derived const *>(&rhs),
		std::is_convertible<rh_derived, lh_derived>()
	); 
}

template <typename... Tn, typename... Un>
enable_if_interoperable_t<
	typename locator_facade<Tn...>::derived_type,
	typename locator_facade<Un...>::derived_type,
	bool
> operator!=(
	locator_facade<Tn...> const &lhs,
	locator_facade<Un...> const &rhs
)
{
	typedef typename locator_facade<Tn...>::derived_type lh_derived;
	typedef typename locator_facade<Un...>::derived_type rh_derived;

	static_assert(is_interoperable<lh_derived, rh_derived>::value);

	return !locator_core_access::equal(
		*static_cast<lh_derived const *>(&lhs),
		*static_cast<rh_derived const *>(&rhs),
		std::is_convertible<rh_derived, lh_derived>()
	); 
}

template <typename... Tn, typename... Un>
enable_if_interoperable_t<
	typename locator_facade<Tn...>::derived_type,
	typename locator_facade<Un...>::derived_type,
	bool
> operator<(
	locator_facade<Tn...> const &lhs,
	locator_facade<Un...> const &rhs
)
{
	typedef typename locator_facade<Tn...>::derived_type lh_derived;
	typedef typename locator_facade<Un...>::derived_type rh_derived;

	static_assert(is_interoperable<lh_derived, rh_derived>::value);

	return 0 > locator_core_access::distance_from(
		*static_cast<lh_derived const *>(&lhs),
		*static_cast<rh_derived const *>(&rhs),
		std::is_convertible<rh_derived, lh_derived>()
	);
}

template <typename... Tn, typename... Un>
enable_if_interoperable_t<
	typename locator_facade<Tn...>::derived_type,
	typename locator_facade<Un...>::derived_type,
	bool
> operator>(
	locator_facade<Tn...> const &lhs,
	locator_facade<Un...> const &rhs
)
{
	typedef typename locator_facade<Tn...>::derived_type lh_derived;
	typedef typename locator_facade<Un...>::derived_type rh_derived;

	static_assert(is_interoperable<lh_derived, rh_derived>::value);

	return 0 < locator_core_access::distance_from(
		*static_cast<lh_derived const *>(&lhs),
		*static_cast<rh_derived const *>(&rhs),
		std::is_convertible<rh_derived, lh_derived>()
	);
}

template <typename... Tn, typename... Un>
enable_if_interoperable_t<
	typename locator_facade<Tn...>::derived_type,
	typename locator_facade<Un...>::derived_type,
	bool
> operator<=(
	locator_facade<Tn...> const &lhs,
	locator_facade<Un...> const &rhs
)
{
	typedef typename locator_facade<Tn...>::derived_type lh_derived;
	typedef typename locator_facade<Un...>::derived_type rh_derived;

	static_assert(is_interoperable<lh_derived, rh_derived>::value);

	return 0 >= locator_core_access::distance_from(
		*static_cast<lh_derived const *>(&lhs),
		*static_cast<rh_derived const *>(&rhs),
		std::is_convertible<rh_derived, lh_derived>()
	);
}

template <typename... Tn, typename... Un>
enable_if_interoperable_t<
	typename locator_facade<Tn...>::derived_type,
	typename locator_facade<Un...>::derived_type,
	bool
> operator>=(
	locator_facade<Tn...> const &lhs,
	locator_facade<Un...> const &rhs
)
{
	typedef typename locator_facade<Tn...>::derived_type lh_derived;
	typedef typename locator_facade<Un...>::derived_type rh_derived;

	static_assert(is_interoperable<lh_derived, rh_derived>::value);

	return 0 <= locator_core_access::distance_from(
		*static_cast<lh_derived const *>(&lhs),
		*static_cast<rh_derived const *>(&rhs),
		std::is_convertible<rh_derived, lh_derived>()
	);
}

template <typename... Tn, typename... Un>
enable_if_interoperable_t<
	typename locator_facade<Tn...>::derived_type,
	typename locator_facade<Un...>::derived_type,
	locator_core_access::choose_difference_type<
		typename locator_facade<Tn...>::derived_type,
		typename locator_facade<Un...>::derived_type
	>
> operator-(
	locator_facade<Tn...> const &lhs,
	locator_facade<Un...> const &rhs
)
{
	typedef typename locator_facade<Tn...>::derived_type lh_derived;
	typedef typename locator_facade<Un...>::derived_type rh_derived;

	static_assert(is_interoperable<lh_derived, rh_derived>::value);
	
	return locator_core_access::distance_from(
		*static_cast<lh_derived const *>(&lhs),
		*static_cast<rh_derived const *>(&rhs),
		std::is_convertible<rh_derived, lh_derived>()
	);
}

template <typename... Tn>
auto operator+(
	locator_facade<Tn...> const &lhs,
	typename locator_facade<Tn...>::derived_type::difference_type rhs
)
{
	typedef typename locator_facade<Tn...>::derived_type lh_derived;

	lh_derived tmp(static_cast<lh_derived const &>(lhs));
	return tmp += rhs; 
}

template <typename... Tn>
auto operator+(
	typename locator_facade<Tn...>::derived_type::difference_type lhs,
	locator_facade<Tn...> const &rhs
)
{
	typedef typename locator_facade<Tn...>::derived_type rh_derived;

	rh_derived tmp(static_cast<rh_derived const &>(rhs));
	return tmp += lhs; 
}

}
#endif
