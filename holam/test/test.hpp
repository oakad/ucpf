/*
 * Copyright (c) 2016 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_0C2342938587191297479B38FA7F457F)
#define HPP_0C2342938587191297479B38FA7F457F

#include <holam/output/collector.hpp>

namespace ucpf { namespace holam {
namespace output {

template <typename ValueType, std::size_t BlockSize>
std::ostream &operator<<(
	std::ostream &os, collector<ValueType, BlockSize> const &cl
)
{
	cl.apply([&os](ValueType const *p, std::size_t sz) {
		for (; sz; --sz, ++p)
			os << *p;
	});
	return os;
}

}
}}
#endif
