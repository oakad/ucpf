/*
 * Copyright (c) 2017 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
/*=============================================================================
   Based on original implementation of GNU ISO C++ Library.

   Copyright (C) 2008-2017 Free Software Foundation, Inc.

   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 3, or (at your option) any later version.
==============================================================================*/

#if !defined(HPP_8F2B97F0410D56C208DC74711E273730)
#define HPP_8F2B97F0410D56C208DC74711E273730

#include <yesod/detail/allocated_object.hpp>
#include <thread>
#include <atomic>
#include <random>
#include <condition_variable>

namespace ucpf::yesod::concurrent {

struct thread;

namespace detail {

template <typename Unused = void>
struct current_thread_data;

struct thread_data {
protected:
	friend struct ucpf::yesod::concurrent::thread;

	template <typename Unused>
	friend struct current_thread_data;

	typedef __gthread_t native_handle_type;

	thread_data(uint32_t tl_prng_seed_)
	: handle{}, flags{0}, use_count{1},
	  tl_prng_state(tl_prng_seed_), park_lock{}, park_cv{}
	{}

	~thread_data() = default;
	virtual void release() = 0;
	virtual void child_attach() = 0;

	template <typename BeforeWaitCallback>
	void park(BeforeWaitCallback &&cb)
	{
		auto f(flags.fetch_and(~FLAG_PARK_RELEASE));
		if (f & FLAG_PARK_RELEASE)
			return;

		if (f & FLAG_INTERRUPTED)
			return;

		std::unique_lock<std::mutex> lk(park_lock, std::try_to_lock);
		if (!lk)
			return;

		f = flags.fetch_and(~FLAG_PARK_RELEASE);
		if (f & FLAG_PARK_RELEASE)
			return;

		if (cb())
			park_cv.wait(lk);

		flags &= ~FLAG_PARK_RELEASE;
		return;
	}

	template <
		typename Rep, typename Period, typename BeforeWaitCallback
	>
	bool park_for(
		std::chrono::duration<Rep, Period> const &rel_time,
		BeforeWaitCallback &&cb
	)
	{
		auto f(flags.fetch_and(~FLAG_PARK_RELEASE));
		if (f & FLAG_PARK_RELEASE)
			return true;

		if (f & FLAG_INTERRUPTED)
			return true;

		std::unique_lock<std::mutex> lk(park_lock, std::try_to_lock);
		if (!lk)
			return true;

		f = flags.fetch_and(~FLAG_PARK_RELEASE);
		if (f & FLAG_PARK_RELEASE)
			return true;

		auto rv(true);
		if (cb())
			rv = park_cv.wait_for(
				lk, rel_time
			) == std::cv_status::no_timeout;

		flags &= ~FLAG_PARK_RELEASE;
		return rv;
	}

	void unpark()
	{
		park_lock.lock();
		auto f(flags.fetch_or(FLAG_PARK_RELEASE));
		park_lock.unlock();
		if (!(f & FLAG_PARK_RELEASE))
			park_cv.notify_one();
	}

	void interrupt()
	{
		if (!(flags.fetch_or(FLAG_INTERRUPTED) & FLAG_INTERRUPTED))
			unpark();
	}

	bool is_interrupted()
	{
		return flags.load() & FLAG_INTERRUPTED;
	}

	bool is_interrupted(bool clear_interrupted)
	{
		if (clear_interrupted) {
			auto old(flags.fetch_and(~FLAG_INTERRUPTED));
			return old & FLAG_INTERRUPTED;
		} else
			return is_interrupted();
	}

	uint32_t next_seed()
	{
		tl_prng_state ^= tl_prng_state << 13;
		tl_prng_state ^= tl_prng_state >> 17;
		tl_prng_state ^= tl_prng_state << 5;

		return tl_prng_state;
	}

	enum : uint32_t {
		FLAG_INTERRUPTED = 1,
		FLAG_PARK_RELEASE = 2
	};

	native_handle_type handle;
	std::atomic<uint32_t> flags;
	std::atomic<uint32_t> use_count;
	uint32_t tl_prng_state;
	std::mutex park_lock;
	std::condition_variable park_cv;
};

template <typename Unused>
struct current_thread_data {
	struct pcg32_state {
		pcg32_state()
		: pcg32_state(std::random_device())
		{}

		template <typename RndGen>
		pcg32_state(RndGen &&rnd)
		: inc((uint64_t(rnd()) << 32) | rnd() | 1), state(0)
		{
			next();
			state += (uint64_t(rnd()) << 32) | rnd();
			next();
		}

