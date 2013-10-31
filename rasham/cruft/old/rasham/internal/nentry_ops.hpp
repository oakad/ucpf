/*
 * Copyright (C) 2012 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(_RASHAM_INTERNAL_NENTRY_OPS_HPP)
#define _RASHAM_INTERNAL_NENTRY_OPS_HPP

namespace rasham
{

template <typename entry_type, typename char_type, typename traits_type>
void nentry_t<entry_type, char_type, traits_type>::tree_dumper::operator()(
	string_type const &s, wrapper_type d
)
{
	if (!last_entry) {
		last_name = s;
		last_entry = d;
		return;
	}

	for (size_t cnt(0); cnt < b.size(); ++cnt) {
		if (b.test(cnt))
			os << "|   ";
		else
			os << "    ";
	}

	bool not_last(d != last_entry);

	if (not_last)
		os << '|';
	else
		os << '`';

	last_entry->lock();
	os << "-- \'" << last_entry->name() << "\' <"
	   << static_cast<void *>(last_entry.get()) << "> ";
	{
		auto &e(last_entry->get_entry());
		entry_dumper(e, os);
	}
	os << '\n';
	tree_dumper dumper(
		os, entry_dumper, b, not_last
	);
	last_entry->children.for_each(std::ref(dumper));
	if (dumper.last_entry)
		dumper(dumper.last_name, dumper.last_entry);

	last_entry->unlock();
	last_name = s;
	last_entry = d;
}

template <typename entry_type, typename char_type, typename traits_type>
template <typename context_type>
void nentry_t<entry_type, char_type, traits_type>::child_finder::operator()(
	string_type const &name, context_type &ctx, bool &pass
) const
{
	wrapper_type q;

	if (p) {
		std::lock_guard<std::mutex> l_g(p->m_lock);
		if (!f(p)) {
			pass = false;
			return;
		}

		auto child(p->children.find(name));
		if (child)
			q = *child;
		else if (create) {
			q = make_counted<self_type>(name, p);
			p->children.add(name, q);
		}
	} else {
		pass = false;
		return;
	}

	pass = bool(q);
	p = q;
}

template <typename entry_type, typename char_type, typename traits_type>
auto nentry_t<entry_type, char_type, traits_type>::apply_to_path(
	wrapper_type r, string_type path, func_type f, bool create
) -> wrapper_type
{
	child_finder finder(r, f, create);

	qi::rule<typename string_type::iterator, string_type()> elem_rule(
		+(('\\' || qi::char_) - '/')
	);
	qi::rule<typename string_type::iterator, void()> path_rule(
		(-qi::lit('/')) >> (elem_rule[finder] % qi::lit('/'))
	);

	if (qi::parse(path.begin(), path.end(), path_rule)) {
		if (finder.p) {
			std::lock_guard<std::mutex> l_g(finder.p->m_lock);
			f(finder.p);
		}
	}

	return finder.p;
}

template <typename entry_type, typename char_type, typename traits_type>
void nentry_t<entry_type, char_type, traits_type>::apply_up(
	wrapper_type r, func_type f
)
{
	wrapper_type p;

	while (r) {
		r->lock();
		if (!f(r)) {
			r->unlock();
			break;
		}
		p = r->parent;
		r->unlock();
		r = p;
	}
}

template <typename entry_type, typename char_type, typename traits_type>
template <bool postorder>
void nentry_t<entry_type, char_type, traits_type>::apply_down(
	wrapper_type r, func_type f
)
{
	if (!r)
		return;

	std::lock_guard<std::mutex> l_g(r->m_lock);

	if (!postorder) {
		if (!f(r))
			return;

		r->children.for_each(
			[f](string_type const &s, wrapper_type d) {
				self_type::apply_down(d, f);
			}
		);
	} else {
		r->children.for_each(
			[f](string_type const &s, wrapper_type d) {
				self_type::apply_down<true>(d, f);
			}
		);
		f(r);
	}
}

template <typename entry_type, typename char_type, typename traits_type>
void nentry_t<entry_type, char_type, traits_type>::cull_tree(wrapper_type r)
{
	apply_down<true>(
		r, [](wrapper_type r) -> bool {
			r->parent.reset();
			r->children.clear();
			return true;
		}
	);
}

}
#endif
