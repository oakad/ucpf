/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(HPP_DC2F14E6CDC5FC3F00E94D68412624CF)
#define HPP_DC2F14E6CDC5FC3F00E94D68412624CF

#include <zuvug/arch/io_event_dispatcher.hpp>

namespace ucpf { namespace zivug { namespace io {

struct server : notification {
	template <typename ConfigType>
	struct server(
		event_dispatcher &disp_, ConfigType const &config
	)
	: d(socket_configurator::create(config.server)),
	  disp(disp_)
	{
		src_disp.reset_read(d, *this);

		auto rv(::listen(d.native(), config.server.listen_backlog));
		if (rv < 0)
			throw std::system_error(errno, std::system_category());
	}

	virtual void read_ready(bool out_of_band, bool priority);

	virtual void error(bool priority);

	virtual void hang_up(bool read_only);

private:
	descriptor d;
	event_dispatcher &disp;
};

}}}

#endif
