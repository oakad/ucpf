/*
 * Copyright (c) 2014-2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#include "io_socket_so.hpp"

#include <yesod/mpl/at.hpp>
#include <yesod/mpl/map.hpp>
#include <yesod/mpl/has_key.hpp>
#include <yesod/mpl/value_transform.hpp>

#if !defined(SO_BPF_EXTENSIONS)
#define SO_BPF_EXTENSIONS 48
#endif

#if !defined(SO_INCOMING_CPU)
#define SO_INCOMING_CPU 49
#endif

#if !defined(SO_ATTACH_BPF)
#define SO_ATTACH_BPF 50
#endif

#if !defined(SO_DETACH_BPF)
#define SO_DETACH_BPF SO_DETACH_FILTER
#endif

namespace mpl = ucpf::yesod::mpl;

using ucpf::zivug::io::detail::string_tag;
using ucpf::zivug::io::detail::option_base;
using ucpf::zivug::io::detail::option_entry;

namespace {

#include "symbols/sol_socket_option_map.hpp"
#include "symbols/sol_socket_option_list.hpp"
#include "symbols/sol_socket_option_type_map.hpp"

template <typename T, T const &v>
struct deref_inst {
	template <typename Ix, Ix... Cn>
	struct  apply {
		typedef mpl::package_c<
			option_base const * const *,
			&option_entry<
				typename std::conditional<
					mpl::has_key<
						sol_socket_option_type_map,
						mpl::int_<v[Cn]>
					>::value,
					typename mpl::at<
						sol_socket_option_type_map,
						mpl::int_<v[Cn]>
					>::type,
					sol_socket_option_default_type
				>::type,
				SOL_SOCKET, v[Cn]
			>::impl...
		> type;
	};
};

typedef typename mpl::value_transform<
	decltype(sol_socket_option_list::list), sol_socket_option_list::list,
	deref_inst
>::value_type registry;

}

namespace ucpf { namespace zivug { namespace io { namespace detail {

template <int Level>
struct socket_level : socket_level_base {};

template <>
struct socket_level<SOL_SOCKET> : socket_level_base {
	virtual option_base const *option_from_string(
		char const *first, char const *last
	) const;
};

option_base const *socket_level<SOL_SOCKET>::option_from_string(
	char const *first, char const *last
) const
{
	auto idx(sol_socket_option_map::find(first, last));

	if (idx)
		return *registry::value[idx - 1];
	else
		throw std::system_error(ENOPROTOOPT, std::system_category());
}

}}}}
