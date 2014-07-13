/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_YESOD_DETAIL_INT128_20140709T2300)
#define UCPF_YESOD_DETAIL_INT128_20140709T2300

#if defined(_GLIBCXX_USE_INT128)

typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;

#else

struct [[gnu::packed]] int128_t {
	uint64_t low;
	uint64_t high;

	int128_t(uint64_t other)
	: low(other), high(other >> 63 ? ~uint64_t(0) : uint64_t(0))
	{}
};

struct [[gnu::packed]] uint128_t {
	uint64_t low;
	uint64_t high;

	uint128_t(uint64_t other)
	: low(other), high(uint64_t(0))
	{}
};

#endif

#endif
