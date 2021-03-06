/*
 * Copyright (c) 2016 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_1DC3219B893A9C61565ECE60B2DCEDBD)
#define HPP_1DC3219B893A9C61565ECE60B2DCEDBD

#include <iterator>

namespace ucpf { namespace holam { namespace output {

template <typename ValueType = char, std::size_t BlockSize = 256>
struct collector {
	typedef ValueType value_type;
	typedef void difference_type;
	typedef void pointer;
	typedef void reference;
	typedef std::output_iterator_tag iterator_category;

	static_assert(
		std::is_pod<value_type>::value,
		"std::is_pod<value_type>::value"
	);

	collector()
	: cur(nullptr), first(nullptr), last(nullptr), cur_offset(0)
	{}

	~collector()
	{
		if (cur)
			delete cur;

		while (first) {
			auto p(first->header.next);
			delete first;
			first = p;
		}
	}

	collector &operator=(value_type ch)
	{
		if (cur_offset == data_size) {
			if (last) {
				last->header.next = cur;
				last = cur;
			} else {
				first = cur;
				last = cur;
			}

			cur = new block();
			cur_offset = 0;
		} else if (!cur)
			cur = new block();

		cur->data[cur_offset++] = ch;
		return *this;
	}

	collector &operator*()
	{
		return *this;
	}

	collector &operator++()
	{
		return *this;
	}

	collector &operator++(int)
	{
		return *this;
	}

	template <std::size_t StrSize>
	friend bool operator==(
		collector const &v0, value_type const (&v1)[StrSize]
	)
	{
		std::size_t pos(0);
		auto p(v0.first);
		while (p) {
			if ((StrSize - pos) < data_size)
				return false;
			if (::memcmp(p->data, &v1[pos], sizeof(p->data)))
				return false;

			p = p->header.next;
			pos += data_size;
		}

		if (v0.cur) {
			if ((StrSize - pos) != v0.cur_offset)
				return false;
			return !::memcmp(
				v0.cur->data, &v1[pos],
				v0.cur_offset * sizeof(value_type)
			);
		}

		return pos == StrSize;
	}

	friend bool operator==(
		collector const &v0, value_type const * const &v1
	)
	{
		std::size_t pos(0);
		auto p(v0.first);
		while (p) {
			for (
				std::size_t s_pos(0); s_pos < data_size;
				++s_pos
			) {
				if (!v1[pos] || (v1[pos] != p->data[s_pos]))
					return false;
				++pos;
			}

			p = p->header.next;
		}

		if (v0.cur) {
			for (
				std::size_t s_pos(0); s_pos < v0.cur_offset;
				++s_pos
			) {
				if (!v1[pos] || (
					v1[pos] != v0.cur->data[s_pos]
				))
					return false;
				++pos;
			}

		}

		return !v1[pos];
	}

	template <typename Func>
	void apply(Func &&f) const
	{
		for (auto p(first); p; p = p->header.next)
			f(p->data, data_size);

		if (cur)
			f(cur->data, cur_offset);
	}

	std::size_t count() const
	{
		std::size_t rv(0);
		apply([&rv](auto data, std::size_t count_) {
			rv += count_;
		});
		return rv;
	}

private:
	struct block;

	struct block_header {
		block *next = nullptr;
	};

	static constexpr auto data_size = (
		BlockSize - sizeof(block_header)
	) / sizeof(value_type);

	struct block {
		block_header header;
		value_type data[data_size];
	};

	block *cur;
	block *first;
	block *last;
	std::size_t cur_offset;
};

}}}
#endif
