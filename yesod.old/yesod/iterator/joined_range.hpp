/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */
#if !defined(UCPF_YESOD_ITERATOR_JOINED_RANGE_20140327T1630)
#define UCPF_YESOD_ITERATOR_JOINED_RANGE_20140327T1630

#include <functional>

#include <yesod/mpl/min_max.hpp>
#include <yesod/mpl/common_base.hpp>
#include <yesod/mpl/package_range_c.hpp>

#include <yesod/iterator/facade.hpp>

namespace ucpf { namespace yesod { namespace iterator {

template <typename... Rn>
struct joined_range {
private:
	template <typename... Iterator>
	struct base_iterator_traits_ {
		typedef typename std::common_type<
			typename std::iterator_traits<Iterator>::value_type...
		>::type value_type;

		typedef typename std::common_type<
			typename std::iterator_traits<
				Iterator
			>::difference_type...
		>::type difference_type;

		typedef typename mpl::common_base<
			typename std::iterator_traits<
				Iterator
			>::iterator_category...
		>::type iterator_category;

		typedef typename mpl::max<
			mpl::integral_constant<
				std::size_t, std::alignment_of<Iterator>::value
			>...
		>::type alignment;

		typedef typename mpl::max<
			mpl::integral_constant<
				std::size_t, sizeof(Iterator)
			>...
		>::type size;

		typedef typename std::aligned_storage<
			size::value, alignment::value
		>::type storage_type;
	};

	typedef base_iterator_traits_<
		decltype(std::begin(std::declval<Rn>()))...
	> base_iterator_traits;

public:
	typedef typename base_iterator_traits::value_type value_type;
	typedef typename base_iterator_traits::iterator_category
	iterator_category;
	typedef std::size_t size_type;
	typedef typename base_iterator_traits::difference_type difference_type;

private:
	struct slice_base {
		typedef typename base_iterator_traits::storage_type
		storage_type;
		typedef typename base_iterator_traits::value_type &reference;
		typedef typename base_iterator_traits::value_type const
		&const_reference;

		virtual ~slice_base()
		{}

