/*==============================================================================
    Copyright (c) 2000-2004 Aleksey Gurtovoy
    Copyright (c) 2013      Alex Dubov <oakad@yahoo.com>

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#define BOOST_TEST_MODULE yesod_mpl
#include <boost/test/unit_test.hpp>

#include <yesod/mpl/comparison.hpp>
#include <yesod/mpl/iterator_tags.hpp>

namespace ucpf { namespace yesod { namespace mpl {

BOOST_AUTO_TEST_CASE(iterator_tags_0)
{
	BOOST_CHECK((less< \
		forward_iterator_tag, bidirectional_iterator_tag \
	>::value));
	BOOST_CHECK((less< \
		bidirectional_iterator_tag, random_access_iterator_tag \
	>::value));
}

}}}
