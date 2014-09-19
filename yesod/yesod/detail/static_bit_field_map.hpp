/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_YESOD_DETAIL_STATIC_BIT_FIELD_MAP_20140919T2300)
#define UCPF_YESOD_DETAIL_STATIC_BIT_FIELD_MAP_20140919T2300

#include <yesod/mpl/package.hpp>
#include <yesod/mpl/value_cast.hpp>

namespace ucpf { namespace yesod { namespace detail {

template <std::size_t BitCount, std::size_t... Fn>
struct static_bit_field_map {
	template <std::size_t C>
	using num = std::integral_constant<std::size_t, C>;

	template <std::size_t Bc, std::size_t Sc>
	struct entry {
		constexpr static std::pair<
			std::size_t, std::size_t
		> value = {Bc, Sc};
	};

	typedef typename std::conditional<
		sizeof...(Fn),
		yesod::mpl::package<num<Fn>...>,
		yesod::mpl::package<num<BitCount>>
	>::type f_pack;

	template <
		std::size_t Bc, std::size_t Sc, std::size_t F,
		typename Pack, typename... Tn
	> struct apply;

	template <std::size_t F, typename Pack, typename... Tn>
	struct apply<0, BitCount, F, Pack, yesod::mpl::package<Tn...>> {
		typedef entry<0, BitCount> last;
		typedef yesod::mpl::package<Tn...> type;
	};

	template <
		std::size_t Bc, std::size_t Sc, std::size_t F, typename... Tn
	> struct apply<
		Bc, Sc, F, yesod::mpl::package<>, yesod::mpl::package<Tn...>
	> {
		typedef entry<F <= Bc ? F : F - Bc, Sc> last;
		typedef typename yesod::mpl::push_back<
			yesod::mpl::package<Tn...>, last
		>::type type;
	};

	template <
		std::size_t Bc, std::size_t Sc, std::size_t F,
		typename Pack, typename... Tn
	> struct apply<Bc, Sc, F, Pack, yesod::mpl::package<Tn...>> : apply<
		F <= Bc ? Bc - F : 0,
		F <= Bc ? Sc + F : BitCount,
		yesod::mpl::front<Pack>::type::value,
		typename yesod::mpl::pop_front<Pack>::type,
		typename yesod::mpl::push_back<
			yesod::mpl::package<Tn...>,
			entry<F <= Bc ? F : F - Bc, Sc>
		>::type
	> {};

	typedef apply<
		BitCount, 0, yesod::mpl::front<f_pack>::type::value,
		typename yesod::mpl::pop_front<f_pack>::type,
		yesod::mpl::package<>
	> x_type;

	typedef typename x_type::type type;

	typedef yesod::mpl::value_cast<
		std::pair<std::size_t, std::size_t>, type
	> value_type;

	struct repeat_last {
		typedef typename static_bit_field_map::x_type base_pack;
		typedef typename x_type::last last;

		template <
			std::size_t Bc, std::size_t Sc, std::size_t F,
			typename... Tn
		> struct apply;

		template <std::size_t F, typename... Tn>
		struct apply<0, BitCount, F, yesod::mpl::package<Tn...>> {
			typedef yesod::mpl::package<Tn...> type;
		};

		template <
			std::size_t Bc, std::size_t Sc, std::size_t F,
			typename... Tn
		> struct apply<Bc, Sc, F, yesod::mpl::package<Tn...>> : apply<
			F <= Bc ? Bc - F : 0,
			F <= Bc ? Sc + F : BitCount, F,
			typename yesod::mpl::push_back<
				yesod::mpl::package<Tn...>,
				entry<F <= Bc ? F : Bc, Sc>
			>::type
		> {};

		typedef typename apply<
			BitCount - last::value.first - last::value.second,
			last::value.first + last::value.second,
			last::value.first, typename base_pack::type
		>::type type;

		typedef yesod::mpl::value_cast<
			std::pair<std::size_t, std::size_t>, type
		> value_type;
	};
};

}}}

#endif
