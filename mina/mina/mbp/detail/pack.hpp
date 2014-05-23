/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_MINA_MBP_DETAIL_PACK_20131107T1700)
#define UCPF_MINA_MBP_DETAIL_PACK_20131107T1700

#include <cstdint>
#include <array>
#include <yesod/float.hpp>
#include <yesod/is_sequence.hpp>
#include <yesod/compose_bool.hpp>
#include <mina/mbp/detail/raw_encoding.hpp>

namespace ucpf { namespace mina { namespace mbp {

template <typename OutputIterator, typename T>
struct custom {
	static void pack(OutputIterator &&sink, T &&v);
};

namespace detail {

template <unsigned int N, typename OutputIterator, typename T>
typename std::enable_if<N == 1, void>::type pack_integral(
	OutputIterator &&sink, T &&v
)
{
	*sink++ = static_cast<uint8_t>(v);
}

template <unsigned int N, typename OutputIterator, typename T>
typename std::enable_if<(N > 1), void>::type pack_integral(
	OutputIterator &&sink, T &&v
)
{
	*sink++ = static_cast<uint8_t>(v & 0xff);
	pack_integral<N - 1>(sink, v >> 8);
}

template <typename OutputIterator, typename T>
void pack_integral(OutputIterator &&sink, T &&v, unsigned int cnt)
{
	for(auto c(cnt); c > 0; --c)
		*sink++ = static_cast<uint8_t>(
			(v >> ((cnt - c) * 8)) & 0xff
		);
}

struct kind_flags {
	enum {
		integral = 1,
		float_t  = 2,
		sequence = 4
	};
};

template <typename T, int Kind, bool IsSequence = false>
struct classify {
	constexpr static int value = ucpf::yesod::compose_bool<
		int, Kind, yesod::is_floating_point<T>::value,
		std::is_integral<T>::value
	>::value;
	typedef typename std::integral_constant<int, value> type;
};

template <typename T, int Kind>
struct classify<T, Kind, true> {
	constexpr static int value = classify<
		typename T::value_type, Kind | kind_flags::sequence, false
	>::value;
	typedef typename std::integral_constant<int, value> type;
};

template <typename OutputIterator, typename T, int Kind>
struct pack_helper;

template <typename OutputIterator, typename T>
struct pack_helper<OutputIterator, T, 0> {
	pack_helper(OutputIterator &&sink, T &&v)
	{
		custom<OutputIterator, T>::pack(
			std::forward<OutputIterator>(sink),
			std::forward<T>(v)
		);
	}
};

template <typename OutputIterator, typename T>
struct pack_helper<OutputIterator, T, kind_flags::sequence> {
	pack_helper(OutputIterator &&sink, T &&v)
	{
		if (v.empty())
			return;

		*sink++ = detail::tuple_start_code;

		constexpr auto next_kind(detail::classify<
			typename T::value_type, 0,
			yesod::is_sequence<typename T::value_type>::value
		>::value);

		for (auto xv: v)
			pack_helper<OutputIterator, T, next_kind>(
				std::forward<OutputIterator>(sink),
				std::forward<T>(v)
			);

		*sink++ = detail::tuple_end_code;
	}
};

template <typename OutputIterator, typename T>
struct pack_helper<OutputIterator, T, kind_flags::integral> {
	pack_helper(OutputIterator &&sink, T &&v)
	{
		typedef typename std::remove_reference<T>::type Tr;
		constexpr auto n_rank(
			std::is_signed<Tr>::value
			? numeric_type_rank::n_signed
			: numeric_type_rank::n_unsigned
		);
		auto s_rank(scalar_rank::from_value(v));

		if (s_rank == scalar_rank::ie)
			return;

		if (s_rank > scalar_rank::i4) {
			*sink++ = list_code[n_rank]
					   [list_size_rank::l3]
					   [s_rank - 1];
			pack_integral(
				std::forward<OutputIterator>(sink),
				std::forward<T>(v),
				scalar_rank::order[s_rank] / 8
			);
		} else if (s_rank == scalar_rank::i4)
			*sink++ = (v & small_int_mask) + (
				n_rank == numeric_type_rank::n_signed
				? small_int_code_offset
				: small_uint_code_offset
			);
	}
};

template <typename OutputIterator, typename T>
struct pack_helper<OutputIterator, T, kind_flags::float_t> {
	pack_helper(OutputIterator &&sink, T &&v)
	{
		typedef typename yesod::fp_adapter_type<T>::type Tw;
		constexpr auto s_rank(
			scalar_rank::from_type<typename Tw::storage_type>()
		);
		*sink++ = list_code[numeric_type_rank::n_float]
				   [list_size_rank::l3]
				   [s_rank - 1];
		pack_integral<scalar_rank::order[s_rank] / 8>(
			std::forward<OutputIterator>(sink),
			Tw(v).get_storable()
		);
	}
};

template <typename OutputIterator, typename T>
struct pack_helper<
	OutputIterator, T, kind_flags::integral | kind_flags::sequence
> {
	pack_helper(OutputIterator &&sink, T &&v)
	{
		typedef typename std::remove_reference<T>::type Tr;
		auto sz(v.size());

		if (!sz)
			return;

		constexpr auto n_rank(
			std::is_signed<typename Tr::value_type>::value
			? numeric_type_rank::n_signed
			: numeric_type_rank::n_unsigned
		);
		auto l_rank(list_size_rank::from_size(sz));
		constexpr auto s_rank(
			scalar_rank::from_type<typename Tr::value_type>()
		);

		if (s_rank == scalar_rank::ie)
			return;

		if (l_rank > list_size_rank::l3) {
			*sink++ = list_code[n_rank][l_rank][s_rank - 1];
			pack_integral(
				std::forward<OutputIterator>(sink),
				sz - 1, l_rank
			);
		} else if (l_rank == list_size_rank::l3)
			*sink++ = list_code[n_rank][l_rank][s_rank - 1]
				  | uint8_t(sz - 1);
		else
			return;

		for (auto xv: v)
			pack_integral<scalar_rank::order[s_rank] / 8>(
				std::forward<OutputIterator>(sink), xv
			);
	}
};

template <typename OutputIterator, typename T>
struct pack_helper<
	OutputIterator, T, kind_flags::float_t | kind_flags::sequence
> {
	pack_helper(OutputIterator &&sink, T &&v)
	{
		typedef typename std::remove_reference<T>::type Tr;
		typedef typename yesod::fp_adapter_type<
			typename Tr::value_type
		>::type Tw;
		auto sz(v.size());

		if (!sz)
			return;

		auto l_rank(list_size_rank::from_size(sz));
		constexpr auto s_rank(
			scalar_rank::from_type<typename Tw::storage_type>()
		);

		if (l_rank > list_size_rank::l3) {
			*sink++ = list_code[numeric_type_rank::n_float]
					   [l_rank]
					   [s_rank - 1];
			pack_integral(
				std::forward<OutputIterator>(sink),
				sz - 1, l_rank
			);
		} else if (l_rank == list_size_rank::l3)
			*sink++ = list_code[numeric_type_rank::n_float]
					   [l_rank]
					   [s_rank - 1]
				  | uint8_t(sz - 1);
		else
			return;

		for (auto xv: v)
			pack_integral<scalar_rank::order[s_rank] / 8>(
				std::forward<OutputIterator>(sink),
				Tw(xv).get_storable()
			);
	}
};

template <typename OutputIterator, typename T>
void pack(OutputIterator &&sink, T &&v)
{
	typedef typename std::remove_reference<T>::type Tr;
	pack_helper<
		OutputIterator, T, detail::classify<
			Tr, 0, yesod::is_sequence<Tr>::value
		>::value
	>(std::forward<OutputIterator>(sink), std::forward<T>(v));
}

template <typename OutputIterator, typename T, typename ...Tn>
void pack(OutputIterator &&sink, T &&v, Tn &&...vn)
{
	pack(std::forward<OutputIterator>(sink), std::forward<T>(v));
	pack(std::forward<OutputIterator>(sink), std::forward<Tn>(vn)...);
}

}
}}}
#endif
