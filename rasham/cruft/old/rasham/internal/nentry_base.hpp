/*
 * Copyright (C) 2012 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(_RASHAM_INTERNAL_NENTRY_BASE_HPP)
#define _RASHAM_INTERNAL_NENTRY_BASE_HPP

#include <mutex>
#include <boost/spirit/include/qi.hpp>
#include <boost/dynamic_bitset.hpp>
#include <rasham/message.hpp>

namespace rasham
{

namespace qi = boost::spirit::qi;

template <typename entry_type, typename char_type, typename traits_type>
struct nentry_t;

template <
	typename entry_type, typename char_type = char,
	typename traits_type = std::char_traits<char_type>
> using nentry = counted_ptr<nentry_t<entry_type, char_type, traits_type>>;

template <
	typename entry_type, typename char_type = char,
	typename traits_type = std::char_traits<char_type>
> using nentry_func = std::function<
	bool (nentry<entry_type, char_type, traits_type>)
>;

template <typename entry_type, typename char_type = char,
	  typename traits_type = std::char_traits<char_type>>
struct nentry_t {
	typedef nentry_t<entry_type, char_type, traits_type> self_type;
	typedef nentry<entry_type, char_type, traits_type> wrapper_type;
	typedef nentry_func<entry_type, char_type, traits_type> func_type;
	typedef std::basic_string<char_type, traits_type> string_type;

	friend struct ref_count_val<self_type>;

	static wrapper_type make(string_type name_ = string_type())
	{
		auto p(make_counted<self_type>(name_));
		return p;
	}

	static wrapper_type get_child(
		string_type name_, wrapper_type parent_, bool create = false
	)
	{
		std::lock_guard<std::mutex> l_g(parent_->m_lock);
		auto child(parent_->children.find(name_));
		if (child)
			return wrapper_type(*child);

		if (!create)
			return wrapper_type();

		wrapper_type p(make_counted<self_type>(name_, parent_));
		parent_->children.add(name_, p);
		return p;
	}

	static wrapper_type apply_to_path(
		wrapper_type r, string_type path, func_type f,
		bool create = false
	);

	static void apply_up(wrapper_type r, func_type f);

	template <bool postorder = false>
	static void apply_down(wrapper_type r, func_type f);

	static wrapper_type lookup_path(
		wrapper_type r, string_type path, bool create = false
	)
	{
		return apply_to_path(
			r, path,
			[](wrapper_type r) -> bool { return true; },
			create
		);
	}

	static std::basic_ostream<char_type, traits_type> &dump(
		wrapper_type r,
		std::basic_ostream<char_type, traits_type> &os,
		std::function<decltype(os) & (entry_type const &,
					      decltype(os) &)> entry_dumper
	)
	{
		if (r) {
			std::lock_guard<std::mutex> l_g(r->m_lock);
			os << '\'' << r->name() << "\' <"
			   << static_cast<void *>(r.get()) << "> ";
			{
				auto &e(r->get_entry());
				entry_dumper(e, os);
			}
			os << '\n';
			tree_dumper dumper(os, entry_dumper);
			r->children.for_each(std::ref(dumper));
			if (dumper.last_entry)
				dumper(dumper.last_name, dumper.last_entry);

			return dumper.os;
		} else
			return os;
	}

	static void cull_tree(wrapper_type r);

	virtual ~nentry_t()
	{
		if (parent) {
			std::lock_guard<std::mutex> l_g(parent->m_lock);

			parent->children.remove(name());
		}
	}

	string_type name() const
	{
		return entry_name;
	}

	entry_type &get_entry()
	{
		return entry;
	}

	void lock()
	{
		m_lock.lock();
	}

	void unlock()
	{
		m_lock.unlock();
	}

	void submit_message(message msg);
private:
	nentry_t(string_type name_) : entry_name(name_) {}

	nentry_t(string_type name_, wrapper_type parent_)
	: entry_name(name_), parent(parent_) {}

	struct tree_dumper {
		std::basic_ostream<char_type, traits_type> &os;
		std::function<
			decltype(os) & (entry_type const &, decltype(os) &)
		> &entry_dumper;
		boost::dynamic_bitset<> b;
		string_type last_name;
		wrapper_type last_entry;

		tree_dumper(
			decltype(os) &os_, decltype(entry_dumper) &entry_dumper_
		)
		: os(os_), entry_dumper(entry_dumper_)
		{}

		tree_dumper(
			decltype(os) &os_,
			decltype(entry_dumper) &entry_dumper_,
			decltype(b) const &b_,
			bool next_bit
		) : os(os_), entry_dumper(entry_dumper_), b(b_)
		{
			b.push_back(next_bit);
		}

		void operator()(string_type const &s, wrapper_type d);
	};

	struct child_finder {
		child_finder(wrapper_type &p_, func_type f_, bool create_)
		: p(p_), f(f_), create(create_) {}

		template <typename context_type>
		void operator()(string_type const &name, context_type &ctx,
				bool &pass) const;

		wrapper_type &p;
		func_type f;
		bool create;
	};

	std::mutex m_lock;
	string_type entry_name;
	boost::spirit::qi::symbols<char_type, wrapper_type> children;
	wrapper_type parent;
	entry_type entry;
};

}

#endif
