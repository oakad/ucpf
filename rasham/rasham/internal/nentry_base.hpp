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

template <typename entry_type>
struct nentry_t;

template <typename entry_type>
using nentry = counted_ptr<nentry_t<entry_type>>;

template <typename entry_type>
using nentry_func = std::function<bool (nentry<entry_type>)>;

template <typename entry_type>
struct nentry_t {
	friend struct ref_count_val<nentry_t<entry_type>>;

	static nentry<entry_type> make(std::string name_ = std::string())
	{
		auto p(make_counted<nentry_t<entry_type>>(name_));
		return p;
	}

	static nentry<entry_type> get_child(
		std::string name_, nentry<entry_type> parent_,
		bool create = false
	)
	{
		std::lock_guard<std::mutex> l_g(parent_->m_lock);
		auto child(parent_->children.find(name_));
		if (child)
			return nentry<entry_type>(*child);

		if (!create)
			return nentry<entry_type>();

		nentry<entry_type> p(make_counted<nentry_t<entry_type>>(
			name_, parent_)
		);
		parent_->children.add(name_, p);
		return p;
	}

	static nentry<entry_type> apply_to_path(
		nentry<entry_type> r, char const *path,
		nentry_func<entry_type> f, bool create = false
	);

	static void apply_up(
		nentry<entry_type> r, nentry_func<entry_type> f
	);

	template <bool postorder = false>
	static void apply_down(
		nentry<entry_type> r, nentry_func<entry_type> f
	);

	static nentry<entry_type> lookup_path(
		nentry<entry_type> r, char const *path, bool create = false
	)
	{
		return apply_to_path(
			r, path,
			[](nentry<entry_type> r_) -> bool { return true; },
			create
		);
	}

	template <typename char_type, typename traits_type>
	static std::basic_ostream<char_type, traits_type> &dump(
		nentry<entry_type> r,
		std::basic_ostream<char_type, traits_type> &os,
		std::function<decltype(os) & (entry_type const &,
					      decltype(os) &)> entry_dumper
	)
	{
		if (r) {
			std::lock_guard<std::mutex> l_g(r->m_lock);
			os << '\'' << r->children.name() << "\' <"
			   << static_cast<void *>(r.get()) << "> ";
			{
				auto &e(r->get_entry());
				entry_dumper(e, os);
			}
			os << '\n';
			tree_dumper<char_type, traits_type> dumper(
				os, entry_dumper
			);
			r->children.for_each(std::ref(dumper));
			if (dumper.last_entry)
				dumper(dumper.last_name, dumper.last_entry);

			return dumper.os;
		} else
			return os;
	}

	static void cull_tree(nentry<entry_type> r);

	virtual ~nentry_t()
	{
		if (parent) {
			std::lock_guard<std::mutex> l_g(parent->m_lock);

			parent->children.remove(children.name());
		}
	}

	std::string name() const
	{
		return children.name();
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
	nentry_t(std::string name_)
	: children(name_) {}

	nentry_t(std::string name_, nentry<entry_type> parent_)
	: children(name_), parent(parent_) {}

	template <typename char_type, typename traits_type>
	struct tree_dumper {
		std::basic_ostream<char_type, traits_type> &os;
		std::function<
			decltype(os) & (entry_type const &, decltype(os) &)
		> &entry_dumper;
		boost::dynamic_bitset<> b;
		std::basic_string<char_type, traits_type> last_name;
		nentry<entry_type> last_entry;

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

		void operator()(
			std::basic_string<char_type, traits_type> const &s,
			nentry<entry_type> d
		);
	};

	struct child_finder {
		child_finder(
			nentry<entry_type> &p_,
			nentry_func<entry_type> f_, bool create_
		)
		: p(p_), f(f_), create(create_) {}

		template <typename context_type>
		void operator()(std::string const &name, context_type &ctx,
				bool &pass) const;

		nentry<entry_type> &p;
		nentry_func<entry_type> f;
		bool create;
	};

	std::mutex m_lock;
	boost::spirit::qi::symbols<char, nentry<entry_type>> children;
	nentry<entry_type> parent;
	entry_type entry;
};

}

#endif
