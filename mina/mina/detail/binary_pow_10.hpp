/*
 * Copyright (c) 2014 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(UCPF_MINA_DETAIL_BINARY_POW_10_20140626T2300)
#define UCPF_MINA_DETAIL_BINARY_POW_10_20140626T2300

namespace ucpf { namespace mina { namespace detail {

struct binary_pow_10 {
	struct entry {
		uint64_t m;
		int exp_2;
		int exp_10;
	};

	constexpr static size_t pow_10_step = 8;
	constexpr static std::array<entry, 87> pow_list;

	template <typename T>
	static entry lookup_exp_10(int exp_10);
};

constexpr std::array<binary_pow_10::entry, 87> binary_pow_10::pow_list = {
	{0xfa8f'd5a0'081c'0288, -1220, -348},
	{0xbaae'e17f'a23e'bf76, -1193, -340},
	{0x8b16'fb20'3055'ac76, -1166, -332},
	{0xcf42'894a'5dce'35ea, -1140, -324},
	{0x9a6b'b0aa'5565'3b2d, -1113, -316},
	{0xe61a'cf03'3d1a'45df, -1087, -308},
	{0xab70'fe17'c79a'c6ca, -1060, -300},
	{0xff77'b1fc'bebc'dc4f, -1034, -292},
	{0xbe56'91ef'416b'd60c, -1007, -284},
	{0x8dd0'1fad'907f'fc3c, -980, -276},
	{0xd351'5c28'3155'9a83, -954, -268},
	{0x9d71'ac8f'ada6'c9b5, -927, -260},
	{0xea9c'2277'23ee'8bcb, -901, -252},
	{0xaecc'4991'4078'536d, -874, -244},
	{0x823c'1279'5db6'ce57, -847, -236},
	{0xc210'9436'4dfb'5637, -821, -228},
	{0x9096'ea6f'3848'984f, -794, -220},
	{0xd774'85cb'2582'3ac7, -768, -212},
	{0xa086'cfcd'97bf'97f4, -741, -204},
	{0xef34'0a98'172a'ace5, -715, -196},
	{0xb238'67fb'2a35'b28e, -688, -188},
	{0x84c8'd4df'd2c6'3f3b, -661, -180},
	{0xc5dd'4427'1ad3'cdba, -635, -172},
	{0x936b'9fce'bb25'c996, -608, -164},
	{0xdbac'6c24'7d62'a584, -582, -156},
	{0xa3ab'6658'0d5f'daf6, -555, -148},
	{0xf3e2'f893'dec3'f126, -529, -140},
	{0xb5b5'ada8'aaff'80b8, -502, -132},
	{0x8762'5f05'6c7c'4a8b, -475, -124},
	{0xc9bc'ff60'34c1'3053, -449, -116},
	{0x964e'858c'91ba'2655, -422, -108},
	{0xdff9'7724'7029'7ebd, -396, -100},
	{0xa6df'bd9f'b8e5'b88f, -369, -92},
	{0xf8a9'5fcf'8874'7d94, -343, -84},
	{0xb944'7093'8fa8'9bcf, -316, -76},
	{0x8a08'f0f8'bf0f'156b, -289, -68},
	{0xcdb0'2555'6531'31b6, -263, -60},
	{0x993f'e2c6'd07b'7fac, -236, -52},
	{0xe45c'10c4'2a2b'3b06, -210, -44},
	{0xaa24'2499'6973'92d3, -183, -36},
	{0xfd87'b5f2'8300'ca0e, -157, -28},
	{0xbce5'0864'9211'1aeb, -130, -20},
	{0x8cbc'cc09'6f50'88cc, -103, -12},
	{0xd1b7'1758'e219'652c, -77, -4},
	{0x9c40'0000'0000'0000, -50, 4},
	{0xe8d4'a510'0000'0000, -24, 12},
	{0xad78'ebc5'ac62'0000, 3, 20},
	{0x813f'3978'f894'0984, 30, 28},
	{0xc097'ce7b'c907'15b3, 56, 36},
	{0x8f7e'32ce'7bea'5c70, 83, 44},
	{0xd5d2'38a4'abe9'8068, 109, 52},
	{0x9f4f'2726'179a'2245, 136, 60},
	{0xed63'a231'd4c4'fb27, 162, 68},
	{0xb0de'6538'8cc8'ada8, 189, 76},
	{0x83c7'088e'1aab'65db, 216, 84},
	{0xc45d'1df9'4271'1d9a, 242, 92},
	{0x924d'692c'a61b'e758, 269, 100},
	{0xda01'ee64'1a70'8dea, 295, 108},
	{0xa26d'a399'9aef'774a, 322, 116},
	{0xf209'787b'b47d'6b85, 348, 124},
	{0xb454'e4a1'79dd'1877, 375, 132},
	{0x865b'8692'5b9b'c5c2, 402, 140},
	{0xc835'53c5'c896'5d3d, 428, 148},
	{0x952a'b45c'fa97'a0b3, 455, 156},
	{0xde46'9fbd'99a0'5fe3, 481, 164},
	{0xa59b'c234'db39'8c25, 508, 172},
	{0xf6c6'9a72'a398'9f5c, 534, 180},
	{0xb7dc'bf53'54e9'bece, 561, 188},
	{0x88fc'f317'f222'41e2, 588, 196},
	{0xcc20'ce9b'd35c'78a5, 614, 204},
	{0x9816'5af3'7b21'53df, 641, 212},
	{0xe2a0'b5dc'971f'303a, 667, 220},
	{0xa8d9'd153'5ce3'b396, 694, 228},
	{0xfb9b'7cd9'a4a7'443c, 720, 236},
	{0xbb76'4c4c'a7a4'4410, 747, 244},
	{0x8bab'8eef'b640'9c1a, 774, 252},
	{0xd01f'ef10'a657'842c, 800, 260},
	{0x9b10'a4e5'e991'3129, 827, 268},
	{0xe710'9bfb'a19c'0c9d, 853, 276},
	{0xac28'20d9'623b'f429, 880, 284},
	{0x8044'4b5e'7aa7'cf85, 907, 292},
	{0xbf21'e440'03ac'dd2d, 933, 300},
	{0x8e67'9c2f'5e44'ff8f, 960, 308},
	{0xd433'179d'9c8c'b841, 986, 316},
	{0x9e19'db92'b4e3'1ba9, 1013, 324},
	{0xeb96'bf6e'badf'77d9, 1039, 332},
	{0xaf87'023b'9bf0'ee6b, 1066, 340}
};

template <>
auto binary_pow_10::lookup_exp_10<double>(int exp_10) -> entry
{
	constexpr double inv_log2_10 = 0.30102999566398114;
	constexpr int bits = 64;

	double k(std::ceil(inv_log2_10 * (exp_10 + bits - 1)));
	auto idx(std::lrint(k));
	idx -= pow_list.front().exp_10 + 1;
	idx /= pow_10_step;
	return pow_list[idx + 1];
}

}}}
#endif
