/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_YESOD_MPL_ERASE_KEY_DEC_11_2013_1530)
#define UCPF_YESOD_MPL_ERASE_KEY_DEC_11_2013_1530

#include <yesod/mpl/sequence_tag.hpp>
#include <yesod/mpl/detail/erase_key.hpp>

namespace ucpf { namespace yesod { namespace mpl {

template<typename Sequence, typename Key>
struct erase_key : detail::erase_key_impl<
	typename sequence_tag<Sequence>::type
>::template apply<Sequence, Key> {};

}}}

#endif
