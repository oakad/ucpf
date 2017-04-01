/*
 * Copyright (c) 2017 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_81C349D48D8EDC0F6359A034C341668C)
#define HPP_81C349D48D8EDC0F6359A034C341668C

namespace ucpf::yesod::concurrent::detail {

template <typename ItemAccessor>
struct ring_fifo {
	typedef ItemAccessor accessor_type;
	typedef typename ItemAccessor::fifo_item_type value_type;

	ring_fifo(ring_fifo &) = delete;
	ring_fifo(ring_fifo &&) = delete;
	ring_fifo(ring_fifo const &) = delete;
	ring_fifo(ring_fifo const &&) = delete;

	ring_fifo(size_t order, accessor_type const &acc)
	: head_start{0}, head_end{0}, tail{0}, use_count{0},
	  ring_size(size_t(1) << order), ring(acc.allocate_array(ring_size))
	{}

	void add(accessor_type const &acc)
	{
		++use_count;
		auto pos(acquire_head(acc));
		acc.emplace_value((*ring)[pos]);
		auto he(head_end.load());

		if (he < pos) {
			--use_count;
			return;
		} else if (he == pos) {
			auto next_he = inc_pos(he);
			if (!head_end.compare_exchange_strong(
				he, next_he
			)) {
				--use_count;
				return;
			} else
				he = next_he;
		}

		while (true) {
			auto hs(head_start.load());
			if (hs == lock_value) {
				--use_count;
				wait_for_head_unlock();
				++use_count;
				he = head_end.load();
				continue;
			}

			if (hs == he)
				break;

			if (!acc.is_set((*ring)[he]))
				break;

			auto next_he = inc_pos(he);
			if (head_end.compare_exchange_strong(
				he, next_he
			))
				he = next_he;
			else
				break;
		}

		--use_count;
	}

	void poll(accessor_type const &acc, bool consume_one)
	{
		++use_count;
		while (head_start.load() == lock_value) {
			--use_count;
			wait_for_head_unlock();
			++use_count;
		}

		auto he(head_end.load());
		auto t(tail.load());

		while (true) {
			if (t == he)
				break;

			if (t == lock_value) {
				acc.yield_on_conflict();
				t = tail.load();
				continue;
			}

			if (!tail.compare_exchange_weak(t, lock_value))
				continue;

			if (acc.consume((*ring)[t])) {
				t = inc_pos(t);
				tail.store(t);
				if (consume_one)
					break;
			} else {
				tail.exchange(t);
				break;
			}
			t = tail.load();
		}
		--use_count;
	}

private:
	typedef uint32_t word_type;
	constexpr static word_type lock_value = 0xffffffff;

	word_type get_tail(accessor_type const &acc)
	{
		auto t(tail.load());

		while (t == lock_value) {
			acc.yield_on_conflict();
			t = tail.load();
		}
		return t;
	}

	word_type acquire_head(accessor_type const &acc)
	{
		auto hs(head_start.load());

		while (true) {
			auto t(get_tail(acc));
			if (hs == lock_value) {
				--use_count;
				wait_for_head_unlock();
				++use_count;
				hs = head_start.load();
				continue;
			}

			auto next_hs(inc_pos(hs));
			if (next_hs == t) {
				if (head_start.compare_exchange_strong(
					hs, lock_value
				))
					hs = resize_ring(acc);

				continue;
			}

			if (head_start.compare_exchange_weak(hs, next_hs))
				return hs;
		}
	}

	void wait_for_head_unlock()
	{
		if (head_start.load() != lock_value)
			return;

		std::unique_lock<std::mutex> lk(resize_lock);
		while (head_start.load() == lock_value)
			resize_cv.wait(lk);
	}

	word_type resize_ring(accessor_type const &acc)
	{
		while (use_count.load() != 1)
			acc.yield_on_conflict();

		try {
			auto next_size(ring_size << 1);
			auto next_ring(acc.allocate_array(next_size));
			auto t(tail.load()), he(head_end.load());
			word_type pos(0);
			
			while (t != he) {
				next_ring[pos] = std::move(ring[t]);
				++pos;
				t = inc_pos(t);
			}

			auto prev_size(ring_size);
			auto prev_ring(ring);
			ring_size = next_size;
			ring = next_ring;
			tail.store(0);
			head_end.store(pos);
			{
				std::unique_lock<std::mutex> lk(resize_lock);
				head_start.store(head_end.load());
			}
			resize_cv.notify_all();
			acc->deallocate_array(prev_ring, prev_size);
		} catch (...) {
			{
				std::unique_lock<std::mutex> lk(resize_lock);
				head_start.store(head_end.load());
			}
			--use_count;
			resize_cv.notify_all();
			throw;
		}
	}

	word_type inc_pos(word_type pos)
	{
		return (pos + 1) & (ring_size - 1);
	}

	std::atomic<word_type> head_start;
	std::atomic<word_type> head_end;
	std::atomic<word_type> tail;
	std::atomic<word_type> use_count;

	std::size_t ring_size;
	value_type *ring;
	std::mutex resize_lock;
	std::condition_variable resize_cv;
};

}
#endif
