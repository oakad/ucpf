/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_YESOD_IS_SEQUENCE_NOV_09_2013_2300)
#define UCPF_YESOD_IS_SEQUENCE_NOV_09_2013_2300

#include <type_traits>

namespace ucpf { namespace yesod {
namespace detail {

template <typename T>
struct has_value_type {
	template <typename U>
	struct wrapper {};

	template <typename U>
	static std::true_type test(
		wrapper<U> const volatile *,
		wrapper<typename U::value_type> * = nullptr
	);

	static std::false_type test(...);

	typedef decltype(test(static_cast<wrapper<T> *>(nullptr))) type;

	static bool const value = type::value;
};

template <typename T>
struct has_const_iterator {
	template <typename U>
	struct wrapper {};

	template <typename U>
	static std::true_type test(
		wrapper<U> const volatile *,
		wrapper<typename U::const_iterator> * = nullptr
	);

	static std::false_type test(...);

	typedef decltype(test(static_cast<wrapper<T> *>(nullptr))) type;

	static bool const value = type::value;
};

/* Consider using SFINAE friendly std::result_of when available. */

template <typename T, bool HasIter = false>
struct has_cbegin {
	typedef std::false_type type;
	static bool const value = type::value;
};

template <typename T>
struct has_cbegin<T, true> {

	template <typename U, U>
	struct wrapper {};

	typedef typename T::const_iterator const_iterator;

	template <typename U>
	static std::true_type test(
		wrapper<
			const_iterator (U::*)() const, &U::begin
		> * = nullptr
	);
	static std::false_type test(...);

	typedef decltype(test<T>()) type;

	static constexpr bool value = type::value;
};

}

template <typename T>
struct is_sequence {
	typedef typename std::integral_constant<
		bool,
		detail::has_value_type<T>::value
		&& detail::has_cbegin<
			T, detail::has_const_iterator<T>::value
		>::value
	>::type type;

	static bool const value = type::value;
};

}}
#endif
