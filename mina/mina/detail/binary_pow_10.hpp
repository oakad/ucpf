/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_MINA_DETAIL_BINARY_POW_10_20140626T2300)
#define UCPF_MINA_DETAIL_BINARY_POW_10_20140626T2300

#include <cmath>
#include <array>

namespace ucpf { namespace mina { namespace detail {

template <typename T>
struct binary_pow_10;

template <>
struct binary_pow_10<double> {
	struct entry {
		uint64_t m;
		int exp_2;
		int exp_10;
	};

	constexpr static size_t pow_10_step = 8;
	constexpr static std::array<entry, 87> pow_5_list = {{
		{0xfa8fd5a0081c0288ull, -1220, -348},
		{0xbaaee17fa23ebf76ull, -1193, -340},
		{0x8b16fb203055ac76ull, -1166, -332},
		{0xcf42894a5dce35eaull, -1140, -324},
		{0x9a6bb0aa55653b2dull, -1113, -316},
		{0xe61acf033d1a45dfull, -1087, -308},
		{0xab70fe17c79ac6caull, -1060, -300},
		{0xff77b1fcbebcdc4full, -1034, -292},
		{0xbe5691ef416bd60cull, -1007, -284},
		{0x8dd01fad907ffc3cull, -980, -276},
		{0xd3515c2831559a83ull, -954, -268},
		{0x9d71ac8fada6c9b5ull, -927, -260},
		{0xea9c227723ee8bcbull, -901, -252},
		{0xaecc49914078536dull, -874, -244},
		{0x823c12795db6ce57ull, -847, -236},
		{0xc21094364dfb5637ull, -821, -228},
		{0x9096ea6f3848984full, -794, -220},
		{0xd77485cb25823ac7ull, -768, -212},
		{0xa086cfcd97bf97f4ull, -741, -204},
		{0xef340a98172aace5ull, -715, -196},
		{0xb23867fb2a35b28eull, -688, -188},
		{0x84c8d4dfd2c63f3bull, -661, -180},
		{0xc5dd44271ad3cdbaull, -635, -172},
		{0x936b9fcebb25c996ull, -608, -164},
		{0xdbac6c247d62a584ull, -582, -156},
		{0xa3ab66580d5fdaf6ull, -555, -148},
		{0xf3e2f893dec3f126ull, -529, -140},
		{0xb5b5ada8aaff80b8ull, -502, -132},
		{0x87625f056c7c4a8bull, -475, -124},
		{0xc9bcff6034c13053ull, -449, -116},
		{0x964e858c91ba2655ull, -422, -108},
		{0xdff9772470297ebdull, -396, -100},
		{0xa6dfbd9fb8e5b88full, -369, -92},
		{0xf8a95fcf88747d94ull, -343, -84},
		{0xb94470938fa89bcfull, -316, -76},
		{0x8a08f0f8bf0f156bull, -289, -68},
		{0xcdb02555653131b6ull, -263, -60},
		{0x993fe2c6d07b7facull, -236, -52},
		{0xe45c10c42a2b3b06ull, -210, -44},
		{0xaa242499697392d3ull, -183, -36},
		{0xfd87b5f28300ca0eull, -157, -28},
		{0xbce5086492111aebull, -130, -20},
		{0x8cbccc096f5088ccull, -103, -12},
		{0xd1b71758e219652cull, -77, -4},
		{0x9c40000000000000ull, -50, 4},
		{0xe8d4a51000000000ull, -24, 12},
		{0xad78ebc5ac620000ull, 3, 20},
		{0x813f3978f8940984ull, 30, 28},
		{0xc097ce7bc90715b3ull, 56, 36},
		{0x8f7e32ce7bea5c70ull, 83, 44},
		{0xd5d238a4abe98068ull, 109, 52},
		{0x9f4f2726179a2245ull, 136, 60},
		{0xed63a231d4c4fb27ull, 162, 68},
		{0xb0de65388cc8ada8ull, 189, 76},
		{0x83c7088e1aab65dbull, 216, 84},
		{0xc45d1df942711d9aull, 242, 92},
		{0x924d692ca61be758ull, 269, 100},
		{0xda01ee641a708deaull, 295, 108},
		{0xa26da3999aef774aull, 322, 116},
		{0xf209787bb47d6b85ull, 348, 124},
		{0xb454e4a179dd1877ull, 375, 132},
		{0x865b86925b9bc5c2ull, 402, 140},
		{0xc83553c5c8965d3dull, 428, 148},
		{0x952ab45cfa97a0b3ull, 455, 156},
		{0xde469fbd99a05fe3ull, 481, 164},
		{0xa59bc234db398c25ull, 508, 172},
		{0xf6c69a72a3989f5cull, 534, 180},
		{0xb7dcbf5354e9beceull, 561, 188},
		{0x88fcf317f22241e2ull, 588, 196},
		{0xcc20ce9bd35c78a5ull, 614, 204},
		{0x98165af37b2153dfull, 641, 212},
		{0xe2a0b5dc971f303aull, 667, 220},
		{0xa8d9d1535ce3b396ull, 694, 228},
		{0xfb9b7cd9a4a7443cull, 720, 236},
		{0xbb764c4ca7a44410ull, 747, 244},
		{0x8bab8eefb6409c1aull, 774, 252},
		{0xd01fef10a657842cull, 800, 260},
		{0x9b10a4e5e9913129ull, 827, 268},
		{0xe7109bfba19c0c9dull, 853, 276},
		{0xac2820d9623bf429ull, 880, 284},
		{0x80444b5e7aa7cf85ull, 907, 292},
		{0xbf21e44003acdd2dull, 933, 300},
		{0x8e679c2f5e44ff8full, 960, 308},
		{0xd433179d9c8cb841ull, 986, 316},
		{0x9e19db92b4e31ba9ull, 1013, 324},
		{0xeb96bf6ebadf77d9ull, 1039, 332},
		{0xaf87023b9bf0ee6bull, 1066, 340}
	}};

	static entry lookup_exp_10(int exp_10)
	{
		constexpr static double inv_log2_10 = 0.30102999566398114;
		constexpr static int bits = 64;

		double k(std::ceil(inv_log2_10 * (exp_10 + bits - 1)));
		auto idx(std::lround(k));
		idx -= pow_5_list.front().exp_10 + 1;
		idx /= pow_10_step;
		return pow_5_list[idx + 1];
	}
};

constexpr std::array<
	binary_pow_10<double>::entry, 87
> binary_pow_10<double>::pow_5_list;

}}}
#endif
