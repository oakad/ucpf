/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#include <list>
#include <string>
#include <mina/detail/unescape_c.hpp>

int main(int argc, char **argv)
{
	std::string s_in;
	std::vector<int8_t> v_in;
	std::list<std::vector> l_in;

	while (std::getline(std::cin, s_in)) {
		if (s_in.empty())
			continue;

		if (!unescape(v_in, s_in))
			return -1;
		add_to_alphabet(v_in);
		l_in.emplace_back(std::move(v_in));
		v_in.clear();
		s_in.clear();
	}

	return 0;
}
