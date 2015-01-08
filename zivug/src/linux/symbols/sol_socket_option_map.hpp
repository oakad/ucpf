/*
 * Copyright (c) 2014-2015 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_9CA2804E66324A97D000156DD430342F)
#define HPP_9CA2804E66324A97D000156DD430342F

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
	 *   30 peername
	 *   31 timestamp
	 *   32 acceptconn
	 *   33 peersec
	 *   34 passsec
	 *   35 timestampns
	 *   36 mark
	 *   37 timestamping
	 *   38 protocol
	 *   39 domain
	 *   40 rxq_ovfl
	 *   41 wifi_status
	 *   42 peek_off
	 *   43 nofcs
	 *   44 lock_filter
	 *   45 select_err_queue
	 *   46 busy_poll
	 *   47 max_pacing_rate
	 *   48 bpf_extensions
	 */

	template <typename Iterator>
	static int find(Iterator &&first, Iterator &&last)
	{
		auto x_first(first);
		return find(x_first, last);
	}

	template <typename Iterator>
	static int find(Iterator &first, Iterator last)
	{
		int base(base_ref[0].base);
		int l_pos(0), n_pos(0);

		for (; first != last; ++first) {
			if (!is_valid(*first))
				return 0;

			n_pos = base + *first - char_offset;

			if (n_pos >= int(base_ref_size))
				return 0;

			if (int(base_ref[n_pos].check) != l_pos)
				return 0;
			else if (base_ref[n_pos].base < 0) {
				++first;
				int r_idx(-base_ref[n_pos].base);
				auto &r(tail_ref[r_idx - 1]);
				auto r_first(tail + r.offset);
				auto r_last(r_first + r.size);

				while (first != last) {
					if (r_first == r_last)
						break;

					if (*first != *r_first)
						break;

					++first;
					++r_first;
				}

				if ((first == last) && (r_first == r_last))
					return r_idx;
				else
					return 0;
			} else {
				l_pos = n_pos;
				base = base_ref[n_pos].base;
			}
		}

		n_pos = base + term_char - char_offset;
		if (n_pos < int(base_ref_size)) {
			if ((int(base_ref[n_pos].check) == l_pos)
			    && (base_ref[n_pos].base < 0))
				return -base_ref[n_pos].base;
		}

		return 0;
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

	constexpr static std::size_t tail_size = 254;

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
		0x69, 0x63, 0x65, 0x74, 0x61, 0x63, 0x68, 0x5f,
		0x66, 0x69, 0x6c, 0x74, 0x65, 0x72, 0x61, 0x63,
		0x68, 0x5f, 0x66, 0x69, 0x6c, 0x74, 0x65, 0x72,
		0x61, 0x6d, 0x65, 0x63, 0x65, 0x70, 0x74, 0x63,
		0x6f, 0x6e, 0x6e, 0x65, 0x63, 0x65, 0x63, 0x73,
		0x6b, 0x6e, 0x67, 0x74, 0x6f, 0x63, 0x6f, 0x6c,
		0x61, 0x69, 0x6e, 0x71, 0x5f, 0x6f, 0x76, 0x66,
		0x6c, 0x69, 0x66, 0x69, 0x5f, 0x73, 0x74, 0x61,
		0x74, 0x75, 0x73, 0x5f, 0x6f, 0x66, 0x66, 0x63,
		0x73, 0x63, 0x6b, 0x5f, 0x66, 0x69, 0x6c, 0x74,
		0x65, 0x72, 0x65, 0x63, 0x74, 0x5f, 0x65, 0x72,
		0x72, 0x5f, 0x71, 0x75, 0x65, 0x75, 0x65, 0x73,
		0x79, 0x5f, 0x70, 0x6f, 0x6c, 0x6c, 0x5f, 0x70,
		0x61, 0x63, 0x69, 0x6e, 0x67, 0x5f, 0x72, 0x61,
		0x74, 0x65, 0x66, 0x5f, 0x65, 0x78, 0x74, 0x65,
		0x6e, 0x73, 0x69, 0x6f, 0x6e, 0x73
	};

	struct tail_ref_type {
		uint8_t offset;
		uint8_t size;
	};

	constexpr static std::size_t tail_ref_size = 48;

	constexpr static tail_ref_type tail_ref[tail_ref_size] = {
		{0, 2}, {2, 3}, {5, 2}, {7, 4},
		{11, 6}, {17, 7}, {24, 0}, {24, 0},
		{24, 4}, {28, 4}, {32, 8}, {40, 8},
		{48, 5}, {53, 5}, {58, 4}, {62, 7},
		{69, 3}, {72, 3}, {75, 3}, {78, 4},
		{82, 4}, {86, 4}, {90, 4}, {94, 13},
		{107, 8}, {115, 6}, {121, 10}, {131, 11},
		{142, 10}, {152, 3}, {155, 0}, {155, 8},
		{163, 2}, {165, 2}, {167, 1}, {168, 1},
		{169, 2}, {171, 5}, {176, 3}, {179, 6},
		{185, 10}, {195, 4}, {199, 2}, {201, 9},
		{210, 13}, {223, 7}, {230, 12}, {242, 12}
	};

	struct base_ref_type {
		int8_t base;
		int8_t check;
	};

	constexpr static std::size_t base_ref_size = 125;

	constexpr static base_ref_type base_ref[base_ref_size] = {
		{92, 0}, {0, -1}, {53, 54}, {48, 64},
		{62, 46}, {-13, 29}, {96, 108}, {34, 17},
		{29, 23}, {16, 99}, {0, -1}, {44, 60},
		{-43, 29}, {17, 24}, {27, 37}, {19, 27},
		{45, 62}, {1, 61}, {-7, 13}, {53, 99},
		{-8, 15}, {-1, 9}, {70, 40}, {0, 30},
		{4, 49}, {29, 80}, {-9, 13}, {6, 34},
		{-10, 15}, {3, 109}, {1, 82}, {-19, 91},
		{16, 36}, {-2, 8}, {3, 25}, {16, 89},
		{10, 111}, {2, 35}, {71, 32}, {-29, 9},
		{14, 111}, {35, 74}, {-30, 91}, {23, 41},
		{-20, 25}, {23, 43}, {0, 45}, {-33, 91},
		{-17, 8}, {0, 11}, {26, 14}, {83, 114},
		{-22, 25}, {61, 111}, {0, 50}, {28, 7},
		{39, 55}, {-24, 2}, {39, 56}, {-21, 11},
		{4, 114}, {0, 2}, {4, 58}, {47, 16},
		{1, 63}, {-26, 3}, {0, -1}, {-23, 11},
		{-32, 96}, {-39, 19}, {-5, 19}, {-25, 3},
		{-31, 81}, {-14, 53}, {25, 115}, {0, -1},
		{0, -1}, {-18, 38}, {62, 4}, {-38, 53},
		{0, 113}, {71, 78}, {6, 113}, {-37, 81},
		{-42, 22}, {-28, 96}, {-15, 107}, {0, -1},
		{-35, 81}, {11, 51}, {-3, 115}, {25, 22},
		{-44, 107}, {-34, 38}, {0, -1}, {0, -1},
		{62, 0}, {100, 0}, {-45, 51}, {1, 0},
		{-4, 0}, {-40, 113}, {0, -1}, {0, -1},
		{0, -1}, {0, -1}, {-11, 0}, {74, 0},
		{2, 0}, {11, 0}, {-12, 0}, {32, 0},
		{-27, 97}, {74, 0}, {43, 0}, {62, 0},
		{0, -1}, {-36, 6}, {-41, 0}, {-48, 97},
		{0, -1}, {-6, 97}, {-16, 97}, {-47, 6},
		{-46, 97}
	};
};

constexpr uint8_t sol_socket_option_map::tail[sol_socket_option_map::tail_size];
constexpr sol_socket_option_map::tail_ref_type sol_socket_option_map::tail_ref[sol_socket_option_map::tail_ref_size];
constexpr sol_socket_option_map::base_ref_type sol_socket_option_map::base_ref[sol_socket_option_map::base_ref_size];

#endif
