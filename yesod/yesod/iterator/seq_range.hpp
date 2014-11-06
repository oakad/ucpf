/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_YESOD_ITERATOR_SEQ_RANGE_20141106T1910)
#define UCPF_YESOD_ITERATOR_SEQ_RANGE_20141106T1910

#include <yesod/iterator/facade.hpp>
#include <yesod/mpl/common_base.hpp>

namespace ucpf { namespace yesod { namespace iterator {

template <typename Sequence>
struct seq_range {
	typedef Sequence container_type;
	typedef typename container_type::value_type nested_type;
	typedef typename nested_type::value_type value_type;
	typedef typename nested_type::reference reference;
	typedef typename nested_type::const_reference const_reference;
	typedef typename nested_type::size_type size_type;
	typedef typename nested_type::difference_type difference_type;

	typedef typename mpl::common_base<
		typename std::iterator_traits<
			typename container_type::iterator
		>::iterator_category,
		typename std::iterator_traits<
			typename nested_type::iterator
		>::iterator_category
	>::type iterator_category;

	template <bool MakeConst>
	struct iterator_base : facade<
		iterator_base<MakeConst>,
		typename std::conditional<
			MakeConst, value_type const, value_type
		>::type,
		iterator_category,
		typename std::conditional<
			MakeConst, const_reference, reference
		>::type,
		difference_type
	> {

	private:
		friend struct core_access;
		friend struct seq_range;

		template <bool OtherConst>
		bool equal(iterator_base<OtherConst> const &other) const
		{
			return (c_iter == other.c_iter) && (
				(n_iter == other.n_iter)
				|| (c_iter == s->seq.end())
			);
		}

		void increment()
		{
			++n_iter;
			while (n_iter == c_iter->end()) {
				++c_iter;
				if (c_iter == s->seq.end())
					return;
				n_iter = c_iter->begin();
			}
		}

		void decrement()
		{
			if (c_iter == s->seq.end()) {
				--c_iter;
				n_iter = c_iter->end();
			}

			while (n_iter == c_iter->begin()) {
				if (c_iter == s->seq.begin())
					return;
				--c_iter;
				n_iter = c_iter->end();
			}

			--n_iter;
		}

		void advance(typename iterator_base::difference_type n)
		{
			if (n < 0) {
				advance_back(-n);
				return;
			}

			while (true) {
				auto d(std::distance(n_iter, c_iter->end()));
				if (d > n) {
					std::advance(n_iter, n);
					return;
				}
				n -= d;
				n_iter = c_iter->end();
				++c_iter;
				if (c_iter == s->seq.end())
					return;

				n_iter = c_iter->begin();
			}
		}

		void advance_back(typename iterator_base::difference_type n)
		{
			while (true) {
				auto d(std::distance(c_iter->begin(), n_iter));
				if (d > n) {
					std::advance(n_iter, -n);
					return;
				}
				n -= d;
				n_iter = c_iter->begin();
				if (c_iter == s->seq.begin())
					return;
				--c_iter;
				n_iter = c_iter->end();
			}
		}

		template <bool OtherConst>
		typename iterator_base::difference_type distance_to(
			iterator_base<OtherConst> const &other
		) const
		{
			auto c_diff(std::distance(c_iter, other.c_iter));
			if (!c_diff)
				return std::distance(n_iter, other.n_iter);
			else if (c_diff < 0)
				return -other.distance_to(*this);

			auto x_iter(c_iter);
			auto rv(std::distance(n_iter, c_iter->end()));
			++x_iter;
			while (x_iter != other.c_iter) {
				rv += std::distance(
					x_iter->begin(), x_iter->end()
				);
				++x_iter;
			}
			rv += std::distance(x_iter->begin(), other.n_iter);
			return rv;
		}

		typename iterator_base::reference dereference() const
		{
			return *n_iter;
		}

		seq_range *s;
		typename std::conditional<
			MakeConst,
			typename container_type::const_iterator,
			typename container_type::iterator
		>::type c_iter;
		typename std::conditional<
			MakeConst,
			typename nested_type::const_iterator,
			typename nested_type::iterator
		>::type n_iter;

		iterator_base()
		: s(nullptr)
		{}

		iterator_base(seq_range *s_, decltype(c_iter) c_iter_)
		: s(s_), c_iter(c_iter_)
		{}

		iterator_base(
			seq_range *s_,
			decltype(c_iter) c_iter_,
			decltype(n_iter) n_iter_
		) : s(s_), c_iter(c_iter_), n_iter(n_iter_)
		{}
	};

	typedef iterator_base<false> iterator;
	typedef iterator_base<
		!std::is_const<value_type>::value
	> const_iterator;

	seq_range(container_type &seq_)
	: seq(seq_)
	{}

	iterator begin()
	{
		return iterator(this, seq.begin(), seq.begin()->begin());
	}

	const_iterator begin() const
	{
		return const_iterator(this, seq.begin(), seq.begin()->begin());
	}

	iterator end()
	{
		return iterator(this, seq.end());
	}

	const_iterator end() const
	{
		return const_iterator(this, seq.end());
	}

	size_type size() const
	{
		size_type rv(0);
		for (auto const &s: seq)
			rv += s.size();

		return rv;
	}

private:
	container_type &seq;
};

template <typename Sequence>
auto make_seq_range(Sequence &s)
{
	return seq_range<Sequence>(s);
}

}}}

#endif
