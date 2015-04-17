/*
 * Copyright (c) 2014-2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

extern "C" {

#include <sys/socket.h>

}

#include "io_socket_so.hpp"
#include <yesod/mpl/value_transform.hpp>
#include <zivug/io/address_family.hpp>

#if !defined(AF_IB)
#define AF_IB 27
#endif

namespace ucpf { namespace zivug { namespace io {

template <::sa_family_t AddrFamily>
struct address_family_inst : address_family<descriptor> {};

extern template struct address_family_inst<AF_INET>;
extern template struct address_family_inst<AF_INET6>;

}}}

namespace mpl = ucpf::yesod::mpl;

using ucpf::zivug::io::descriptor;
using ucpf::zivug::io::address_family;
using ucpf::zivug::io::address_family_inst;

namespace {

#include "symbols/address_family_map.hpp"
#include "symbols/address_family_list.hpp"
#include "symbols/socket_type_map.hpp"
#include "symbols/socket_type_list.hpp"

template <::sa_family_t AddrFamily>
struct family_entry {
	constexpr static address_family_inst<AddrFamily> af = {};
	constexpr static address_family<descriptor> const *impl = &af;
};

template <::sa_family_t AddrFamily>
constexpr address_family_inst<AddrFamily> family_entry<AddrFamily>::af;

template <typename T, T const &v>
struct deref_inst {
	template <typename Ix, Ix... Cn>
	struct  apply {
		typedef mpl::package_c<
			address_family<descriptor> const * const *,
			&family_entry<v[Cn]>::impl...
		> type;
	};
};

typedef typename mpl::value_transform<
	decltype(address_family_list::list), address_family_list::list,
	deref_inst
>::value_type registry_af;

}

namespace ucpf { namespace zivug { namespace io {

template<>
std::pair<
	descriptor, address_family<descriptor> const *
> address_family<descriptor>::make_descriptor(
	char const *first, char const *last
)
{
	/* family.type || family.type.protocol */
	auto af_last(first);
	while ((af_last != last) && (*af_last != '.'))
		++af_last;

	auto idx(address_family_map::find(first, af_last));
	if (!idx)
		throw std::system_error(
			EAFNOSUPPORT, std::system_category()
		);

	auto af(*registry_af::value[idx - 1]);

	auto type_first(af_last);
	if (af_last != last)
		++type_first;
	else
		throw std::system_error(EINVAL, std::system_category());

	auto type_last(type_first);
	while ((type_last != last) && (*type_last != '.'))
		++type_last;

	idx = socket_type_map::find(type_first, type_last);
	if (!idx)
		throw std::system_error(
			ESOCKTNOSUPPORT, std::system_category()
		);

	auto s_type(socket_type_list[idx - 1]);

	auto proto_first(type_last);
	if (proto_first != last)
		++proto_first;

	return std::make_pair(af->create(s_type, proto_first, last), af);
}

template <>
void address_family<descriptor>::set_option(
	descriptor const &d, char const *first, char const *last
) const
{
	/* socket_level.option = val */
	auto sol_last(first);
	while ((sol_last != last) && (*sol_last != '.'))
		++sol_last;
	auto sol(detail::socket_level_base::level_from_string(first, sol_last));

	auto opt_first(sol_last);
	if (opt_first != last)
		++opt_first;

	auto opt_last(opt_first);
	while (
		(opt_last != last)
		&& !std::isspace(*opt_last)
		&& (*opt_last != '=')
	)
		++opt_last;
	auto opt(sol->option_from_string(opt_first, opt_last));

	auto val_first(opt_last);
	if (val_first != last)
		++val_first;

	while ((val_first != last) && std::isspace(*val_first))
		++val_first;

	opt->set(d, val_first, last);
}

}}}
