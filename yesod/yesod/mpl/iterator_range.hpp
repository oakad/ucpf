/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_YESOD_MPL_ITERATOR_RANGE_DEC_11_2013_1515)
#define UCPF_YESOD_MPL_ITERATOR_RANGE_DEC_11_2013_1515

namespace ucpf { namespace yesod { namespace mpl {

struct iterator_range_tag;

template <typename First, typename Last>
struct iterator_range {
	typedef iterator_range_tag tag;
	typedef iterator_range type;
	typedef First begin;
	typedef Last end;
};

}}}

#endif
