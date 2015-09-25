/*
 * Copyright (c) 2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_EF9205ED691EC421971F242C94A2ADC5)
#define HPP_EF9205ED691EC421971F242C94A2ADC5

#include <mutex>
#include <yesod/allocator/klipah/detail/basic_block.hpp>
#include <yesod/allocator/klipah/detail/basic_object_manager.hpp>

namespace ucpf { namespace yesod { namespace allocator { namespace klipah {
namespace policy {

struct basic {
	typedef detail::basic_block<0> block_type;
	typedef void * pointer;
	typedef std::mutex mutex_type;
	typedef detail::basic_object_manager object_manager_type;
};

}
}}}}
#endif
