/*
 * Copyright (c) 2013 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_YESOD_MPL_SEQUENCE_FWD_DEC_11_2013_1410)
#define UCPF_YESOD_MPL_SEQUENCE_FWD_DEC_11_2013_1410

namespace ucpf { namespace yesod { namespace mpl {
namespace detail {

template <typename Tag> struct advance_impl;
template <typename Tag> struct at_impl;
template <typename Tag> struct back_impl;
template <typename Tag> struct begin_impl;
template <typename Tag> struct clear_impl;
template <typename Tag> struct contains_impl;
template <typename Tag> struct count_impl;
template <typename Tag> struct distance_impl;
template <typename Tag> struct erase_impl;
template <typename Tag> struct erase_key_impl;
template <typename Tag> struct empty_impl;
template <typename Tag> struct end_impl;
template <typename Tag> struct front_impl;
template <typename Tag> struct has_key_impl;
template <typename Tag> struct has_push_back_impl;
template <typename Tag> struct insert_impl;
template <typename Tag> struct insert_range_impl;
template <typename Tag> struct key_type_impl;
template <typename Tag> struct O1_size_impl;
template <typename Tag> struct order_impl;
template <typename Tag> struct pop_back_impl;
template <typename Tag> struct pop_front_impl;
template <typename Tag> struct push_back_impl;
template <typename Tag> struct push_front_impl;
template <typename Tag> struct size_impl;
template <typename Tag> struct value_type_impl;

}

template <typename...> struct advance;
template <typename...> struct at;

template <typename...> struct back;
template <typename...> struct begin;
template <typename...> struct clear;

template <typename...> struct distance;

template <typename...> struct contains;
template <typename...> struct count;

template <typename...> struct empty;
template <typename...> struct end;

template <typename...> struct erase;

template <typename...> struct erase_key;

template <typename...> struct front;

template <typename...> struct has_key;

template <typename...> struct insert;

template <typename...> struct insert_range;

template <typename...> struct key_type;

template <typename...> struct O1_size;

template <typename...> struct order;

template <typename...> struct pop_back;
template <typename...> struct pop_front;

template <typename...> struct push_back;
template <typename...> struct push_front;

template <typename...> struct size;

template <typename...> struct value_type;

}}}

#endif
