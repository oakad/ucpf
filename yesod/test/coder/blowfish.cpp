/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#define BOOST_TEST_MODULE yesod
#include <boost/test/included/unit_test.hpp>

#include <yesod/coder/blowfish.hpp>
#include <yesod/coder/detail/pi_word.hpp>

namespace ucpf { namespace yesod { namespace coder {

BOOST_AUTO_TEST_CASE(blowfish_0)
{
	std::array<uint64_t, 16> in = {{
		0x561c45560283d4f2ull, 0xfe3d32f8098b186dull,
		0xce67f5bb2246527cull, 0xbb2f834cd3584fb9ull,
		0x55a80c4217701c7eull, 0xa9d5ae50ec113817ull,
		0x84cc91265c5d2504ull, 0xc76f3ddffd4e55cdull,
		0x4d787e388cd7bf1full, 0xa1fdc754d7b1e516ull,
		0x5393f8cda9b98a1aull, 0xb37892325fbe67d3ull,
		0xdc03745b942efcbeull, 0x3da5224d41368a7eull,
		0x984e7faa43f4067dull, 0xfe85d247920962e0ull
	}};
	std::array<uint64_t, 16> out_enc, out_dec;

	uint8_t key[] = "any key 1 2 3";

	blowfish cd;
	cd.set_key(key, sizeof(key) - 1, detail::bellard_pi_word);
	std::transform(
		in.begin(), in.end(), out_enc.begin(), std::bind(
			&blowfish::encrypt, std::ref(cd),
			std::placeholders::_1
		)
	);
	std::transform(
		out_enc.begin(), out_enc.end(), out_dec.begin(),
		std::bind(
			&blowfish::decrypt, std::ref(cd),
			std::placeholders::_1
		)
	);

	BOOST_CHECK((out_dec == in));
}

}}}
