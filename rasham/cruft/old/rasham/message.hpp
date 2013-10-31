/*
 * Copyright (C) 2012 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(_RASHAM_MESSAGE_HPP)
#define _RASHAM_MESSAGE_HPP

#include <memory>
#include <vector>
#include <string>
#include <iostream>
#include <ctime>

#include <rasham/rasham.hpp>
#include <rasham/counted_ptr.hpp>

namespace rasham
{

struct message_head {
	message_head()
	: origin(nullptr), time_stamp({0, 0}) {}

	void set_origin(locus const *origin_)
	{
		origin = origin_;
		clock_gettime(CLOCK_REALTIME, &time_stamp);
	}

	locus const *origin;
	timespec time_stamp;
};

typedef counted_ptr<message_head> message;

static inline message make_message(size_t count)
{
	return make_counted<message_head>(message::extra_size_t(count + 1));
}

message make_message(char const *fmt, ...);

template <typename char_type, typename traits_type>
std::basic_ostream<char_type, traits_type> &operator<<(
	std::basic_ostream<char_type, traits_type> &os, message val
)
{
	return os << val.get_extra<char_type>();
}

}

#endif
