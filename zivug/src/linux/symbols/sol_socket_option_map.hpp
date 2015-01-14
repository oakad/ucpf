/*
 * Copyright (c) 2014-2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_66280DFEDF8D881285AB0CC1D6D4395B)
#define HPP_66280DFEDF8D881285AB0CC1D6D4395B

struct sol_socket_option_map {
	/*
	 *    1 debug
	 *    2 reuseaddr
	 *    3 type
	 *    4 error
	 *    5 dontroute
	 *    6 broadcast
	 *    7 sndbuf
	 *    8 rcvbuf
	 *    9 sndbufforce
	 *   10 rcvbufforce
	 *   11 keepalive
	 *   12 oobinline
	 *   13 no_check
	 *   14 priority
	 *   15 linger
	 *   16 bsdcompat
	 *   17 reuseport
	 *   18 passcred
	 *   19 peercred
	 *   20 rcvlowat
	 *   21 sndlowat
	 *   22 rcvtimeo
	 *   23 sndtimeo
	 *   24 security_authentication
	 *   25 security_encryption_transport
	 *   26 security_encryption_network
	 *   27 bindtodevice
	 *   28 attach_filter
	 *   29 detach_filter
	 *   30 get_filter
	 *   31 peername
	 *   32 timestamp
	 *   33 acceptconn
	 *   34 peersec
	 *   35 passsec
	 *   36 timestampns
	 *   37 mark
	 *   38 timestamping
	 *   39 protocol
	 *   40 domain
	 *   41 rxq_ovfl
	 *   42 wifi_status
	 *   43 peek_off
	 *   44 nofcs
	 *   45 lock_filter
	 *   46 select_err_queue
	 *   47 busy_poll
	 *   48 max_pacing_rate
	 *   49 bpf_extensions
	 *   50 incoming_cpu
	 *   51 attach_bpf
	 *   52 detach_bpf
	 */

	struct locus {
		locus()
		: offset(-1), tail_pos(0)
		{}

		explicit operator bool() const
		{
			return offset >= 0;
		}

	private:
		friend struct sol_socket_option_map;

		locus(int offset_, int tail_pos_)
		: offset(offset_), tail_pos(tail_pos_)
		{}

		int offset;
		int tail_pos;
	};

	static locus search_root()
	{
		return locus(0, 0);
	}

	template <typename Iterator>
	static locus locate_rel(locus loc, Iterator first, Iterator last)
	{
		int l_pos(loc.offset), n_pos(l_pos);		

		if (base_ref[l_pos].base >= 0) {
			for (; first != last; ++first) {
				if (!is_valid(*first))
					return locus();

				n_pos = base_ref[l_pos].base + *first
					- char_offset;

				if (n_pos >= int(base_ref_size))
					return locus();

				if (int(base_ref[n_pos].check) != l_pos)
					return locus();

				if (int(base_ref[n_pos].base) < 0)
					break;

				l_pos = n_pos;
			}

			if (first == last)
				return locus(n_pos, 0);
			else
				++first;
		}

		
		auto &r(tail_ref[-1 - base_ref[n_pos].base]);
		auto r_first(&tail[r.offset]);
		auto d(std::distance(first, last));

		if (d > (r.size - loc.tail_pos))
			return locus();

		while (first != last) {
			if (*first != r_first[loc.tail_pos])
				return locus();

			++first;
			++loc.tail_pos;
		}
		return locus(n_pos, loc.tail_pos);
	}

	template <typename Iterator>
	static int find_rel(locus loc, Iterator first, Iterator last)
	{
		loc = locate_rel(loc, first, last);
		if (!loc)
			return 0;

		auto b(base_ref[loc.offset].base);
		if (b >= 0) {
			auto n_pos(b + term_char - char_offset);
			if (base_ref[n_pos].check != loc.offset)
				return 0;
			else
				return -base_ref[n_pos].base;
		} else {
			auto &r(tail_ref[-1 - b]);
			if (r.size == loc.tail_pos)
				return -b;
			else
				return 0;
		} 
	}

	template <typename Iterator>
	static int find(Iterator first, Iterator last)
	{
		return find_rel(search_root(), first, last);
	}

