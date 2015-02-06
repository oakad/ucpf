/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(HPP_5AF9A5BAC9C9C8F182489F819FA23DCF)
#define HPP_5AF9A5BAC9C9C8F182489F819FA23DCF

#include <zivug/arch/io_event_dispatcher.hpp>

namespace ucpf { namespace zivug { namespace io {

struct socket_configurator {
	template <typename ConfiType>
	static descriptor make_server(ConfiType const& config)
	{
		void const *ctx;

		descriptor d(make_descriptor(
			std::begin(config.protocol).base(),
			std::end(config.protocol).base(),
			&ctx
		));

		for (auto const &opt: config.pre_bind_options)
			set_option(
				d, std::begin(opt).base(),
				std::end(opt).base(), ctx
			);

		bind(
			d, std::begin(config.bind_address).base(),
			std::end(config.bind_address).base(), ctx
		);

		for (auto const &opt: config.post_bind_options)
			set_option(
				d, std::begin(opt).base(),
				std::end(opt).base(), ctx
			);

		return d;
	}

private:
	static descriptor make_descriptor(
		char const *first, char const *last, void const **ctx
	);

	static void set_option(
		descriptor const &d, char const *first, char const *last,
		void const *ctx
	);

	static void bind(
		descriptor const &d, char const *first, char const *last,
		void const *ctx
	);
};

}}}

#endif
