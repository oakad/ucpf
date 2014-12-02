/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_ZIVUG_IO_SOCKET_CONFIGURATOR_20141201T2300)
#define UCPF_ZIVUG_IO_SOCKET_CONFIGURATOR_20141201T2300

namespace ucpf { namespace zivug { namespace io {
namespace detail {

struct socket_option_symbol {
};


}

struct socket_configurator {
	template <typename ConfiType>
	static descriptor create_server(ConfiType const& config)
	{
	}

};

}}}

#endif

