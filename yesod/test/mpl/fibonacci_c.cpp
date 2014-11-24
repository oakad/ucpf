/*==============================================================================
    Copyright (c) 2014      Alex Dubov <oakad@yahoo.com>

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#define BOOST_TEST_MODULE yesod_mpl
#include <boost/test/unit_test.hpp>

#include <yesod/mpl/value_cast.hpp>
#include <yesod/mpl/fibonacci_c.hpp>

namespace ucpf { namespace yesod { namespace mpl {

BOOST_AUTO_TEST_CASE(fibonacci_c_0)
{
	static constexpr std::array<uint64_t, 52> fib_ref = {{
		1, 2, 3, 5,
		8, 13, 21, 34,
		55, 89, 144, 233,
		377, 610, 987, 1597,
		2584, 4181, 6765, 10946,
		17711, 28657, 46368, 75025,
		121393, 196418, 317811, 514229,
		832040, 1346269, 2178309, 3524578,
		5702887, 9227465, 14930352, 24157817,
		39088169, 63245986, 102334155, 165580141,
		267914296, 433494437, 701408733, 1134903170,
		1836311903ull, 2971215073ull, 4807526976ull, 7778742049ull,
		12586269025ull, 20365011074ull, 32951280099ull, 53316291173ull
	}};

	typedef value_cast<typename fibonacci_c<uint64_t, 52>::type> r;

	BOOST_CHECK(r::value == fib_ref);
}

}}}
