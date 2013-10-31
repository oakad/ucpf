#define BOOST_TEST_MODULE rope
#include <boost/test/included/unit_test.hpp>
#include <boost/test/output_test_stream.hpp>

#include <rasham/rope.hpp>
#include <iostream>

BOOST_AUTO_TEST_CASE(append)
{
	using boost::test_tools::output_test_stream;
	output_test_stream out("ref/rope/append.00.out", true);
	output_test_stream h1("ref/rope/append.01.out", false);
	using namespace rasham;

	crope a("aaaa");
	crope b("bbbb");

	crope c(a + b);
	out << c << std::endl;
	h1 << c.dump() << std::endl;
	BOOST_CHECK(out.match_pattern());
	BOOST_CHECK(h1.match_pattern());
}