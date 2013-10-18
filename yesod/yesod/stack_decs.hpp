/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * Based on algorithm from:
 * 
 *      A Dynamic Elimination-Combining Stack Algorithm (Gal Bar-Nissan,
 *      Danny Hendler, Adi Suissa) in 15th International Conference,
 *      OPODIS 2011
 * 
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_YESOD_STACK_DECS_OCT_18_2013_1330)
#define UCPF_YESOD_STACK_DECS_OCT_18_2013_1330

#include <boost/lockfree/detail/tagged_ptr.hpp>

#include <boost/intrusive/detail/utilities.hpp>
#include <boost/intrusive/member_value_traits.hpp>

#include <yesod/detail/stack_decs.hpp>

namespace ucpf { namespace yesod {
namespace detail {
	
}

template <
	typename ValueType, std::size_t MaxActors,
	typename HeadType, HeadType ValueType::* HeadPtr
>
struct stack_decs {
	typedef ValueType       value_type;
	typedef ValueType       &reference;
	typedef ValueType const &const_reference;
	typedef ValueType       *pointer;
        typedef ValueType const *const_pointer;

	struct actor {
		std::size_t id;
	};

	stack_decs()
	{
		id_set.set();
	}

	bool attach(actor &r)
	{
		std::unique_lock<std::mutex> l_g(lock);
		r.id = id_set._Find_first();
		if (r.id >= id_set.size())
			return false;
		else {
			id_set.reset(r.id);
			return true;
		}
	}

	void detach(actor &r)
	{
		std::unique_lock<std::mutex> l_g(lock);
		id_set.set(r.id);
	}

	void push(reference v)
	{
		stack_head *h(member_value_traits::to_node_ptr(v));
		
	}

private:
	struct Op {
		std::size_t actor_id;
		size_t count;
		stack_head *node;
		Op *next;
		Op *last;
		bool push_op;
	};

	std::mutex lock;
	std::bitset<MaxActors> id_set;
	std::array<int, MaxActors> collision;
	std::array<Op, MaxActors> location;
};

}}

#endif