		uint32_t next()
		{
			auto prev_state(state.load());
			uint64_t next_state;
			do {
				next_state = prev_state * mul_step + inc;
			} while (!state.compare_exchange_weak(
				prev_state, next_state
			));

			uint32_t xorshifted = (
				(prev_state >> 18) ^ prev_state
			) >> 27;
			uint32_t rot = prev_state >> 59;
			return (
				xorshifted >> rot
			) | (xorshifted << ((-rot) & 31));
		}

		constexpr static uint64_t mul_step = 6364136223846793005ULL;

		uint64_t const inc;
		std::atomic<uint64_t> state;
	};

	struct thread_exit_handler {
		~thread_exit_handler()
		{
			holder.destroy();
		}
	};

	void set_data(thread_data *tdata_)
	{
		tdata = tdata_;
		accessor = get_data;
		exit_handler_ptr = &exit_handler;
	}

	void destroy()
	{
		if (tdata)
			tdata->release();
	}

	typedef thread_data *(*thread_data_accessor)();

	static thread_local current_thread_data holder;
	static thread_local thread_exit_handler exit_handler;

	static pcg32_state shared_prng_state;

	static thread_data *allocate_and_get_data();

	static thread_data *get_data()
	{
		return holder.tdata;
	}

	thread_data *tdata = nullptr;
	thread_data_accessor accessor = allocate_and_get_data;
	thread_exit_handler *exit_handler_ptr = nullptr;
};

template <typename Unused>
thread_local current_thread_data<Unused> current_thread_data<Unused>::holder;

template <typename Unused>
thread_local typename current_thread_data<Unused>::thread_exit_handler
current_thread_data<Unused>::exit_handler;

template <typename Unused>
typename current_thread_data<Unused>::pcg32_state
current_thread_data<Unused>::shared_prng_state;

template <typename Allocator>
struct thread_data_impl : thread_data {
	static thread_data *create(Allocator const &alloc)
	{
		auto tl_seed(current_thread_data<>::shared_prng_state.next());
		while (!tl_seed)
			current_thread_data<>::shared_prng_state.next();

		return yesod::detail::allocated_object<
			thread_data_impl, Allocator
		>::create(alloc, tl_seed)->get();
	}

	thread_data_impl(uint32_t tl_prng_seed)
	: thread_data(tl_prng_seed)
	{}

protected:
	friend struct ucpf::yesod::concurrent::thread;

	template <typename Unused>
	friend struct current_thread_data;

	void release() override
	{
		if (!(--use_count))
			yesod::detail::allocated_object<
				thread_data_impl, Allocator
			>::to_storage_ptr(this)->destroy();
	}

	void child_attach() override
	{
		++use_count;
		current_thread_data<>::holder.set_data(this);
	}
};

template <typename Unused>
thread_data *current_thread_data<Unused>::allocate_and_get_data()
{
	auto data = detail::thread_data_impl<std::allocator<void>>::create(
		std::allocator<void>{}
	);
	data->handle = __gthread_self();
	holder.set_data(data);
	return data;
}

}

namespace this_thread {

static thread get();

}

struct thread {
	thread() noexcept = default;
	thread(thread &) = delete;
	thread(thread const &) = delete;
	thread(thread const &&) = delete;

	thread(thread &&other) noexcept
	: tdata{other.tdata.exchange(nullptr)}, transient{other.transient}
	{}

	template <typename Allocator, typename Callable, typename... Args>
	explicit thread(
		std::allocator_arg_t tag, Allocator const &alloc,
		Callable &&f, Args &&...args
	)
	: tdata{detail::thread_data_impl<Allocator>::create(alloc)}
	{
		start_thread(make_invoker(
			alloc, std::forward<Callable>(f),
			std::forward<Args>(args)...
		));
	}

	template <
		typename Callable,
		typename... Args,
		typename = std::enable_if_t<!std::is_same<
			typename std::__decay_and_strip<Callable>::__type,
			std::allocator_arg_t
		>::value>
	>
	explicit thread(Callable &&f, Args &&...args)
	: thread(
		std::allocator_arg,
		std::allocator<void>{},
		std::forward<Callable>(f),
		std::forward<Args>(args)...
	)
	{}

	~thread()
	{
		release();
	}

	void release()
	{
		auto tdata_(tdata.exchange(nullptr));
		if (tdata_) {
			if (transient)
				tdata_->release();
			else
				std::terminate();
		}
	}

	thread &operator=(thread const &) = delete;

	thread &operator=(thread &&other) noexcept
	{
		if (tdata.load())
			std::terminate();

		tdata.store(other.tdata.exchange(nullptr));
		transient = other.transient;
		return *this;
	}

	typedef __gthread_t native_handle_type;

	explicit operator bool()
	{
		return tdata.load() != nullptr;
	}

