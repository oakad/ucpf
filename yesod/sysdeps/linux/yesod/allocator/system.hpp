/*
 * Copyright (c) 2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_330385F083C459F3D0A781A50CEB6128)
#define HPP_330385F083C459F3D0A781A50CEB6128

extern "C" {

#include <unistd.h>
#include <sys/mman.h>

}

#include <system_error>

namespace ucpf { namespace yesod { namespace allocator {

struct system {
	static std::size_t alloc_unit_size()
	{
		return ::sysconf(_SC_PAGESIZE);
	}

	static void *allocate(std::size_t n, void const *hint = nullptr)
	{
		auto rv(::mmap(
			hint, n, PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_ANONYMOUS, -1, 0
		));
		if (rv == MAP_FAILED)
			throw std::system_error(errno, std::system_category());
		else
			return rv;
	}

	static void deallocate(void *p, std::size_t n)
	{
		auto rc(::munmap(p, n));
		if (rc < 0)
			throw std::system_error(errno, std::system_category());
	}
};

}}}
#endif
