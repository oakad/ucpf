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

struct socket_configurator {
	template <typename ConfiType>
	static descriptor create_server(ConfiType const& config)
	{
		void *ctx;

		descriptor d(make_descriptor(
			std::begin(config.type), std::end(config.type), &ctx
		));

		for (auto &opt: config.settings)
			apply_setting(d, std::begin(opt), std::end(opt), ctx);

		return d;
	}

private:
	static descriptor make_descriptor(
		char const *type_first, char const *type_last,
		void const **ctx
	);

	static void apply_setting(
		descriptor &d, char const *s_first, char const *s_last,
		void const *ctx
	);
};

}}}

#endif
