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
#include <mina/from_ascii_numeric.hpp>

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

template <typename T, bool IsFloat>
struct string_cvt_scalar {
	template <typename StringType, typename Alloc>
	static bool generate(StringType &s, T const &v, Alloc const &a)
	{
		yesod::collector<
			typename StringType::value_type, 32, true, Alloc
		> sink(a);

		to_ascii_decimal_converter<T, IsFloat>::apply(
			std::back_inserter(sink), v, a
		);

		if (sink.empty()) {
			s = std::move(StringType());
			return false;
		} else {
			s = std::move(StringType(sink.begin(), sink.end(), a));
			return true;
		}
	}

	template <typename StringType, typename Alloc>
	static bool parse(T &v, StringType const &s, Alloc const &a)
	{
		return from_ascii_numeric(v, s.begin(), s.end(), a);
	}
};

template <typename T, bool IsFloat, bool IsString = false>
struct string_cvt_sequence {
	template <typename StringType, typename Alloc>
	static bool generate(StringType &s, T const &v, Alloc const &a)
	{
		typedef typename T::value_type Tv;

		yesod::collector<
			typename StringType::value_type, 32, true, Alloc
		> sink(a);

		auto first(v.begin());
		auto last(v.end());

		if (first == last)
			return false;

		to_ascii_decimal_converter<Tv, IsFloat>::apply(
			std::back_inserter(sink), *first, a
		);

		for (++first; first != last; ++first) {
			sink.push_back(',');
			sink.push_back(' ');
			to_ascii_decimal_converter<Tv, IsFloat>::apply(
				std::back_inserter(sink), *first, a
			);
		}

		s = std::move(StringType(sink.begin(), sink.end(), a));
		return true;
	}

	template <typename StringType, typename Alloc>
	static bool parse(T &v, StringType const &s, Alloc const &a)
	{
		typedef typename T::value_type Tv;

		auto skip_space = [](auto &first, auto last) -> bool {
			for (; first != last; ++first) {
				if (!std::isspace(*first))
					return true;
			}
			return false;
		};

		auto first(s.begin());
		auto last(s.end());
		Tv vv;

		if (!skip_space(first, last))
			return false;

		if (!from_ascii_numeric(vv, first, last, a))
			return false;

		v.emplace_back(vv);
		if (!skip_space(first, last))
			return true;

		while (true) {
			if (*first != ',')
				break;

			++first;
			if (!skip_space(first, last))
				break;

			if (!from_ascii_numeric(vv, first, last, a))
				break;

			v.emplace_back(vv);
			if (!skip_space(first, last))
				break;
		}
		return true;
	}
};

template <typename T>
struct string_cvt_sequence<T, false, true> {
	template <typename StringType, typename Alloc>
	static bool generate(StringType &s, T const &v, Alloc const &a)
	{
		s = std::move(StringType(v.begin(), v.end(), a));
		return true;
	}

	template <typename StringType, typename Alloc>
	static bool parse(T &v, StringType const &s, Alloc const &a)
	{
		v = std::move(T(s.begin(), s.end(), a));
		return true;
	}
};

template <typename T, int Kind>
struct string_cvt_helper;

template <typename T>
struct string_cvt_helper<
	T, kind_flags::integral
> : string_cvt_scalar<T, false> {};

template <typename T>
struct string_cvt_helper<
	T, kind_flags::float_t
> : string_cvt_scalar<T, true> {};

template <typename CharType, typename TraitsType, typename Alloc>
struct string_cvt_helper<
	std::basic_string<CharType, TraitsType, Alloc>,
	kind_flags::integral_sequence
> : string_cvt_sequence<
	std::basic_string<CharType, TraitsType, Alloc>, false, true
> {};

template <typename T>
struct string_cvt_helper<
	T, kind_flags::integral_sequence
> : string_cvt_sequence<T, false, false> {};

template <typename T>
struct string_cvt_helper<
	T, kind_flags::float_sequence
> : string_cvt_sequence<T, true, false> {};

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
