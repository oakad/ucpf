/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_MINA_DETAIL_FROM_ASCII_DECIMAL_U_20140727T2300)
#define UCPF_MINA_DETAIL_FROM_ASCII_DECIMAL_U_20140727T2300

namespace ucpf { namespace mina { namespace detail {

template <typename T>
struct has_max_digits {
	template <typename U, U>
	struct wrapper {};

	template <typename U>
	static std::true_type test(
		U *, wrapper<
			size_t (U::*)() const, &U::max_digits
		> * = nullptr
	);
	static std::false_type test(...);

	typedef decltype(test(static_cast<T *>(nullptr))) type;

	static constexpr bool value = type::value;
};

template <typename Policy, bool HasMaxDigits = true>
struct get_max_digits {
	static size_t apply(Policy const &p)
	{
		return p.max_digits();
	}
};

template <typename Policy>
struct get_max_digits<Policy, false> {
	static size_t apply(Policy const &p)
	{
		return 0;
	}
};

template <typename T, typename Policy>
struct from_ascii_decimal_u {
	template <typename InputIterator>
	from_ascii_decimal_u(
		InputIterator &first, InputIterator last,
		T initial = 0, Policy const &p = Policy()
	) : converted(0), head_cnt(0), tail_cnt(0), zero_tail(true),
	    value(initial)
	{
		constexpr bool counted(has_max_digits<Policy>::value);

		while (true) {
			if ((first == last) || !std::isdigit(*first))
				return;

			if (counted) {
				if (converted >= get_max_digits<
					Policy, counted
				>::apply(p))
					break;
			}

			if (Policy::skip_leading_zeros) {
				if (converted && (*first == '0')) {
					++first;
					++head_cnt;
					continue;
				}
			}

			value *= T(10);
			value += *first - '0';
			++converted;
			++first;
		}

		if (!Policy::consume_trailing_digits)
			return;

		if (Policy::round_tail) {
			if ((first != last) && std::isdigit(*first)) {
				if (*first >= '5')
					++value;
				else if (*first != '0')
					zero_tail = false;

				++tail_cnt;
				++first;
			}
		}

		while ((first != last) && std::isdigit(*first)) {
			if (*first != '0')
				zero_tail = false;

			++tail_cnt;
			++first;
		}
	}

	size_t consumed() const
	{
		return converted + head_cnt + tail_cnt;
	}

	size_t converted;
	size_t head_cnt;
	size_t tail_cnt;
	bool zero_tail;
	T value;
};

}}}
#endif
