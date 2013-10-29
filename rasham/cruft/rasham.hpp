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
#include <dlfcn.h>
#include <utility>

extern void* __etext;

namespace ucpf { namespace rasham {

struct rshm {
	template <typename... Args>
	__attribute__ ((noinline)) rshm(Args... a)
	{
		printf("exec %p\n", __etext);
		auto a1(__builtin_return_address(0));
		auto a2(__builtin_extract_return_addr(a1));
		printf("rshm constructed %p - %p\n", a1, a2);
		Dl_info info;
		printf("rshm addr %d\n", dladdr(a1, &info));
		printf("  fname %s\n", info.dli_fname);
		printf("  fbase %p\n", info.dli_fbase);
		printf("  sname %s\n", info.dli_sname);
		printf("  saddr %p\n", info.dli_saddr);
	}

	template <long N, typename... Args>
	rshm &fmt(Args... a)
	{
		printf("fmt arg %ld\n", N);
		return *this;
	}

	template <typename... Args>
	rshm &dst(Args... a)
	{
		printf("dst\n");
		return *this;
	}

	~rshm()
	{
		printf("rshm destroyed\n");
	}
};

}}

#endif
