#define BOOST_TEST_MODULE basic_btree
#include <boost/test/unit_test.hpp>

#include <pardes/basic_btree.hpp>
#include "x_type.hpp"

BOOST_AUTO_TEST_CASE(constructors)
{
	pardes::basic_btree<x_type, x_type> t0;
	auto p0(t0.insert_unique(5));
}
