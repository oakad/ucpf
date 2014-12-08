/*
 *  fs/sendfd.c
 *
 *  Copyright (C) 2014 Alex Dubov <oakad@yahoo.com>
 *
 */

struct task_msg {
	uint32_t id;
	uint32_t type;
	__kernel_pid_t pid;
	__ARCH_SI_UID_T uid;
	uintptr_t val;

	union {
		struct {
			uintptr_t val[2];
		} raw;

		struct {
			int fd;
			int errno;
		} dup_fd;
	} extra_val;
};

struct ktask_msg {
	task_msg msg;
	ktask_msg *prev;
	ktask_msg *next_left;
	ktask_msg *next_right;
	union {
		uintptr_t order;
		ktask_msg *tail;
	};
};

struct msgq_base {
	uint32_t key_mask;
};

struct msgq_trivial {
	struct msgq_base base;
	ktask_msg *msg_list;
};

struct msgq_mlist {
	struct msgq_base base;
	size_t order;
	struct {
		ktask_msg *msg_list;
		uint32_t key_mask;
		uint32_t key_shift;
	} lists[];
};

struct msgq_heap {
	struct msgq_base base;
	ktask_msg *pending_heap;
	ktask_msg *incoming_heap;
};

struct msgq_ops {
	void (*destroy)(msgq_base *msgq);
	void (*flush)(msgq_base *msgq);
	size_t (*pop_min)(msgq_base *msgq, size_t count);
	void (*push)(msgq_base *msgq, ktask_msg *msg_list);
};

static struct kmem_cache *ktask_msg_cache;

void __init task_msg_init(void)
{
	ktask_msg_cache = KMEM_CACHE(ktask_msg, SLAB_PANIC);
}
