/*
 * Copyright (c) 2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_11109CC3D0125D55C012CFBB2EDD03C5)
#define HPP_11109CC3D0125D55C012CFBB2EDD03C5

namespace ucpf { namespace zivug { namespace io {

struct descriptor;
struct scheduler;
struct actor;

struct scheduler_token {
	scheduler_token()
	: ptr(nullptr)
	{}

	scheduler_token(scheduler_token &&other)
	: ptr(other.ptr)
	{
		other.ptr = nullptr;
	}

	scheduler_token &operator=(scheduler_token &&other)
	{
		ptr = other.ptr;
		other.ptr = nullptr;
		return *this;
	}

	scheduler_token(scheduler_token const &other) = delete;
	scheduler_token &operator=(scheduler_token const &other) = delete;

private:
	friend struct scheduler;

	scheduler_token(void *ptr_)
	: ptr(ptr_)
	{}

	void *ptr;
};

struct scheduler_action {
	virtual void resume_read() = 0;

	virtual void resume_read(scheduler_token &&tk) = 0;

	virtual void resume_write() = 0;

	virtual void resume_write(scheduler_token &&tk) = 0;

	virtual void wait_read() = 0;

	virtual void wait_read(scheduler_token &&tk) = 0;

	virtual void wait_write() = 0;

	virtual void wait_write(scheduler_token &&tk) = 0;

	virtual void release() = 0;

	virtual void release(scheduler_token &&tk) = 0;

	virtual scheduler_token suspend() = 0;

	virtual void set_actor(actor &act) = 0;

	virtual scheduler &get_scheduler() = 0;

	virtual descriptor const &get_descriptor() = 0;
};

struct actor {
	virtual void init(scheduler_action &&sa, bool new_desc)
	{
	}

	virtual void fini(scheduler_action &&sa)
	{
	}

	virtual bool read(
		scheduler_action &&sa, bool out_of_band, bool priority
	)
	{
		return false;
	}

	virtual bool write(
		scheduler_action &&sa, bool out_of_band, bool priority
 	)
	{
		return false;
	}

	virtual bool error(scheduler_action &&sa, bool priority)
	{
		return false;
	}

	virtual bool hang_up(scheduler_action &&sa, bool read_only)
	{
		return false;
	}
};

}}}
#endif
