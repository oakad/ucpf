/*
 * Copyright (c) 2014-2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#include "io_socket_so.hpp"

namespace ucpf { namespace zivug { namespace io { namespace detail {

template <>
struct option_level<SOL_SOCKET> : option_level_base {
	constexpr static int level = SOL_SOCKET;

	virtual void set(
		descriptor const &d, char const *first, char const *last
	) const
	{
		/* option = val */
		auto name_last(first);
		for (; name_last != last; ++name_last)
			if (*name_last == '=')
				break;

		if (name_last == last)
			throw std::system_error(
				EINVAL, std::system_category()
			);

		auto val_first(name_last + 1);
		for (; val_first != last; ++val_first)
			if (!std::isspace(*val_first))
				break;

		if (val_first == last)
			throw std::system_error(
				EINVAL, std::system_category()
			);

		for (; name_last > first; --name_last)
			if (!std::isspace(*(name_last - 1)))
				break;

		if (name_last == first)
			throw std::system_error(
				EINVAL, std::system_category()
			);

		auto idx(sol_socket_option_map::find(first, name_last));
		if (idx)
			sock_option[idx - 1]->set(d, val_first, last);
		else
			throw std::system_error(
				ENOPROTOOPT, std::system_category()
			);
	}

private:
	constexpr static option_base *sock_option[] = {
		option_entry<bool, level, SO_DEBUG>::impl,
		option_entry<bool, level, SO_REUSEADDR>::impl,
		option_entry<int, level, SO_TYPE>::impl,
		option_entry<int, level, SO_ERROR>::impl,
		option_entry<bool, level, SO_DONTROUTE>::impl,
		option_entry<bool, level, SO_BROADCAST>::impl,
		option_entry<int, level, SO_SNDBUF>::impl,
		option_entry<int, level, SO_RCVBUF>::impl,
		option_entry<int, level, SO_SNDBUFFORCE>::impl,
		option_entry<int, level, SO_RCVBUFFORCE>::impl,
		option_entry<bool, level, SO_KEEPALIVE>::impl,
		option_entry<bool, level, SO_OOBINLINE>::impl,
		option_entry<bool, level, SO_NO_CHECK>::impl,
		option_entry<int, level, SO_PRIORITY>::impl,
		option_entry<::linger, level, SO_LINGER>::impl,
		option_entry<void, level, SO_BSDCOMPAT>::impl,
		option_entry<bool, level, SO_REUSEPORT>::impl
		option_entry<bool, level, SO_PASSCRED>::impl,
		option_entry<::ucred, level, SO_PEERCRED>::impl,
		option_entry<int, level, SO_RCVLOWAT>::impl,
		option_entry<int, level, SO_SNDLOWAT>::impl,
		option_entry<::timeval, level, SO_RCVTIMEO>::impl,
		option_entry<::timeval, level, SO_SNDTIMEO>::impl,
		option_entry<void, level, SO_SECURITY_AUTHENTICATION>::impl,
		option_entry<void, level, SO_SECURITY_ENCRYPTION_TRANSPORT>::impl,
		option_entry<void, level, SO_SECURITY_ENCRYPTION_NETWORK>::impl,
		option_entry<string_tag, level, SO_BINDTODEVICE>::impl,
		option_entry<::sock_fprog, level, SO_ATTACH_FILTER>::impl,
		option_entry<bool, level, SO_DETACH_FILTER>::impl,
		option_entry<::sock_fprog, level, SO_GET_FILTER>::impl,
		option_entry<string_tag, level, SO_PEERNAME>::impl,
		option_entry<bool, level, SO_TIMESTAMP>::impl,
		option_entry<int, level, SO_ACCEPTCONN>::impl,
		option_entry<string_tag, level, SO_PEERSEC>::impl,
		option_entry<bool, level, SO_PASSSEC>::impl,
		option_entry<bool, level, SO_TIMESTAMPNS>::impl,
		option_entry<int, level, SO_MARK>::impl,
		option_entry<int, level, SO_TIMESTAMPING>::impl,
		option_entry<int, level, SO_PROTOCOL>::impl,
		option_entry<int, level, SO_DOMAIN>::impl,
		option_entry<bool, level, SO_RXQ_OVFL>::impl,
		option_entry<bool, level, SO_WIFI_STATUS>::impl,
		option_entry<int, level, SO_PEEK_OFF>::impl,
		option_entry<bool, level, SO_NOFCS>::impl,
		option_entry<bool, level, SO_LOCK_FILTER>::impl,
		option_entry<bool, level, SO_SELECT_ERR_QUEUE>::impl,
		option_entry<int, level, SO_BUSY_POLL>::impl,
		option_entry<int, level, SO_MAX_PACING_RATE>::impl,
		option_entry<int, level, SO_BPF_EXTENSIONS>::impl,
		option_entry<int, level, SO_INCOMING_CPU>::impl,
		option_entry<int, level, SO_ATTACH_BPF>::impl,
		option_entry<bool, level, SO_DETACH_BPF>::impl
	};
};

template <>
constexpr option_base *option_level<SOL_SOCKET>::sock_option[];

}}}}
