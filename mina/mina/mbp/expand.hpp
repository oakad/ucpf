/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_MINA_MBP_EXPAND_20140619T2300)
#define UCPF_MINA_MBP_EXPAND_20140619T2300

#include <mina/mbp/raw_encoding.hpp>

namespace ucpf { namespace mina { namespace mbp { namespace detail {

template <
	int ScalarRank, int NumericTypeRank, typename Func, typename... Args
> bool expand_apply(field_class<> f_class, Args &&...args)
{
	if (ScalarRank != f_class.scalar_r) {
		switch (f_class.numeric_type_r) {
		case scalar_rank<>::i8:
			return expand_apply<
				scalar_rank<>::i8, NumericTypeRank, Func
			>(std::forward<Args>(args)...);
		case scalar_rank<>::i16:
			return expand_apply<
				scalar_rank<>::i16, NumericTypeRank, Func
			>(std::forward<Args>(args)...);
		case scalar_rank<>::i32:
			return expand_apply<
				scalar_rank<>::i32, NumericTypeRank, Func
			>(std::forward<Args>(args)...);
		case scalar_rank<>::i64:
			return expand_apply<
				scalar_rank<>::i64, NumericTypeRank, Func
			>(std::forward<Args>(args)...);
		case scalar_rank<>::i128:
			return expand_apply<
				scalar_rank<>::i128, NumericTypeRank, Func
			>(std::forward<Args>(args)...);
		default:
			return false;
		}
	}

	if (NumericTypeRank != f_class.numeric_type_r) {
		switch (f_class.numeric_type_r) {
		case numeric_type_rank::n_signed:
			return expand_apply<
				ScalarRank, numeric_type_rank::n_signed, Func
			>(std::forward<Args>(args)...);
		case numeric_type_rank::n_unsigned:
			return expand_apply<
				ScalarRank, numeric_type_rank::n_unsigned, Func
			>(std::forward<Args>(args)...);
		case numeric_type_rank::n_float:
			return expand_apply<
				ScalarRank, numeric_type_rank::n_float, Func
			>(std::forward<Args>(args)...);
		default:
			return false;
		}
	}

	return Func::template rebind<ScalarRank, NumericTypeRank>::apply(
		std::forward<Args>(args)...
	);
}

template <
	int ScalarRank = scalar_rank<>::ie,
	int NumericTypeRank = numeric_type_rank::n_invalid
> struct expand_s {
	template <int ScalarRank1, int NumericTypeRank1>
	using rebind = expand_s<ScalarRank1, NumericTypeRank1>;

	template <typename ForwardIterator, typename Visitor>
	static bool apply(
		ForwardIterator &first, ForwardIterator last, Visitor &&v
	) {
		return false;
	}
};

template <int ScalarRank>
struct expand_s<ScalarRank, numeric_type_rank::n_signed> {
	template <int ScalarRank1, int NumericTypeRank1>
	using rebind = expand_s<ScalarRank1, NumericTypeRank1>;

	template <typename ForwardIterator, typename Visitor>
	static bool apply(
		ForwardIterator &first, ForwardIterator last, Visitor &&v
	) {
		typedef typename scalar_type<
			ScalarRank, numeric_type_rank::n_signed
		>::type value_type;

		auto xv(unpack_integral<true, value_type>(first, last));
		v.value(xv);
		return true;
	}
};

template <int ScalarRank>
struct expand_s<ScalarRank, numeric_type_rank::n_unsigned> {
	template <int ScalarRank1, int NumericTypeRank1>
	using rebind = expand_s<ScalarRank1, NumericTypeRank1>;

	template <typename ForwardIterator, typename Visitor>
	static bool apply(
		ForwardIterator &first, ForwardIterator last, Visitor &&v
	) {
		typedef typename scalar_type<
			ScalarRank, numeric_type_rank::n_unsigned
		>::type value_type;

		auto xv(unpack_integral<false, value_type>(first, last));
		v.value(xv);
		return true;
	}
};

template <int ScalarRank>
struct expand_s<ScalarRank, numeric_type_rank::n_float> {
	template <int ScalarRank1, int NumericTypeRank1>
	using rebind = expand_s<ScalarRank1, NumericTypeRank1>;

