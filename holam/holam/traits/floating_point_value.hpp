/*
 * Copyright (c) 2016 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_D000D5E3C92C841ABA3E31D67C1D8F74)
#define HPP_D000D5E3C92C841ABA3E31D67C1D8F74

namespace ucpf { namespace holam { namespace traits {

template <typename T>
struct floating_point_value {
	typedef std::true_type disabled;

	static double apply(T &&val)
	{
		return 0.0;
	}
};

}}}
#endif
