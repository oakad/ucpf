/*=============================================================================
    Copyright (c) 2002-2004 Aleksey Gurtovoy
    Copyright (c) 2013      Alex Dubov <oakad@yahoo.com>

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE yesod_mpl
#include <boost/test/unit_test.hpp>

#include <yesod/mpl/common_base.hpp>

namespace ucpf { namespace yesod { namespace mpl {

BOOST_AUTO_TEST_CASE(common_base_0)
{
	typedef typename common_base<
		std::forward_iterator_tag,
		std::forward_iterator_tag,
		std::forward_iterator_tag,
		std::forward_iterator_tag
	>::type b0;
	BOOST_CHECK((std::is_same<b0, std::forward_iterator_tag>::value));

	typedef typename common_base<
		std::bidirectional_iterator_tag,
		std::random_access_iterator_tag,
		std::input_iterator_tag
	>::type b1;
	BOOST_CHECK((std::is_same<b1, std::input_iterator_tag>::value));
}

}}}
