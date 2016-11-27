/*==============================================================================
    Copyright (c) 2015      Alex Dubov <oakad@yahoo.com>

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#define BOOST_TEST_MODULE yesod_mpl
#include <boost/test/unit_test.hpp>

#include <yesod/mpl/value_transform.hpp>

namespace ucpf { namespace yesod { namespace mpl {
namespace test {

template <typename T, T const &v>
struct double_op {
	template <typename Ix, Ix... Cn>
	struct  apply {
		typedef package_c<int, (2 * v[Cn])...> type;
	};
};

// GCC bug 52036
struct v0 {
	constexpr static int value[] = {7, 8, 9, 10, 11};
};

struct  v1 {
	constexpr static std::array<int, 5> value = {{
		12, 13, 14, 15, 16
	}};	
};

}

BOOST_AUTO_TEST_CASE(value_transform_0)
{

	std::array<int, 5> ref0 = {{14, 16, 18, 20, 22}};

	std::array<int, 5> ref1 = {{24, 26, 28, 30, 32}};

	typedef typename value_transform<
		decltype(test::v0::value), test::v0::value, test::double_op
	>::value_type r0;

	BOOST_CHECK_EQUAL_COLLECTIONS(
		ref0.begin(), ref0.end(), r0::value.begin(), r0::value.end()
	);

	typedef typename value_transform<
		decltype(test::v1::value), test::v1::value, test::double_op
	>::value_type r1;

	BOOST_CHECK_EQUAL_COLLECTIONS(
		ref1.begin(), ref1.end(), r1::value.begin(), r1::value.end()
	);
}

}}}