private:
	constexpr static bool is_valid(int c)
	{
		return (term_char == (char_offset + 1))
		       ? ((c > term_char) && (c <= (term_char + char_count)))
		       : ((c > char_offset) && (c < term_char));
	}

	constexpr static int char_offset = 93;
	constexpr static int char_count = 28;
	constexpr static int term_char = 94;

	constexpr static std::size_t tail_size = 267;

	constexpr static uint8_t tail[tail_size] = {
		0x75, 0x67, 0x64, 0x64, 0x72, 0x70, 0x65, 0x72,
		0x72, 0x6f, 0x72, 0x74, 0x72, 0x6f, 0x75, 0x74,
		0x65, 0x6f, 0x61, 0x64, 0x63, 0x61, 0x73, 0x74,
		0x6f, 0x72, 0x63, 0x65, 0x6f, 0x72, 0x63, 0x65,
		0x65, 0x65, 0x70, 0x61, 0x6c, 0x69, 0x76, 0x65,
		0x6f, 0x62, 0x69, 0x6e, 0x6c, 0x69, 0x6e, 0x65,
		0x63, 0x68, 0x65, 0x63, 0x6b, 0x6f, 0x72, 0x69,
		0x74, 0x79, 0x6e, 0x67, 0x65, 0x72, 0x64, 0x63,
		0x6f, 0x6d, 0x70, 0x61, 0x74, 0x6f, 0x72, 0x74,
		0x72, 0x65, 0x64, 0x72, 0x65, 0x64, 0x6f, 0x77,
		0x61, 0x74, 0x6f, 0x77, 0x61, 0x74, 0x69, 0x6d,
		0x65, 0x6f, 0x69, 0x6d, 0x65, 0x6f, 0x75, 0x74,
		0x68, 0x65, 0x6e, 0x74, 0x69, 0x63, 0x61, 0x74,
		0x69, 0x6f, 0x6e, 0x72, 0x61, 0x6e, 0x73, 0x70,
		0x6f, 0x72, 0x74, 0x65, 0x74, 0x77, 0x6f, 0x72,
		0x6b, 0x6e, 0x64, 0x74, 0x6f, 0x64, 0x65, 0x76,
		0x69, 0x63, 0x65, 0x69, 0x6c, 0x74, 0x65, 0x72,
		0x69, 0x6c, 0x74, 0x65, 0x72, 0x65, 0x74, 0x5f,
		0x66, 0x69, 0x6c, 0x74, 0x65, 0x72, 0x61, 0x6d,
		0x65, 0x63, 0x65, 0x70, 0x74, 0x63, 0x6f, 0x6e,
		0x6e, 0x65, 0x63, 0x65, 0x63, 0x73, 0x6b, 0x6e,
		0x67, 0x74, 0x6f, 0x63, 0x6f, 0x6c, 0x61, 0x69,
		0x6e, 0x71, 0x5f, 0x6f, 0x76, 0x66, 0x6c, 0x69,
		0x66, 0x69, 0x5f, 0x73, 0x74, 0x61, 0x74, 0x75,
		0x73, 0x5f, 0x6f, 0x66, 0x66, 0x63, 0x73, 0x63,
		0x6b, 0x5f, 0x66, 0x69, 0x6c, 0x74, 0x65, 0x72,
		0x65, 0x63, 0x74, 0x5f, 0x65, 0x72, 0x72, 0x5f,
		0x71, 0x75, 0x65, 0x75, 0x65, 0x73, 0x79, 0x5f,
		0x70, 0x6f, 0x6c, 0x6c, 0x5f, 0x70, 0x61, 0x63,
		0x69, 0x6e, 0x67, 0x5f, 0x72, 0x61, 0x74, 0x65,
		0x66, 0x5f, 0x65, 0x78, 0x74, 0x65, 0x6e, 0x73,
		0x69, 0x6f, 0x6e, 0x73, 0x6e, 0x63, 0x6f, 0x6d,
		0x69, 0x6e, 0x67, 0x5f, 0x63, 0x70, 0x75, 0x70,
		0x66, 0x70, 0x66
	};

	struct tail_ref_type {
		uint16_t offset;
		uint16_t size;
	};

	constexpr static std::size_t tail_ref_size = 52;

	constexpr static tail_ref_type tail_ref[tail_ref_size] = {
		{0, 2}, {2, 3}, {5, 2}, {7, 4},
		{11, 6}, {17, 7}, {24, 0}, {24, 0},
		{24, 4}, {28, 4}, {32, 8}, {40, 8},
		{48, 5}, {53, 5}, {58, 4}, {62, 7},
		{69, 3}, {72, 3}, {75, 3}, {78, 4},
		{82, 4}, {86, 4}, {90, 4}, {94, 13},
		{107, 8}, {115, 6}, {121, 10}, {131, 5},
		{136, 5}, {141, 9}, {150, 3}, {153, 0},
		{153, 8}, {161, 2}, {163, 2}, {165, 1},
		{166, 1}, {167, 2}, {169, 5}, {174, 3},
		{177, 6}, {183, 10}, {193, 4}, {197, 2},
		{199, 9}, {208, 13}, {221, 7}, {228, 12},
		{240, 12}, {252, 11}, {263, 2}, {265, 2}
	};

	struct base_ref_type {
		int16_t base;
		int16_t check;
	};

	constexpr static std::size_t base_ref_size = 137;

	constexpr static base_ref_type base_ref[base_ref_size] = {
		{110, 0}, {0, -1}, {53, 54}, {48, 64},
		{54, 46}, {-13, 29}, {55, 126}, {34, 17},
		{29, 23}, {16, 117}, {69, 66}, {44, 60},
		{-44, 29}, {17, 24}, {27, 37}, {19, 27},
		{45, 62}, {1, 61}, {-7, 13}, {62, 117},
		{-8, 15}, {-1, 9}, {60, 40}, {0, 30},
		{4, 49}, {29, 109}, {-9, 13}, {6, 34},
		{-10, 15}, {3, 127}, {1, 111}, {-19, 81},
		{16, 36}, {-2, 8}, {3, 25}, {16, 85},
		{10, 129}, {2, 35}, {66, 32}, {88, 9},
		{14, 129}, {35, 68}, {-31, 81}, {23, 41},
		{-20, 25}, {23, 43}, {0, 45}, {-34, 81},
		{-17, 8}, {0, 11}, {26, 14}, {79, 132},
		{-22, 25}, {65, 129}, {0, 50}, {28, 7},
		{39, 55}, {-24, 2}, {39, 56}, {-21, 11},
		{4, 132}, {0, 2}, {4, 58}, {47, 16},
		{1, 63}, {-26, 3}, {6, 86}, {-23, 11},
		{25, 133}, {-33, 114}, {54, 4}, {-25, 3},
		{-18, 38}, {79, 70}, {-43, 22}, {78, 10},
		{-37, 6}, {-14, 53}, {-40, 19}, {-5, 19},
		{-32, 73}, {25, 22}, {-48, 6}, {-39, 53},
		{-3, 133}, {11, 51}, {43, 114}, {-15, 125},
		{-35, 38}, {88, 75}, {92, 89}, {-38, 73},
		{92, 39}, {-45, 125}, {-46, 51}, {-27, 115},
		{-36, 73}, {-51, 90}, {88, 92}, {98, 98},
		{101, 99}, {-28, 90}, {-49, 115}, {0, -1},
		{-6, 115}, {-16, 115}, {-52, 100}, {-47, 115},
		{0, -1}, {0, 131}, {-29, 100}, {6, 131},
		{0, -1}, {0, -1}, {63, 0}, {83, 0},
		{0, -1}, {1, 0}, {-4, 0}, {0, -1},
		{-30, 0}, {0, -1}, {-50, 0}, {0, -1},
		{-11, 0}, {75, 0}, {2, 0}, {11, 0},
		{-12, 0}, {32, 0}, {-41, 131}, {103, 0},
		{43, 0}, {56, 0}, {0, -1}, {0, -1},
		{-42, 0}
	};
};

constexpr uint8_t sol_socket_option_map::tail[sol_socket_option_map::tail_size];
constexpr sol_socket_option_map::tail_ref_type sol_socket_option_map::tail_ref[sol_socket_option_map::tail_ref_size];
constexpr sol_socket_option_map::base_ref_type sol_socket_option_map::base_ref[sol_socket_option_map::base_ref_size];

#endif
