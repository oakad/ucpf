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

struct channel {
	struct item {
		uint8_t *data;
	};

	channel(size_t item_sz_order_, size_t item_cnt_order_)
	: item_sz_order(item_sz_order_), item_sz(size_t(1) << item_sz_order),
	  item_cnt_order(item_cnt_order_),
	  item_cnt(size_t(1) << item_cnt_order),
	  item_cnt_mask((size_t(1) << item_cnt_order) - 1),
	  fq_head(item_cnt / 2), fq_tail(item_cnt / 2),
	  rq_head(item_cnt), rq_tail(0)
	{
		alloc_item_buf();
	}

	~channel()
	{
		for (size_t cnt(0); cnt < item_cnt; ++cnt) {
			fq_item(cnt).~std::atomic<uint8_t *>();
			rq_item(cnt).~std::atomic<uint8_t *>();
		}
		::operator delete(item_buf);
	}

	item acquire()
	{
		auto r_pos(rq_tail.fetch_add(1));
		item rv = { nullptr };

		while (true) {
			auto c_pos(rq_head.load());
			auto delta(c_pos - item_cnt);

			if ((c_pos - delta) > (r_pos - delta)) {
				rv.data = rq_item(
					r_pos & item_cnt_mask
				).exchange(nullptr);
				if (rv.data)
					return item;
			}
			{
				std::unique_lock<std::mutex> w_g(rq_lock);
				rq_cv.wait(w_g);
			}
		}
	}

	void release(item it)
	{
		auto r_pos(rq_head.fetch_add(1));
		rq_item(r_pos & item_cnt_mask).exchange(it.data);
		rq_cv.notify_one();
	}

	void enqueue(item it)
	{
		auto r_pos(fq_head.fetch_add(1));
		fq_item(r_pos & item_cnt_mask).exchange(it.data);
		fq_cv.notify_one();
	}

	item dequeue()
	{
		auto r_pos(fq_tail.fetch_add(1));
		item rv = { nullptr };

		while (true) {
			auto c_pos(fq_head.load());
			auto delta(c_pos - item_cnt);

			if ((c_pos - delta) > (r_pos - delta)) {
				rv.data = fq_item(
					r_pos & item_cnt_mask
				).exchange(nullptr);
				if (rv.data)
					return item;
			}
			{
				std::unique_lock<std::mutex> w_g(fq_lock);
				fq_cv.wait(w_g);
			}
		}
	}

private:
	typedef std::atomic<uint8_t *> item_ptr_type;

	uint8_t *locus_ptr(size_t pos)
	{
		return item_buf + pos * (item_sz + 2 * sizeof(item_ptr_type));
	}

	uint8_t *data_ptr(size_t pos)
	{
		return locus_ptr(pos) + sizeof(item_ptr_type);
	}

	item_ptr_type &fq_item(size_t pos)
	{
		return *reinterpret_cast<item_ptr_type *>(locus_ptr(pos));
	}

	item_ptr_type &rq_item(size_t pos)
	{
		return *reinterpret_cast<item_ptr_type *>(
			data_ptr(pos) + item_sz
		);
	}


	void alloc_item_buf()
	{
		auto b_sz(
			item_sz * item_cnt
			+ item_cnt * sizeof(item_ptr_type) * 2
		);

		item_buf = ::operator new(b_sz);

		for (size_t cnt(0); cnt < item_cnt; ++cnt) {
			new (&fq_item(cnt)) std::atomic<uint8_t *>(nullptr);
			new (&rq_item(cnt)) std::atomic<uint8_t *>(
				data_ptr(cnt)
			);
		}
	}

	uint8_t *item_buf;
	size_t item_sz_order;
	size_t item_sz;
	size_t item_cnt_order;
	size_t item_cnt;
	size_t item_cnt_mask;

	std::mutex fq_lock;
	std::condition_variable fq_cv;
	std::mutex rq_lock;
	std::condition_variable rq_cv;
	std::atomic_size_t fq_head;
	std::atomic_size_t rq_head;
	std::atomic_size_t fq_tail;
	std::atomic_size_t rq_tail;
};

}}
#endif

