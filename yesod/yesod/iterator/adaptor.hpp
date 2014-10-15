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

template <typename... Args>
struct adaptor;

namespace detail {

template <typename Derived, typename Base, typename... Args>
struct adaptor_base {
	typedef Derived derived_type;
	typedef Base iterator_type;

	template <typename IterType, size_t N>
	struct default_args {
		typedef std::iterator_traits<IterType> traits_type;

		typedef mpl::package<
			typename traits_type::value_type,
			typename traits_type::iterator_category,
			typename traits_type::reference,
			typename traits_type::difference_type
		> all_traits_pack;

		typedef typename mpl::at_c_indices<
			mpl::at_c_value<all_traits_pack>::template at_c,
			typename mpl::package_range_c<
				long, N,
				mpl::size<all_traits_pack>::type::value
			>::type
	        >::type pack;
	};

	template <typename IterType>
	struct default_args<IterType, 4> {
		typedef mpl::package<> pack;
	};

	typedef typename mpl::join_pack<
		mpl::package<Args...>,
		typename default_args<iterator_type, sizeof...(Args)>::pack
	>::type optional_args_pack;

	typedef facade<
		derived_type,
		typename mpl::at_c<optional_args_pack, 0>::type,
		typename mpl::at_c<optional_args_pack, 1>::type,
		typename mpl::at_c<optional_args_pack, 2>::type,
		typename mpl::at_c<optional_args_pack, 3>::type
	> type;
};

}

/* Optional arguments in order:
 * 1. Value type
 * 2. Iterator category or traversal type
 * 3. Reference type
 * 4. Difference type
 */
template <typename Derived, typename Base, typename... Args>
struct adaptor<Derived, Base, Args...> : detail::adaptor_base<
	Derived, Base, Args...
>::type {
	static_assert(sizeof...(Args) < 5, "too many optional parameters");

	typedef detail::adaptor_base<Derived, Base, Args...> base_type;
	typedef typename base_type::iterator_type base_iterator_type;
	typedef adaptor<Derived, Base, Args...> adaptor_type;

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