		virtual void put(storage_type *iter) = 0;
		virtual void get_first(storage_type *iter) = 0;
		virtual void get_last(storage_type *iter) = 0;
		virtual bool equal(
			storage_type const *iter, storage_type const *other
		) const = 0;
		virtual bool increment(storage_type *iter) = 0;
		virtual bool decrement(storage_type *iter) = 0;
		virtual reference dereference(
			storage_type const *iter
		) const = 0;
		virtual size_type size() const = 0;
		virtual std::pair<size_type, size_type> location(
			storage_type const *iter
		) const = 0;
		virtual difference_type distance_to(
			storage_type const *iter, storage_type const *other
		) const = 0;
		virtual std::pair<difference_type, int> advance(
			storage_type *iter, difference_type n
		) = 0;
	};
public:
	template <typename ValueType>
	struct iterator_base : facade<
		iterator_base<ValueType>, ValueType, iterator_category,
		ValueType &, difference_type
	> {
		~iterator_base()
		{
			if (r && (slice_pos < r->s_access.size()))
				r->s_access[slice_pos]->put(&iter);
		}

	private:
		friend struct core_access;
		friend struct joined_range;

		iterator_base(joined_range *r_, std::size_t slice_pos_)
		: r(r_), slice_pos(slice_pos_)
		{
			if (r && (slice_pos < r->s_access.size()))
				r->s_access[slice_pos]->get_first(&iter);
		}

		template <typename ValueType1>
		bool equal(iterator_base<ValueType1> const &other) const
		{
			if ((r != other.r) || (slice_pos != other.slice_pos))
				return false;

			if (slice_pos >= r->s_access.size())
				return true;

			return r->s_access[slice_pos]->equal(
				&iter, &other.iter
			);
		}

		void increment()
		{
			if (slice_pos >= r->s_access.size())
				return;

			if (r->s_access[slice_pos]->increment(&iter)) {
				r->s_access[slice_pos]->put(&iter);
				++slice_pos;

				if (slice_pos < r->s_access.size())
					r->s_access[slice_pos]->get_first(
						&iter
					);

			}
		}

		void decrement()
		{
			if (slice_pos >= r->s_access.size())
				return;

			if (r->s_access[slice_pos]->decrement(&iter)) {
				if (slice_pos > 0) {
					r->s_access[slice_pos]->put(&iter);
					--slice_pos;
					r->s_access[slice_pos]->get_last(
						&iter
					);
				}
			}
		}

		void advance(typename iterator_base::difference_type n)
		{
			int carry;
			while (true) {
				std::tie(n, carry) = r->s_access[
					slice_pos
				]->advance(&iter, n);

				if (n > 0 || carry > 0) {
					r->s_access[slice_pos]->put(&iter);
					++slice_pos;

					if (slice_pos < r->s_access.size()) {
						r->s_access[
							slice_pos
						]->get_first(&iter);
					}
				} else if (n < 0 || carry < 0) {
					if (slice_pos) {
						r->s_access[
							slice_pos
						]->put(&iter);
						--slice_pos;
						r->s_access[
							slice_pos
						]->get_last(&iter);
					}
				} else
					break;

				if (!n)
					break;
			}
		}

		template <typename ValueType1>
		typename iterator_base::difference_type distance_to(
			iterator_base<ValueType1> const &other
		) const
		{
			if (slice_pos == other.slice_pos)
				return r->s_access[slice_pos]->distance_to(
					&iter, &other.iter
				);

			auto f_pos(std::min(slice_pos, other.slice_pos));
			auto l_pos(std::max(slice_pos, other.slice_pos));
			typename iterator_base::difference_type rv(0);

			for (auto pos(f_pos + 1); pos < l_pos; ++pos)
				rv += r->s_access[pos]->size();

			if (other.slice_pos > slice_pos) {
				if (other.slice_pos < other.r->s_access.size())
					rv += r->s_access[
						other.slice_pos
					]->location(&other.iter).first;

					rv += r->s_access[slice_pos]->location(
						&iter
					).second;
			} else {
				rv = -rv;
				rv -= r->s_access[other.slice_pos]->location(
					&other.iter
				).second;

				if (slice_pos < r->s_access.size())
					rv -= r->s_access[slice_pos]->location(
						&iter
					).first;
			}
			return rv;
		}

		typename iterator_base::reference dereference() const
		{
			return r->s_access[slice_pos]->dereference(&iter);
		}

		joined_range *r;
		std::size_t slice_pos;
		typename base_iterator_traits::storage_type iter;
	};

	typedef iterator_base<value_type> iterator;

	typedef typename std::conditional<
		std::is_const<value_type>::value,
		iterator_base<value_type>,
		iterator_base<typename std::add_const<value_type>::type>
	>::type const_iterator;

	joined_range(Rn&&... r)
	: slices(std::make_tuple(std::ref(r), this)...)
	{}

	iterator begin()
	{
		return iterator(this, 0);
	}

	iterator end()
	{
		return iterator(this, s_access.size());
	}

	const_iterator begin() const
	{
		return const_iterator(this, 0);
	}

	const_iterator end() const
	{
		return const_iterator(this, s_access.size());
	}

	size_type size() const
	{
		size_type rv(0);

		for (auto s: s_access)
			rv += s->size();

		return rv;
	}

private:
	template <typename Range, std::size_t N>
	struct slice : slice_base {
		typedef decltype(
			std::begin(std::declval<Range>())
		) iterator;
		typedef std::allocator<iterator> allocator_type;
		typedef std::allocator_traits<allocator_type> allocator_traits;

		template <typename P>
		slice(P &&p)
		: first(std::begin(std::get<0>(p))),
		  last(std::end(std::get<0>(p)))
		{
			std::get<1>(p)->s_access[N]
			= dynamic_cast<slice_base *>(this);
		}

