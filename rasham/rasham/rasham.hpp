/*
 * Copyright (C) 2012 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(_RASHAM_RASHAM_HPP)
#define _RASHAM_RASHAM_HPP

#include <boost/preprocessor/cat.hpp>

#if !defined(RASHAM_FLAG_PREFIX)
#  define RASHAM_FLAG_PREFIX rasham
#endif

#define rasham_LOCUS_ID BOOST_PP_CAT(RASHAM_FLAG_PREFIX, _loc_id)
#define rasham_LOCUS_POS BOOST_PP_CAT(RASHAM_FLAG_PREFIX, _loc_pos)

namespace rasham
{

struct locus {
	unsigned long *cond;
	char const    *dest;
	char const    *file;
	char const    *func;
	unsigned int  line;
};

void print(locus const *loc, char const *fmt, ...)
__attribute__((format(printf, 2, 3)));

}

#if !defined(RASHAM_FLAG_PREFIX)
#  define RASHAM_FLAG_PREFIX rasham
#endif

#define rasham_LOCUS_ID BOOST_PP_CAT(RASHAM_FLAG_PREFIX, _loc_id)

#if defined(RASHAM_NO_SOURCE_INFO)

#define rasham_print(dest, fmt, ...)                                  \
do {                                                                  \
	static unsigned long RASHAM_LOCUS_ID(1UL);                    \
	static const rasham::locus RASHAM_LOCUS_POS(                  \
		{&RASHAM_LOCUS_ID, dest, nullptr, nullptr, 0}         \
	);                                                            \
        if (RASHAM_LOCUS_ID)                                          \
		rasham::print(&RASHAM_LOCUS_POS, fmt, ##__VA_ARGS__); \
} while (0)

#else

#define rasham_print(dest, fmt, ...)                                       \
do {                                                                       \
	static unsigned long RASHAM_LOCUS_ID(1UL);                         \
	static const rasham::locus RASHAM_LOCUS_POS(                       \
		{&RASHAM_LOCUS_ID, dest, __FILE__, __FUNCTION__, __LINE__} \
	);                                                                 \
	if (RASHAM_LOCUS_ID)                                               \
		rasham::print(&RASHAM_LOCUS_POS, fmt,                      \
			      ##__VA_ARGS__);                              \
} while (0)

#endif


#endif
