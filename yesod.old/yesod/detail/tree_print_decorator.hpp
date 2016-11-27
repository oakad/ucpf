/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_YESOD_DETAIL_TREE_PRINT_DECORATOR_20140929T1545)
#define UCPF_YESOD_DETAIL_TREE_PRINT_DECORATOR_20140929T1545

#include <string>

namespace ucpf { namespace yesod { namespace detail {

template <typename Printer, typename Alloc = std::allocator<void>>
struct tree_print_decorator {
	tree_print_decorator(Printer &p_, Alloc const &a = Alloc())
	: p(p_), prefix(a)
	{}

	void push_level()
	{
		if ((prefix.size() > 3) && (*(prefix.end() - 4) == '`')) {
			prefix.erase(prefix.end() - 4, prefix.end());
			prefix.append("    ");
		} else
			prefix.append("|   ");
	}

	void pop_level()
	{
		if ((prefix.size() > 3) && (*(prefix.end() - 4) == '`'))
			prefix.erase(prefix.end() - 4, prefix.end());

		if (prefix.size() > 3)
			prefix.erase(prefix.end() - 4, prefix.end());
	}

	void next_child()
	{
		prefix.append("|-- ");

		p(prefix.c_str());

		prefix.erase(prefix.end() - 4, prefix.end());
	}

	void last_child()
	{
		prefix.append("`-- ");

		p(prefix.data());
	}

	Printer &p;
	std::basic_string<char, std::char_traits<char>, Alloc> prefix;
};
}}}
#endif
