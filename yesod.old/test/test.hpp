/*
 * Copyright (c) 2013-2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_C3C4CF14353A8C6CD6C45E744924BB3B)
#define HPP_C3C4CF14353A8C6CD6C45E744924BB3B

#include <typeinfo>
#include <cxxabi.h>

namespace ucpf { namespace yesod { namespace test {

template <typename T>
std::string demangle()
{
	auto *s(abi::__cxa_demangle(typeid(T).name(), 0, 0, 0));
	std::string rv(s);
	if (std::is_const<T>::value)
		rv.append(" const");

	if (std::is_rvalue_reference<T>::value) {
		if (std::is_const<
			typename std::remove_reference<T>::type
		>::value)
			rv.append(" const");

		rv.append(" &&");
	} else if (std::is_lvalue_reference<T>::value) {
		if (std::is_const<
			typename std::remove_reference<T>::type
		>::value)
			rv.append(" const");

		rv.append(" &");
	}

	free(s);
	return rv;
}

template <typename NodeType, typename ValueGen, typename Traits>
NodeType *make_list(ValueGen &&g, Traits const &t = Traits())
{
	bool valid(false);
	auto v(g(valid));
	if (!valid)
		return nullptr;

	auto head(t.list_head(new NodeType(v)));
	auto tail(head);
	v = g(valid);
	while (valid) {
		tail = t.link_after(tail, new NodeType(v));
		v = g(valid);
	}

	return head;
}

}}}

#endif
