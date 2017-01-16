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
	BOOST_TEST(p0.size(), p0_ref.size());
	BOOST_TEST(p0 == p0_ref, boost::test_tools::per_element());
	BOOST_TEST(std::equal(
		std::make_reverse_iterator(p0.end()),
		std::make_reverse_iterator(p0.begin()),
		std::make_reverse_iterator(p0_ref.end()),
		std::make_reverse_iterator(p0_ref.begin())
	));

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
	BOOST_TEST(p1.size(), p1_ref.size());
	BOOST_TEST(p1 == p1_ref, boost::test_tools::per_element());
	BOOST_TEST(std::equal(
		std::make_reverse_iterator(p0.end()),
		std::make_reverse_iterator(p0.begin()),
		std::make_reverse_iterator(p0_ref.end()),
		std::make_reverse_iterator(p0_ref.begin())
	));
}

BOOST_AUTO_TEST_CASE(string_path_1)
{
	string_path p0(string_path::from_posix_like_string("aaa/bbb/ccc"));
	string_path p1(string_path::from_posix_like_string("ddd/eee/fff"));
	string_path p2(string_path::from_posix_like_string("ggg/hhh/iii"));
	string_path p_ref(string_path::from_posix_like_string(
		"aaa/bbb/ccc/ddd/eee/fff/ggg/hhh/iii"
	));
	BOOST_TEST(string_path::cat(p0, p1, p2) == p_ref);
}

BOOST_AUTO_TEST_CASE(string_path_2)
{
	string_path p0(string_path::from_posix_like_string(
		"aaa/bbb/ccc/ddd/eee"
	));
	string_path p1a(string_path::from_posix_like_string("aaa/bbb"));
	string_path p1b(string_path::from_posix_like_string("aaabbb"));
	string_path p1c(string_path::from_posix_like_string("aaa/bbbccc"));
	string_path p2a(string_path::from_posix_like_string("ddd/eee"));
	string_path p2b(string_path::from_posix_like_string("dddeee"));
	string_path p2c(string_path::from_posix_like_string("cccddd/eee"));

	BOOST_TEST(p0.common_head_size(p0) == 5);
	BOOST_TEST(p0.common_tail_size(p0) == 5);

	BOOST_TEST(p0.common_head_size(p1a) == 2);
	BOOST_TEST(p0.common_head_size(p1b) == 0);
	BOOST_TEST(p0.common_head_size(p1c) == 1);

	BOOST_TEST(p0.common_tail_size(p2a) == 2);
	BOOST_TEST(p0.common_tail_size(p2b) == 0);
	BOOST_TEST(p0.common_tail_size(p2c) == 1);
}
}
