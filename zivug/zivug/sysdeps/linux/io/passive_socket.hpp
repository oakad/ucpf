/*
 * Copyright (c) 2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_EAAD16E6594FF081FA6DFA928CBEAEA7)
#define HPP_EAAD16E6594FF081FA6DFA928CBEAEA7

namespace ucpf { namespace zivug { namespace io {

struct passive_socket {
	template <ConfigType>
	static passive_socket *make(ConfigType const &cfg)
	{
	}
};

}}}
#endif
