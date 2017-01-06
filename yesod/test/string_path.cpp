/*
 * Copyright (c) 2016 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#define BOOST_TEST_MODULE yesod
#include <boost/test/unit_test.hpp>

#include "test.hpp"
#include <yesod/string_path.hpp>
#include <yesod/string_utils/u8string_tokenizer_sce.hpp>

namespace ucpf::yesod {

BOOST_AUTO_TEST_CASE(string_path_0)
{
	std::array<std::string, 3> p0_ref{
		std::string("aaa"), 
		std::string("bbb"),
		std::string("ccc")
	};
	string_path p0(string_path::from_posix_like_string("aaa/bbb/ccc"));
	BOOST_CHECK_EQUAL(p0.size(), p0_ref.size());
	for (auto c(0); c < p0_ref.size(); c++) {
		BOOST_CHECK_EQUAL(p0.at(c), p0_ref[c]);
	}

	std::array<std::string, 7> p1_ref{
		std::string("aaaaa"), 
		std::string("bbbbb"),
		std::string("ccccc"),
		std::string("ddddd"), 
		std::string("eeeee"),
		std::string("fffff"),
		std::string("ggggg")
	};
	string_path p1(string_path::from_posix_like_string(
		"aaaaa/bbbbb/ccccc/ddddd/eeeee/fffff/ggggg"
	));
	BOOST_CHECK_EQUAL(p1.size(), p1_ref.size());
	for (auto c(0); c < p1_ref.size(); c++) {
		BOOST_CHECK_EQUAL(p1.at(c), p1_ref[c]);
	}
}

}
