/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

/*=============================================================================
    Based on implementation of boost::iterator library:

    Copyright (c) 2003 David Abrahams

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#if !defined(UCPF_YESOD_ITERATOR_IS_LVALUE_ITERATOR_DEC_23_2013_1400)
#define UCPF_YESOD_ITERATOR_IS_LVALUE_ITERATOR_DEC_23_2013_1400

namespace ucpf { namespace yesod { namespace iterator {
namespace detail {

struct not_an_lvalue {};

template <typename T>
T &lvalue_preserver(T &, int);

template <typename T>
not_an_lvalue lvalue_preserver(T const &, ...);

template <typename Value>
struct is_lvalue_iterator_impl {
	struct conversion_eater {
		conversion_eater(Value &);
	};

	struct any_conversion_eater {
		template <typename T>
		any_conversion_eater(T const &);
	};

	static std::true_type tester(conversion_eater, int);
	static std::false_type tester(any_conversion_eater, int);

	template <typename It>
	struct rebind {
		static It &x;
		static constexpr bool value = decltype(tester(
			lvalue_preserver(*x, 0), 0
		))::value;
	};
};

template <>
struct is_lvalue_iterator_impl<void> {
	template <typename It>
	using rebind = std::false_type;
};

template <>
struct is_lvalue_iterator_impl<void const> {
	template <typename It>
	using rebind = std::false_type;
};

template <>
struct is_lvalue_iterator_impl<void volatile> {
	template <typename It>
	using rebind = std::false_type;
};

template <>
struct is_lvalue_iterator_impl<void const volatile> {
	template <typename It>
	using rebind = std::false_type;
};

template <typename It>
using is_readable_lvalue_iterator_impl = typename is_lvalue_iterator_impl<
	typename std::iterator_traits<It>::value_type const
>::template rebind<It>;

template <typename It>
using is_non_const_lvalue_iterator_impl = typename is_lvalue_iterator_impl<
	typename std::iterator_traits<It>::value_type
>::template rebind<It>;

}

template <typename T>
using is_lvalue_iterator = std::integral_constant<
	bool, detail::is_readable_lvalue_iterator_impl<T>::value
>;

template <typename T>
using is_non_const_lvalue_iterator = std::integral_constant<
	bool, detail::is_non_const_lvalue_iterator_impl<T>::value
>;

}}}
#endif
