/*
 *  fs/task_msgfd.c
 *
 *  Copyright (C) 2014 Alex Dubov <oakad@yahoo.com>
 *
 */

struct task_msg {
	uint32_t id;
	uint32_t type;
	__kernel_pid_t tgid;
	__kernel_pid_t tid;
	__ARCH_SI_UID_T uid;
	uintptr_t val;

	union {
		struct {
			uintptr_t val[2];
		} gen;

		struct {
			int fd;
			int errno;
		} dup_fd;
	};
};

#define TASK_MSG_TYPE_GEN 0
#define TASK_MSG_TYPE_DUP_FD 1
#define TASK_MSG_TYPE_MAX 2

struct ktask_msg {
	task_msg msg;
	ktask_msg *prev;
	ktask_msg *next_left;
	ktask_msg *next_right;
	union {
		unsigned long order;
		ktask_msg *tail;
	};
};

static struct kmem_cache *ktask_msg_cache;

static void ktask_msg_destroy_gen(ktask_msg *msg)
{
	kmem_cache_free(ktask_msg_cache, msg);
}

static void ktask_msg_destroy_dup_fd(ktask_msg *msg)
{
	struct file *f = (struct file *)msg->msg.gen.val[0];
	fput_raw(f);
	ktask_msg_destroy_gen(msg);
}

struct ktask_msg_traits {
	unsigned long msg_type_mask;
	void (*destroy)(ktask_msg *msg);
};

static const ktask_msg_traits ktask_msg_disp[TASK_MSG_TYPE_MAX] = {{
		.msg_type_mask = 1ul << TASK_MSG_TYPE_GEN,
		.destroy = ktask_msg_destroy_gen
	}, {
		.msg_type_mask = 1ul << TASK_MSG_TYPE_DUP_FD,
		.destroy = ktask_msg_destroy_dup_fd
	}
};

struct ktask_msg_endp {
	atomic_long_t allowed_msg_types;
	atomic_long_t pending_max;
	atomic_long_t pending_current;
	atomic_long_t pending;
	wait_queue_head_t task_msg_wqh;
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

void __init ktask_msg_init(void)
{
	ktask_msg_cache = KMEM_CACHE(ktask_msg, SLAB_PANIC);
}

static int ktask_msg_allowed(ktask_msg *msg, unsigned long allowed_types)
{
	unsigned long msg_type_mask = (
		msg->msg.type >= TASK_MSG_TYPE_MAX
	) ? 0 : ktask_msg_disp[msg->msg.type].msg_type_mask;

	return (msg_type_mask & allowed_types) ? 0 : -ENOMSG;
}

static int ktask_msg_enqueue(ktask_msg_endp *dst, ktask_msg *msg)
{
	int rc = ktask_msg_allowed(
		msg, atomic_long_read(&dst->allowed_msg_types)
	);
	if (rc)
		return rc;

	unsigned long q_pos = atomic_long_inc_return(&dst->pending_current);
	if (q_pos > atomic_long_read(&dst->pending_max)) {
		atomic_long_dec(&dst->pending_current);
		return -EAGAIN;
	}

	do {
		msg->order = atomic_long_read(&dst->pending);
	} while (atomic_long_cmpxchg(
		&dst->pending, msg->order, (unsigned long)msg
	) != msg->order);

	return 0;
}
