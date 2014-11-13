/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_MINA_FIXED_STRING_UTILS_20141111T2300)
#define UCPF_MINA_FIXED_STRING_UTILS_20141111T2300

#include <yesod/collector.hpp>
#include <mina/fixed_string.hpp>
#include <mina/detail/classify.hpp>
#include <mina/to_ascii_decimal.hpp>
#include <mina/from_ascii_decimal.hpp>

namespace ucpf { namespace mina {
namespace detail {

struct fixed_string_wrapper {
	typedef typename fixed_string::value_type value_type;

	fixed_string_wrapper()
	: str(fixed_string::make())
	{}

	template <typename Iterator, typename Alloc>
	fixed_string_wrapper(Iterator first, Iterator last, Alloc const &a)
	: str(fixed_string::make_r(a, first, last))
	{}

	fixed_string str;
};

template <typename T, int Kind>
struct string_cvt_helper;

template <typename T>
struct string_cvt_helper<T, kind_flags::integral> {
	template <typename StringType, typename Alloc>
	static bool generate(StringType &s, T const &v, Alloc const &a)
	{
		yesod::collector<
			typename StringType::value_type, 32, true, Alloc
		> sink(a);

		to_ascii_decimal_converter<T, false>::apply(
			std::back_inserter(sink), v, a
		);

		if (sink.empty()) {
			s = StringType();
			return false;
		} else {
			s = StringType(sink.begin(), sink.end(), a);
			return true;
		}
	}

	template <typename StringType, typename Alloc>
	static bool parse(T &v, StringType const &s, Alloc const &a)
	{
		auto first(s.begin());
		return from_ascii_decimal_converter<T, false>::apply(
			first, s.end(), v, a
		);
	}
};

}

template <typename T, typename Alloc>
bool to_fixed_string(fixed_string &s, T const &v, Alloc const &a)
{
	detail::fixed_string_wrapper w_s;

	auto rv(detail::string_cvt_helper<
		T, detail::classify<T>::value
        >::generate(w_s, v, a));

	s = w_s.str;
	return rv;
}

template <typename T, typename Alloc>
bool from_fixed_string(T &v, fixed_string const &s, Alloc const &a)
{
	return detail::string_cvt_helper<
		T, detail::classify<T>::value
	>::parse(v, s, a);
}

}}
#endif
