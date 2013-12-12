/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_YESOD_TEST_DEMANGLE_DEC_12_2013_1800)
#define UCPF_YESOD_TEST_DEMANGLE_DEC_12_2013_1800

#include <typeinfo>
#include <cxxabi.h>

namespace ucpf { namespace yesod { namespace test {

template <typename T>
std::string demangle()
{
	auto *s(abi::__cxa_demangle(typeid(T).name(), 0, 0, 0));
	std::string rv(s);
	free(s);
	return rv;
}

}}}

#endif
