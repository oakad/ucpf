/*
 * Copyright (c) 2014-2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#include "io_socket_so.hpp"
#include <yesod/mpl/value_transform.hpp>

namespace ucpf { namespace zivug { namespace io { namespace detail {

template <int Level>
struct socket_level : socket_level_base {};

extern template struct socket_level<SOL_SOCKET>;

}}}}

using ucpf::yesod::mpl::package_c;
using ucpf::yesod::mpl::value_transform;

using ucpf::zivug::io::detail::socket_level;
using ucpf::zivug::io::detail::socket_level_base;

namespace {

#include "symbols/socket_level_map.hpp"
#include "symbols/socket_level_list.hpp"

template <int Level>
struct socket_level_entry {
	constexpr static socket_level<Level> sol = {};
	constexpr static socket_level_base const *impl = &sol;
};

template <int Level>
constexpr socket_level<Level> socket_level_entry<Level>::sol;

template <typename T, T const &v>
struct deref_sol_inst {
	template <typename Ix, Ix... Cn>
	struct  apply {
		typedef package_c<
			socket_level_base const *,
			socket_level_entry<v[Cn]>::impl...
		> type;
	};
};

typedef typename value_transform<
	decltype(socket_level_list), socket_level_list, deref_sol_inst
>::value_type registry;

}

namespace ucpf { namespace zivug { namespace io { namespace detail {

socket_level_base const *socket_level_base::level_from_string(
	char const *first, char const *last
)
{
	auto idx(socket_level_map::find(first, last));
	if (idx)
		return registry::value[idx - 1];
	else
		throw std::system_error(
			ENOPROTOOPT, std::system_category()
		);
}

}}}}