	bool joinable() const noexcept
	{
		return tdata.load() != nullptr;
	}

	void join()
	{
		int err(EINVAL);

		err = __gthread_join(tdata.load()->handle, 0);

		if (err)
			throw std::system_error(std::error_code(
				err, std::generic_category()
			));

		tdata.exchange(nullptr)->release();
	}

	void detach()
	{
		int err(EINVAL);

		err = __gthread_detach(tdata.load()->handle);

		if (err)
			throw std::system_error(std::error_code(
				err, std::generic_category()
			));

		tdata.exchange(nullptr)->release();
	}

	void interrupt()
	{
		tdata.load()->interrupt();
	}

	bool is_interrupted()
	{
		return tdata.load()->is_interrupted();
	}

	bool is_interrupted(bool clear_interrupted)
	{
		return tdata.load()->is_interrupted(clear_interrupted);
	}

	template <typename BeforeWaitCallback = std::true_type>
	void park(BeforeWaitCallback &&cb = std::true_type{})
	{
		tdata.load()->park(std::forward<BeforeWaitCallback>(cb));
	}

	template <
		typename Rep, typename Period,
		typename BeforeWaitCallback = std::true_type
	>
	bool park_for(
		std::chrono::duration<Rep, Period> const &rel_time,
		BeforeWaitCallback &&cb = std::true_type{}
	)
	{
		return tdata.load()->park_for(
			rel_time, std::forward<BeforeWaitCallback>(cb)
		);
	}

	void unpark()
	{
		tdata.load()->unpark();
	}

	uint32_t next_seed()
	{
		return tdata.load()->next_seed();
	}

	std::thread::id get_id() const noexcept
	{
		return std::thread::id(native_handle());
	}

	native_handle_type native_handle() const noexcept
	{
		return tdata.load()
			? tdata.load()->handle 
			: detail::thread_data::native_handle_type{};
	}

private:
	friend thread this_thread::get();

	explicit thread(detail::thread_data *tdata_)
	: tdata{tdata_}, transient{true}
	{
		++tdata_->use_count;
	}

	struct invoker_base {
		virtual ~invoker_base() = default;
		virtual void run() = 0;
		virtual void destroy() = 0;
	};

	static void *execute_native_thread_routine(void *p_)
	{
		auto p(reinterpret_cast<invoker_base *>(p_));

		try {
			p->run();
		} catch(__cxxabiv1::__forced_unwind const &) {
			p->destroy();
			throw;
		} catch(...) {
			p->destroy();
			std::terminate();
		}
		p->destroy();
		return nullptr;
	}

	void start_thread(invoker_base *p)
	{
		auto err(__gthread_create(
			&tdata.load()->handle, execute_native_thread_routine, p
		));

		if (err) {
			p->destroy();
			throw std::system_error(std::error_code(
				err, std::generic_category()
			));
		}
	}

	template <typename TupleType, typename Allocator>
	struct invoker : public invoker_base {
		template <size_t Index>
		static std::tuple_element_t<Index, TupleType> &&declval_();

		template<size_t ...Index>
		auto invoke(std::_Index_tuple<Index...>) noexcept(
			noexcept(std::__invoke(declval_<Index>()...))
		) -> decltype(std::__invoke(declval_<Index>()...))
		{
			return std::__invoke(std::get<Index>(std::move(t))...);
		}

		using Indices = typename std::_Build_index_tuple<
			std::tuple_size<TupleType>::value
		>::__type;

		void run() override
		{
			tdata->child_attach();
			invoke(Indices());
		}

		void destroy() override
		{
			yesod::detail::allocated_object<
				invoker, Allocator
			>::to_storage_ptr(this)->destroy();
		}

		invoker(detail::thread_data *tdata_, TupleType &&t_)
		: tdata(tdata_), t(std::forward<TupleType>(t_))
		{}

		detail::thread_data *tdata;
		TupleType t;
	};

	template<typename Allocator, typename Callable, typename... Args>
	invoker_base *make_invoker(
		Allocator const &alloc, Callable &&f, Args &&...args
	)
	{
		typedef std::tuple<
			typename std::__decay_and_strip<Callable>::__type,
			typename std::__decay_and_strip<Args>::__type...
		> tuple_type;

		return yesod::detail::allocated_object<
			invoker<tuple_type, Allocator>, Allocator
		>::create(alloc, tdata, tuple_type(
			std::allocator_arg, alloc,
			std::forward<Callable>(f),
			std::forward<Args>(args)...
		))->get();
	}

	std::atomic<detail::thread_data *> tdata = {nullptr};
	bool transient = false;
};

namespace this_thread {

static inline thread get()
{
	return thread(detail::current_thread_data<>::holder.accessor());
}

}
}

#endif
