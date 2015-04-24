/*
 * Copyright (c) 2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#define BOOST_TEST_MODULE yesod_iterator
#include <boost/test/unit_test.hpp>
#include <boost/test/output_test_stream.hpp>

#include <fcntl.h>
#include <system_error>
#include <cstdio>

#include <yesod/iterator/induced_range.hpp>

namespace ucpf { namespace yesod { namespace iterator {
namespace test {

struct fd_reader {
	constexpr static std::size_t preferred_block_size = 16;

	fd_reader(int fd_)
	: fd(fd_)
	{}

	std::size_t copy(char *buf, std::size_t count)
	{
		auto rc(::read(fd, buf, count));
		if (rc < 0)
			throw std::system_error(errno, std::system_category());

		return rc;
	}

	int fd;
};

}

BOOST_AUTO_TEST_CASE(induced_range_1)
{
	test::fd_reader frd(::open("../ref/string_map/names.00.in", O_RDONLY));
	using boost::test_tools::output_test_stream;
	output_test_stream out("../ref/string_map/names.00.in", true);

	auto ir_0(make_induced_range<char>(frd));
	auto l_iter(ir_0.begin());
	int l_cnt(0);

	for (auto iter(ir_0.begin()); iter != ir_0.end(); ++iter) {
		if (*iter == '\n') {
			std::string s(l_iter, iter);
			printf("xx %d: %s\n", l_cnt, s.c_str());
			out << s << '\n';
			BOOST_CHECK(out.match_pattern());
			++l_cnt;
			l_iter = iter;
			++l_iter;
		}
	}

	if (l_iter != ir_0.end()) {
		std::string s(l_iter, ir_0.end());
		out << s << '\n';
		BOOST_CHECK(out.match_pattern());
		++l_cnt;
	}

	BOOST_CHECK_EQUAL(l_cnt, 4676);
}

}}}
