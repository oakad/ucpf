/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_MINA_CHANNEL_NOV_25_2013_1515)
#define UCPF_MINA_CHANNEL_NOV_25_2013_1515

namespace ucpf { namespace mina {

#define FCSA_PADDING 64

struct channel {
	struct item {
		size_t id;
		uint8_t *data;
	};

	channel(size_t item_sz_order_, size_t item_cnt_order_)
	: item_sz_order(item_sz_order_), item_sz(size_t(1) << item_sz_order),
	  item_cnt_order(item_cnt_order_),
	  item_cnt(size_t(1) << item_cnt_order),
	  item_cnt_mask((size_t(1) << item_cnt_order) - 1)
	{
		alloc_item_buf();
	}

	~channel()
	{
		::operator delete(item_buf);
	}

	item acquire()
	{

	}

	void release(item it)
	{
		auto r_pos(rq_head.fetch_add(1));
		r_path_q[r_pos & item_cnt_mask] = it.data;
		if (waiter_cnt.load())
			free_items.notify_one();
	}

private:
	void alloc_item_buf()
	{
		auto d_sz(size_t(1) << (item_sz_order + item_cnt_order));
		auto q_sz(
			(size_t(1) << (item_cnt_order + 1)) * sizeof(uint8_t *)
		);

		item_buf = ::operator new(d_sz + q_sz);
		f_path_q = item_buf + d_sz;
		r_path_q = item_buf + d_sz + q_sz / 2;
		std::memset(f_path_q, 0, q_sz);
		for (auto cnt(0); cnt < item_cnt; ++cnt)
			r_path_q[cnt] = item_buf + (cnt << item_sz_order);
	}

	size_t item_sz_order;
	size_t item_sz;
	size_t item_cnt_order;
	size_t item_cnt;
	size_t item_cnt_mask;
	uint8_t *item_buf;
	uint8_t **f_path_q;
	uint8_t **r_path_q;
	std::mutex lock;
	std::condition_variable free_items;
	alignas(FCSA_PADDING) std::atomic<size_t> fq_head;
	alignas(FCSA_PADDING) std::atomic<size_t> fq_tail;
	alignas(FCSA_PADDING) std::atomic<size_t> rq_head;
	alignas(FCSA_PADDING) std::atomic<size_t> rq_tail;
	alignas(FCSA_PADDING) std::atomic<size_t> waiter_cnt;
};

}}
#endif