		virtual void put(typename slice_base::storage_type *iter)
		{
			allocator_type a;
			allocator_traits::destroy(
				a, reinterpret_cast<iterator *>(iter)
			);
		}

		virtual void get_first(typename slice_base::storage_type *iter)
		{
			allocator_type a;
			allocator_traits::construct(
				a, reinterpret_cast<iterator *>(iter), first
			);
		}

		virtual void get_last(typename slice_base::storage_type *iter)
		{
			auto x_last(last);
			--x_last;
			allocator_type a;
			allocator_traits::construct(
				a, reinterpret_cast<iterator *>(iter), x_last
			);
		}

		virtual bool equal(
			typename slice_base::storage_type const *iter,
			typename slice_base::storage_type const *other
		) const
		{
			auto &x_iter(
				*reinterpret_cast<iterator const *>(iter)
			);
			auto &x_other(
				*reinterpret_cast<iterator const *>(other)
			);
			return x_iter == x_other;
		}

		virtual bool increment(typename slice_base::storage_type *iter)
		{
			auto &x_iter(*reinterpret_cast<iterator *>(iter));
			if (x_iter != last)
				++x_iter;

			return x_iter == last;
		}

		virtual bool decrement(typename slice_base::storage_type *iter)
		{
			auto &x_iter(*reinterpret_cast<iterator *>(iter));
			if (x_iter == first)
				return true;

			--x_iter;
			return false;
		}

		virtual typename slice_base::reference dereference(
			typename slice_base::storage_type const *iter
		) const
		{
			auto &x_iter(*reinterpret_cast<iterator const *>(iter));
			return const_cast<typename slice_base::reference>(
				*x_iter
			);
		}

		virtual size_type size() const
		{
			return std::distance(first, last);
		}

		virtual std::pair<size_type, size_type> location(
			typename slice_base::storage_type const *iter
		) const
		{
			auto &x_iter(*reinterpret_cast<iterator const *>(iter));
			return std::make_pair(
				std::distance(first, x_iter),
				std::distance(x_iter, last)
			);
		}

		virtual difference_type distance_to(
			typename slice_base::storage_type const *iter,
			typename slice_base::storage_type const *other
		) const
		{
			auto &x_iter(
				*reinterpret_cast<iterator const *>(iter)
			);
			auto &x_other(
				*reinterpret_cast<iterator const *>(other)
			);

			return std::distance(x_iter, x_other);
		}

		virtual std::pair<difference_type, int> advance(
			typename slice_base::storage_type *iter,
			difference_type n
		)
		{
			auto &x_iter(
				*reinterpret_cast<iterator *>(iter)
			);
			if (n >= 0) {
				auto x_n(std::distance(x_iter, last));
				std::advance(x_iter, std::min(n, x_n));
				return std::make_pair(
					n - std::min(n, x_n),
					x_iter == last ? 1 : 0
				);
			} else {
				auto x_n(std::distance(x_iter, first));
				std::advance(x_iter, std::max(n, x_n));
				return std::make_pair(
					n - std::max(n, x_n),
					x_iter == first ? -1 : 0
				);
			}
		}

		iterator first;
		iterator last;
	};

	struct slice_pack_type {
		typedef typename mpl::package_range_c<
			std::size_t, 0, sizeof...(Rn)
		>::type indices;

		template <typename Ix, Ix... Cn>
		struct apply {
			typedef std::tuple<slice<Rn, Cn>...> type;
		};
	};

	typename mpl::apply_pack_c<
		typename slice_pack_type::indices,
		slice_pack_type::template apply
	>::type slices;
	std::array<slice_base *, sizeof...(Rn)> s_access;
};

template <typename... Range>
joined_range<Range...> make_joined_range(Range&&... r)
{
	return joined_range<Range...>(std::forward<Range>(r)...);
}

}}}

#endif