	template <typename ForwardIterator, typename Visitor>
	static bool apply(
		ForwardIterator &first, ForwardIterator last, Visitor &&v
	) {
		typedef typename scalar_type<
			ScalarRank, numeric_type_rank::n_float
		>::type value_type;

		value_type xv(unpack_integral<
			false, typename value_type::storage_type
		>(first, last));
		v.value((typename value_type::machine_type)xv);
		return true;
	}
};

template <
	int ScalarRank = scalar_rank<>::ie,
	int NumericTypeRank = numeric_type_rank::n_invalid
> struct expand_l {
	template <int ScalarRank1, int NumericTypeRank1>
	using rebind = expand_l<ScalarRank1, NumericTypeRank1>;

	template <typename ForwardIterator, typename Visitor>
	static bool apply(
		ForwardIterator &first, size_t list_size, int step, Visitor &&v
	) {
		return false;
	}
};

template <int ScalarRank>
struct expand_l<ScalarRank, numeric_type_rank::n_signed> {
	template <int ScalarRank1, int NumericTypeRank1>
	using rebind = expand_l<ScalarRank1, NumericTypeRank1>;

	template <typename ForwardIterator, typename Visitor>
	static bool apply(
		ForwardIterator &first, size_t list_size, int step, Visitor &&v
	) {
		typedef typename scalar_type<
			ScalarRank, numeric_type_rank::n_signed
		>::type value_type;

		auto s(v.template list_sink<value_type>(list_size));
		for (size_t c(0); c < list_size; ++c) {
			auto x_first(first);
			std::advance(first, step);
			*s = unpack_integral<true, value_type>(x_first, first);
		}

		return true;
	}
};

template <int ScalarRank>
struct expand_l<ScalarRank, numeric_type_rank::n_unsigned> {
	template <int ScalarRank1, int NumericTypeRank1>
	using rebind = expand_l<ScalarRank1, NumericTypeRank1>;

	template <typename ForwardIterator, typename Visitor>
	static bool apply(
		ForwardIterator &first, size_t list_size, int step, Visitor &&v
	) {
		typedef typename scalar_type<
			ScalarRank, numeric_type_rank::n_unsigned
		>::type value_type;

		auto s(v.template list_sink<value_type>(list_size));
		for (size_t c(0); c < list_size; ++c) {
			auto x_first(first);
			std::advance(first, step);
			*s = unpack_integral<false, value_type>(x_first, first);
		}

		return true;
	}
};

template <int ScalarRank>
struct expand_l<ScalarRank, numeric_type_rank::n_float> {
	template <int ScalarRank1, int NumericTypeRank1>
	using rebind = expand_l<ScalarRank1, NumericTypeRank1>;

	template <typename ForwardIterator, typename Visitor>
	static bool apply(
		ForwardIterator &first, size_t list_size, int step, Visitor &&v
	) {
		typedef typename scalar_type<
			ScalarRank, numeric_type_rank::n_float
		>::type value_type;

		auto s(v.template list_sink<
			typename value_type::machine_type
		>(list_size));

		for (size_t c(0); c < list_size; ++c) {
			auto x_first(first);
			std::advance(first, step);
			value_type xv(unpack_integral<
				false, typename value_type::storage_type
			>(x_first, first));
			*s = xv;
		}

		return true;
	}
};

template <typename ForwardIterator, typename Visitor>
bool expand(ForwardIterator &first, ForwardIterator last, Visitor &&v)
{
	uint8_t xv(*first);
	if (!field_class<>::header_valid(xv))
		return false;

	auto f_class(field_class<>::from_header(xv));
	if (f_class.scalar_r == scalar_rank<>::i5) {
		if (xv < small_uint_code_offset) {
			int8_t val(xv & small_int_mask);
			val -= small_int_code_offset;
			v.value(val);
		} else {
			uint8_t val(xv & small_int_mask);
			v.value(val);
		}
		++first;
		return true;
	};

	size_t list_size(0);
	int step(scalar_rank<>::order[f_class.scalar_r] / 8);

	if (f_class.list_size_r == list_size_rank::l3) {
		list_size = xv & small_list_size_mask;
		if (!advance_n(first, last, 1))
			return false;
	} else {
		if (!advance_n(first, last, 1))
			return false;
		auto x_first(first);
		if (!advance_n(first, last, f_class.list_size_r))
			return false;

		list_size = unpack_integral<false, size_t>(x_first, first);
	}

	++list_size;
	auto x_first(first);
	if (!advance_n(first, last, list_size * step))
		return false;

	if (list_size == 1)
		return expand_apply<
			scalar_rank<>::i8,
			numeric_type_rank::n_signed,
			expand_s<>
		>(f_class, x_first, first, std::forward<Visitor>(v));
	else
		return expand_apply<
			scalar_rank<>::i8,
			numeric_type_rank::n_signed,
			expand_l<>
		>(f_class, x_first, list_size, step, std::forward<Visitor>(v));

	return true;
}

}}}}

#endif
