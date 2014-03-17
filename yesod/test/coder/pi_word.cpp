/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#define BOOST_TEST_MODULE yesod
#include <boost/test/included/unit_test.hpp>

#include <yesod/coder/detail/pi_word.hpp>

namespace ucpf { namespace yesod { namespace coder { namespace detail {
namespace test {

/* pi digits terminating at hex digit 75000 */
constexpr std::array<uint32_t, 6> pi_at_9369 = {{
	0x327fd065, 0xf0c4d23f, 0x82f48a26, 0xbb03abbd,
	0x9f66e01b, 0x4f748a88
}};

/* pi digits terminating at hex digit 250000 */
constexpr std::array<uint32_t, 6> pi_at_31244 = {{
	0xd0a1a806, 0x4bdaaea8, 0x05cce3ff, 0xae2360ad,
	0xc248e783, 0xd94e8399
}};

};

BOOST_AUTO_TEST_CASE(pi_word_0)
{
	std::array<uint32_t, 6> bbp_at_9369;

	for (auto c(0); c < 6; ++c)
		bbp_at_9369[c] = bbp_pi_word(9369 + c);

	BOOST_CHECK((bbp_at_9369 == test::pi_at_9369));

	std::array<uint32_t, 6> bbp_at_31244;

	for (auto c(0); c < 6; ++c)
		bbp_at_31244[c] = bbp_pi_word(31244 + c);

	BOOST_CHECK((bbp_at_31244 == test::pi_at_31244));
}

BOOST_AUTO_TEST_CASE(pi_word_1)
{
	std::array<uint32_t, 6> bellard_at_9369;

	for (auto c(0); c < 6; ++c)
		bellard_at_9369[c] = bellard_pi_word(9369 + c);

	BOOST_CHECK((bellard_at_9369 == test::pi_at_9369));

	std::array<uint32_t, 6> bellard_at_31244;

	for (auto c(0); c < 6; ++c)
		bellard_at_31244[c] = bellard_pi_word(31244 + c);

	BOOST_CHECK((bellard_at_31244 == test::pi_at_31244));
}

}}}}
