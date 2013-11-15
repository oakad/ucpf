/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#define BOOST_TEST_MODULE mina
#include <boost/test/included/unit_test.hpp>
#include <boost/test/output_test_stream.hpp>

#include <mina/pack.hpp>

namespace ucpf { namespace mina {
namespace test {

struct a {
	int x = 0x134567;
	uint16_t y = 0x6563;
	int64_t z = 0x109483209482309LL;
};

}

template <typename OutputIterator>
struct custom<OutputIterator, test::a> {
	static void pack(OutputIterator &&sink, test::a &&va)
	{
		mina::pack(
			std::forward<OutputIterator>(sink), va.x, va.y, va.z
		);
	}
};

BOOST_AUTO_TEST_CASE(pack_0)
{
	using boost::test_tools::output_test_stream;
	output_test_stream out("ref/pack.00.out", false, true);
	std::ostream_iterator<uint8_t> sink(out);

	pack(sink, 0xfedbca98);
	pack(sink, 8, -1, 0x1000, 0x10000, -15);
	pack(sink, 4637.534f, 2394.235);
	BOOST_CHECK(out.match_pattern());
}

BOOST_AUTO_TEST_CASE(pack_1)
{
	using boost::test_tools::output_test_stream;
	output_test_stream out("ref/pack.01.out", false, true);
	std::ostream_iterator<uint8_t> sink(out);

	pack(sink, test::a());
	BOOST_CHECK(out.match_pattern());
}

}}
