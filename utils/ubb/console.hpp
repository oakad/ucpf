/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UBB_CONSOLE_NOV_26_2013_1610)
#define UBB_CONSOLE_NOV_26_2013_1610

namespace ubb {

struct console {
	constexpr struct {
		enum {
			PROGRESS = 1
			JOBID    = 2,
			TIME     = 4,
			COLOR    = 8
			
		};
	} printout_flags;
};

}
#endif
