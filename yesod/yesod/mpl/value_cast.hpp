/*
 * Copyright (c) 2013-2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_YESOD_MPL_VALUE_CAST_DEC_17_2013_1730)
#define UCPF_YESOD_MPL_VALUE_CAST_DEC_17_2013_1730

#include <array>

#include <yesod/mpl/copy.hpp>
#include <yesod/mpl/package.hpp>
#include <yesod/mpl/inserter.hpp>

namespace ucpf { namespace yesod { namespace mpl {

template <typename...>
struct value_cast;

template <typename T, typename... Tn>
struct value_cast<T, package<Tn...>> {
	template <typename... Un>
	struct array_values {
		static constexpr std::array<
			T, sizeof...(Un)
		> value = {{
			Un::value...
		}};
	};

	struct apply_values {
		template <typename... Un>
		struct apply {
			typedef array_values<Un...> type;
		};
	};

	typedef package<Tn...> package_type;

	typedef typename apply_pack<
		package_type, apply_values::template apply
	>::type array_type;

	static constexpr decltype(array_type::value) value = array_type::value;
};

template <typename T, typename... Tn>
constexpr decltype(value_cast<T, package<Tn...>>::array_type::value)
value_cast<T, package<Tn...>>::value;

template <typename T, T... Tn>
struct value_cast<package_c<T, Tn...>> {
	template <typename U, U... Un>
	struct array_values {
		static constexpr std::array<
			U, sizeof...(Un)
		> value = {{
			Un...
		}};
	};

	struct apply_values {
		template <typename U, U... Un>
		struct apply {
			typedef array_values<U, Un...> type;
		};
	};

	typedef package_c<T, Tn...> package_type;

	typedef typename apply_pack_c<
		package_type, apply_values::template apply
	>::type array_type;

	static constexpr decltype(array_type::value) value = array_type::value;
};

template <typename T, T... Tn>
constexpr decltype(value_cast<package_c<T, Tn...>>::array_type::value)
value_cast<package_c<T, Tn...>>::value;

template <typename Sequence>
struct value_cast<Sequence> : value_cast<
	typename Sequence::value_type,
	typename copy<Sequence, back_inserter<package<>>>::type
> {};

}}}

#endif
