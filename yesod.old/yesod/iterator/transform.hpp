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

#if !defined(UCPF_YESOD_ITERATOR_TRANSFORM_20140914T2300)
#define UCPF_YESOD_ITERATOR_TRANSFORM_20140914T2300

#include <yesod/iterator/adaptor.hpp>

namespace ucpf { namespace yesod { namespace iterator {

template <typename... Args>
struct transform;

namespace detail {

template <typename F, typename Base, typename... Args>
struct transform_base {
	typedef Base iterator_type;
	typedef mpl::package<Args...> optional_args_pack;

private:
	template <typename F0, typename Pack, bool Optional = false>
	struct extract_reference {
		typedef typename std::result_of<const F0(
			typename std::iterator_traits<iterator_type>::reference
		)>::type type;
	};

	template <typename F0, typename Pack>
	struct extract_reference<F0, Pack, true> {
		typedef typename mpl::at_c<Pack, 0>::type type;
	};

	typedef typename extract_reference<
		F, optional_args_pack, (sizeof...(Args) > 0)
	>::type reference;

public:
	typedef adaptor<
		transform<F, Base, Args...>, Base,
		typename std::conditional<
			(sizeof...(Args) > 1),
			mpl::at_c<optional_args_pack, 1>,
			std::remove_reference<reference>
		>::type::type,
		typename std::iterator_traits<
			iterator_type
		>::iterator_category,
		reference
	> type;
};

}

/* Optional arguments in order:
 * 1. Reference type
 * 2. Value type
 */
template <typename F, typename Base, typename... Args>
struct transform<F, Base, Args...> : detail::transform_base<
	F, Base, Args...
>::type {
	static_assert(sizeof...(Args) < 3, "too many optional parameters");

	typedef detail::transform_base<F, Base, Args...> base_type;
	typedef typename base_type::iterator_type base_iterator_type;

	transform()
	{}

	transform(base_iterator_type const &iter_, F f_)
	: base_type::type(iter_), f(f_)
	{}

	explicit transform(base_iterator_type const &iter_)
	: base_type::type(iter_)
	{
		static_assert(
			std::is_class<F>::value, "std::is_class<F>::value"
		);
	}

	template <typename F0, typename Base0, typename... Args0>
	transform(
		transform<F0, Base0, Args0...> const &other,
		typename std::enable_if<
			std::is_convertible<Base0, Base>::value
		>::type * = nullptr,
		typename std::enable_if<
			std::is_convertible<F0, F>::value
		>::type * = nullptr
	) : base_type::type(other.base()), f(other.functor())
	{}

	F functor() const
	{
		return f;
	}

private:
	friend struct core_access;

	typename base_type::type::reference dereference() const
	{
		return f(*this->base());
	}

	F f;
};

template <typename F, typename Iterator>
transform<F, Iterator> make_transform(Iterator iter, F f)
{
	return transform<F, Iterator>(iter, f);
}

template <typename F, typename Iterator>
auto make_transform(Iterator iter) -> typename std::enable_if<
	std::is_class<F>::value, transform<F, Iterator>
>::type
{
	return transform<F, Iterator>(iter, F());
}

template <typename Return, typename Argument, typename Iterator>
auto make_transform(Iterator iter, Return (*f)(Argument)) -> transform<
	Return (*)(Argument), Iterator, Return
>
{
	return transform<
		Return (*)(Argument), Iterator, Return
	>(iter, f);
}

}}}
#endif
