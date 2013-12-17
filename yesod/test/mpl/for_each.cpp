/*==============================================================================
    Copyright (c) 2000-2004 Aleksey Gurtovoy
    Copyright (c) 2013      Alex Dubov <oakad@yahoo.com>

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#define BOOST_TEST_MODULE yesod_mpl
#include <boost/test/included/unit_test.hpp>
#include <boost/test/output_test_stream.hpp>

#include <yesod/mpl/list.hpp>
#include <yesod/mpl/range_c.hpp>
#include <yesod/mpl/for_each.hpp>

#include <typeinfo>

namespace ucpf { namespace yesod { namespace mpl {
namespace test {

struct type_printer {
	type_printer(std::ostream& s) : f_stream(&s) {}

	template <typename U>
	void operator()(identity<U>) {
		*f_stream << typeid(U).name();
	}

	std::ostream *f_stream;
};

struct value_printer {
	value_printer(std::ostream& s) : f_stream(&s) {}
   
	template <typename U>
	void operator()(U x) {
		*f_stream << x;
	}

	std::ostream *f_stream;
};

void push_back(std::vector<int> *c, int i)
{
	c->push_back(i);
}

}

BOOST_AUTO_TEST_CASE(for_each_0)
{
	boost::test_tools::output_test_stream output;

	typedef list<char, short, int, long, float, double> types;

	for_each<types, make_identity<arg<0>>>(test::type_printer(output));
	BOOST_CHECK(output.is_equal("csilfd"));

	typedef range_c<int, 0, 10> numbers;
	std::vector<int> v;

	for_each<numbers>(
		std::bind(&test::push_back, &v, std::placeholders::_1)
	);

	for_each<numbers>(test::value_printer(output));
	BOOST_CHECK(output.is_equal("0123456789"));

	for (unsigned i = 0; i < v.size(); ++i)
		BOOST_CHECK_EQUAL(v[i], int(i));
}

}}}
