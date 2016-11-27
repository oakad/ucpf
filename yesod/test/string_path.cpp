/*
 * Copyright (c) 2016 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#define BOOST_TEST_MODULE yesod
#include <boost/test/unit_test.hpp>

#include <yesod/string_path.hpp>
#include <yesod/string_utils/u8string_tokenizer_sce.hpp>

namespace ucpf::yesod {

BOOST_AUTO_TEST_CASE(string_path_0)
{
	string_path p0(string_path::from_posix_like_string("aaa/bbb/ccc"));
}

}
