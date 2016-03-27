/*
 * Copyright (c) 2014-2016 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_99464AC9C502434F21A2F25B74635BFE)
#define HPP_99464AC9C502434F21A2F25B74635BFE

#include <array>

namespace ucpf { namespace yesod { namespace detail {

template <std::size_t BitCount, bool RepeatLast, std::size_t... Fs>
struct static_bit_field_map {
	template <std::size_t...>
	struct pack {};

	template <typename...>
	struct value_pack_t;

	template <
		std::size_t... Ps, template <std::size_t...> class PackP,
		std::size_t... Cs, template <std::size_t...> class PackC
	>
	struct value_pack_t<PackP<Ps...>, PackC<Cs...>> {
		constexpr static std::array<
			std::pair<std::size_t, std::size_t>, sizeof...(Ps)
		> value = {{ std::pair<std::size_t, std::size_t>(Ps, Cs)... }};
	};

	template <std::size_t L, std::size_t R>
	struct min {
		typedef typename std::conditional<
			L <= R,
			std::integral_constant<std::size_t, L>,
			std::integral_constant<std::size_t, R>
		>::type type;
	};

	template <typename...>
	struct cum_sum;

	template <std::size_t... XFs, template <std::size_t...> class PackF>
	struct cum_sum<PackF<XFs...>> {
		template <std::size_t YF0, std::size_t... YFs>
		struct apply {
			typedef typename min<YF0, BitCount>::type C0;

			typedef typename cum_sum<
				PackF<YFs...>,
				PackF<0>,
				PackF<C0::value>,
				C0, std::integral_constant<
					bool, (C0::value < BitCount)
				>
			>::type type;
		};

		template <std::size_t YF0>
		struct apply<YF0> {
			typedef typename min<YF0, BitCount>::type C0;

			typedef typename cum_sum<
				PackF<0>,
				PackF<C0::value>,
				C0, std::integral_constant<std::size_t, YF0>,
				std::integral_constant<
					bool, (C0::value < BitCount)
				>
			>::type type;
		};

		typedef typename apply<XFs...>::type type;
	};

	template <
		std::size_t... XFs, template <std::size_t...> class PackF,
		std::size_t... Ps, template <std::size_t...> class PackP,
		std::size_t... Cs, template <std::size_t...> class PackC,
		typename NextP
	>
	struct cum_sum<
		PackF<XFs...>, PackP<Ps...>, PackC<Cs...>, NextP,
		std::true_type
	> {
		template <std::size_t YF0, std::size_t... YFs>
		struct apply {
			typedef typename min<
				YF0, BitCount - NextP::value
			>::type C0;
			typedef std::integral_constant<
				std::size_t, NextP::value + C0::value
			> next_pos;

			typedef typename cum_sum<
				PackF<YFs...>,
				PackP<Ps..., NextP::value>,
				PackC<Cs..., C0::value>,
				next_pos, std::integral_constant<
					bool, (next_pos::value < BitCount)
				>
			>::type type;
		};

		template <std::size_t YF0>
		struct apply<YF0> {
			typedef typename min<
				YF0, BitCount - NextP::value
			>::type C0;
			typedef std::integral_constant<
				std::size_t, NextP::value + C0::value
			> next_pos;

			typedef typename cum_sum<
				PackP<Ps..., NextP::value>,
				PackC<Cs..., C0::value>,
				next_pos, std::integral_constant<
					std::size_t, YF0
				>,
				std::integral_constant<
					bool, (next_pos::value < BitCount)
				>
			>::type type;
		};

		typedef typename apply<XFs...>::type type;
	};

	template <
		std::size_t... Ps, template <std::size_t...> class PackP,
		std::size_t... Cs, template <std::size_t...> class PackC,
		typename NextP, typename LastF
	>
	struct cum_sum<
		PackP<Ps...>, PackC<Cs...>, NextP, LastF,
		std::true_type
	> {
			typedef typename min<
				RepeatLast
				? LastF::value : BitCount - NextP::value,
				BitCount - NextP::value
			>::type C0;
			typedef std::integral_constant<
				std::size_t, NextP::value + C0::value
			> next_pos;

			typedef typename cum_sum<
				PackP<Ps..., NextP::value>,
				PackC<Cs..., C0::value>,
				next_pos, LastF,
				std::integral_constant<
					bool, (next_pos::value < BitCount)
				>
			>::type type;
	};

	template <
		std::size_t... XFs, template <std::size_t...> class PackF,
		std::size_t... Ps, template <std::size_t...> class PackP,
		std::size_t... Cs, template <std::size_t...> class PackC,
		typename NextP
	>
	struct cum_sum<
		PackF<XFs...>, PackP<Ps...>, PackC<Cs...>, NextP,
		std::false_type
	> {
		typedef value_pack_t<PackP<Ps...>, PackC<Cs...>> type;
	};

	template <
		std::size_t... Ps, template <std::size_t...> class PackP,
		std::size_t... Cs, template <std::size_t...> class PackC,
		typename NextP, typename LastF
	>
	struct cum_sum<
		PackP<Ps...>, PackC<Cs...>, NextP, LastF,
		std::false_type
	> {
		typedef value_pack_t<PackP<Ps...>, PackC<Cs...>> type;
	};

	typedef typename cum_sum<pack<Fs...>>::type value_type;

	static constexpr decltype(value_type::value) value = value_type::value;
};

template <std::size_t BitCount, bool RepeatLast, std::size_t... Fs>
template <
	std::size_t... Ps, template <std::size_t...> class PackP,
	std::size_t... Cs, template <std::size_t...> class PackC
>
constexpr std::array<
	std::pair<std::size_t, std::size_t>, sizeof...(Ps)
> static_bit_field_map<
	BitCount, RepeatLast, Fs...
>::value_pack_t<PackP<Ps...>, PackC<Cs...>>::value;

}}}

#endif
