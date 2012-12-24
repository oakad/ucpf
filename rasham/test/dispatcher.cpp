#define BOOST_TEST_MODULE dispatcher
#include <boost/test/included/unit_test.hpp>
#include <boost/test/output_test_stream.hpp>

#include <rasham/rasham.hpp>
#include <rasham/sink.hpp>
#include <iostream>
#include <unistd.h>

BOOST_AUTO_TEST_SUITE(basic)

void xtry()
{
	for (int cnt = 0; cnt < 5; ++cnt)
		rasham_print("/info/c4", "Test xtry %d: %f", cnt, 4.0);
}

BOOST_AUTO_TEST_CASE(rasham_print)
{
	using boost::test_tools::output_test_stream;
	output_test_stream out(
		"ref/dispatcher/basic/rasham_print.00.out", true
	);

	int repeat(0);
	rasham::ostream_sink o_sink(out);
	rasham::bind_sink("/", &o_sink);
again:
	if (repeat > 2) {
		BOOST_CHECK(out.match_pattern());
		return;
	}

	rasham_print("/debug/c1", "Test me %d", 5);
	for (int cnt = 0; cnt < 5; ++cnt) {
		rasham_print("/info/c2", "Test again %d: %f", cnt, 3.0);
	}
	xtry();
	++repeat;
goto again;
}

BOOST_AUTO_TEST_CASE(dump_hierarchy)
{
	using boost::test_tools::output_test_stream;
	output_test_stream out(
		"ref/dispatcher/basic/dump_hierarchy.00.out", true
	);
	output_test_stream h1(
		"ref/dispatcher/basic/dump_hierarchy.01.out", false
	);
	output_test_stream h2(
		"ref/dispatcher/basic/dump_hierarchy.02.out", false
	);

	{
		rasham::ostream_sink o_sink(out);
		rasham::bind_sink("/", &o_sink);
		rasham_print("/a1/b1/c1", "/a1/b1/c1");
		rasham_print("/a1/b2/c1", "/a1/b2/c1");
		rasham_print("/a2/b1/c1", "/a2/b1/c1");
		rasham_print("/a2/b1/c2", "/a2/b1/c2");
		rasham_print("/a1/b1/c2", "/a1/b1/c2");
		rasham::dump_hierarchy(h1);
		BOOST_CHECK(out.match_pattern());
		BOOST_CHECK(h1.match_pattern());
	}
	rasham::dump_hierarchy(h2);
	BOOST_CHECK(h2.match_pattern());
}

BOOST_AUTO_TEST_SUITE_END()
