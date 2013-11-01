/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_RASHAM_RASHAM_OCT_29_2013_1750)
#define UCPF_RASHAM_RASHAM_OCT_29_2013_1750

#include <cstdio>
#include <utility>

namespace ucpf { namespace rasham {
namespace detail {

struct arg_pack;

bool suppress_location(void *loc, std::thread::id thr);

}

struct rshm {
	rshm(rshm const &) = delete;
	rshm const &operator=(rshm const &) = delete;

	template <typename... Args>
	__attribute__ ((noinline)) rshm(Args... a)
	: suppress(false)
	{
		auto loc(__builtin_extract_return_addr(
			__builtin_return_address(0)
		));
		auto thr(std::this_thread::get_id());
		suppress = detail::suppress_location(loc, thr);
	}

	template <long N, typename... Args>
	rshm &fmt(Args... a)
	{
		if (suppress)
			return this;

		printf("fmt arg %ld\n", N);
		return *this;
	}

	template <typename... Args>
	rshm &dst(Args... a)
	{
		if (suppress)
			return this;

		printf("dst\n");
		return *this;
	}

	~rshm()
	{
		if (suppress)
			return;

		printf("rshm destroyed\n");
	}

private:
	bool suppress;
	arg_pack *args;
};

}}

#endif
