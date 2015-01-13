/*
 * Copyright (c) 2014-2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#include <string>
#include <iostream>

#include "nom_test.hpp"

 int main(int argc, char **argv)
 {
 	std::string s_in;
 	int v_ref(1);

	while (std::getline(std::cin, s_in)) {
                if (s_in.empty())
                        continue;

                auto v_out(nom_test::find(s_in.begin(), s_in.end()));
                printf(
                	"(%d) lookup %s: got %d, expect %d\n",
                	v_out == v_ref, s_in.c_str(), v_out, v_ref
                );
                ++v_ref;
        }
        return 0;
 }
