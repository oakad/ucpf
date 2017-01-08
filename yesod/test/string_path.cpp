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

using test::operator ""_us;

BOOST_AUTO_TEST_CASE(string_path_0)
{
	std::array<test::ustring, 3> p0_ref{
		test::ustring("aaa"_us),
		test::ustring("bbb"_us),
		test::ustring("ccc"_us)
	};
	string_path p0(string_path::from_posix_like_string("aaa/bbb/ccc"));
	BOOST_CHECK_EQUAL(p0.size(), p0_ref.size());
	for (size_t c(0); c < p0_ref.size(); c++) {
		BOOST_CHECK_EQUAL(p0.at(c).to_string(), p0_ref[c]);
	}

	std::array<test::ustring, 7> p1_ref{
		test::ustring("aaaaa"_us), 
		test::ustring("bbbbb"_us),
		test::ustring("ccccc"_us),
		test::ustring("ddddd"_us), 
		test::ustring("eeeee"_us),
		test::ustring("fffff"_us),
		test::ustring("ggggg"_us)
	};
	string_path p1(string_path::from_posix_like_string(
		"aaaaa/bbbbb/ccccc/ddddd/eeeee/fffff/ggggg"
	));
	BOOST_CHECK_EQUAL(p1.size(), p1_ref.size());
	for (size_t c(0); c < p1_ref.size(); c++) {
		BOOST_CHECK_EQUAL(p1.at(c).to_string(), p1_ref[c]);
	}
}

}
