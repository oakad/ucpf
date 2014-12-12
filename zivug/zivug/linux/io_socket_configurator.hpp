/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(HPP_5AF9A5BAC9C9C8F182489F819FA23DCF)
#define HPP_5AF9A5BAC9C9C8F182489F819FA23DCF

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
		char const *first, char const *last, void const **ctx
	);

	static void apply_setting(
		descriptor &d, char const *first, char const *last,
		void const *ctx
	);
};

}}}

#endif
