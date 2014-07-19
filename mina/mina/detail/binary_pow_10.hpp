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
struct binary_pow_10 {
	struct u_entry {
		uint64_t m_high;
		uint64_t m_low;
		int exp_2;
		int exp_5;
	};

	struct entry {
		T m;
		int exp_2;
		int exp_5;

		entry()
		: m(0), exp_2(0), exp_5(0)
		{}

		entry(u_entry const &e);
	};

	constexpr static size_t pow_5_step = 8;
	constexpr static std::array<u_entry, 255> pow_5_list = {{
		{0xc68c427b75081ec7ull, 0x5c1624a46fa03e85ull, -3388, -1020},
		{0x93ee0129fbd5676full, 0x9498faf7b82f7609ull, -3361, -1012},
		{0xdc6eb44787c525eeull, 0xe6513a5e8ffae6f7ull, -3335, -1004},
		{0xa43c26a751d4f7e7ull, 0xcf1f49c33399c5acull, -3308, -996},
		{0xf4baaac82ce2253dull, 0x71e49b39e9bcd82cull, -3282, -988},
		{0xb656626d51a9d353ull, 0x384efd538d690c58ull, -3255, -980},
		{0x87da1b483731adc4ull, 0x2f52610febfa41fbull, -3228, -972},
		{0xca6f6acdcad0dcb2ull, 0xd9d4d4e6c9ee0c80ull, -3202, -964},
		{0x96d374651901ae88ull, 0xec9a498a3df7d155ull, -3175, -956},
		{0xe0bf8d0de2b47fccull, 0x03f2c2ebc9696e35ull, -3149, -948},
		{0xa7735373d6162fbdull, 0xd4ba93707d01a265ull, -3122, -940},
		{0xf9854b1f58c854f9ull, 0x7284da122ba226b5ull, -3096, -932},
		{0xb9e84ad5184dcd48ull, 0x94cde1ba3cfca944ull, -3069, -924},
		{0x8a83055e25e23a88ull, 0x59672753ba3f96ccull, -3042, -916},
		{0xce660f0863dafca0ull, 0xeb9a843bd544dfccull, -3016, -908},
		{0x99c76be80626e8ccull, 0x549dd110d4c13757ull, -2989, -900},
		{0xe5260784c0e4cc3eull, 0xf3e99f4780cb7f1eull, -2963, -892},
		{0xaaba9e26cd980fb9ull, 0x50ca62453e028de2ull, -2936, -884},
		{0xfe67ef8889b56214ull, 0x30096751b6502ddeull, -2910, -876},
		{0xbd8c17e83c6ad135ull, 0xaebcc797b23b9bb7ull, -2883, -868},
		{0x8d3944516e60ad44ull, 0xd543249cc2a2179full, -2856, -860},
		{0xd27090d38885fac8ull, 0xd4fa1a47404da9cbull, -2830, -852},
		{0x9cca30756cc62395ull, 0xf8e721cb7155c517ull, -2803, -844},
		{0xe9a29018060b3b08ull, 0x6aa227dd677ca2deull, -2777, -836},
		{0xae125787f823c6d1ull, 0x00b3b5ffde622d58ull, -2750, -828},
		{0x81b188317cf5c6d9ull, 0x98dc052405b4298cull, -2723, -820},
		{0xc1422355e038bb64ull, 0x8035810006a8cfb7ull, -2697, -812},
		{0x8ffd1af3f3e580c5ull, 0x4c626d0f71162ef7ull, -2670, -804},
		{0xd68f53c0fb13e133ull, 0x5f5b9d8bb5cee291ull, -2644, -796},
		{0x9fdc0c3ca689431full, 0xfea67220d73a86d1ull, -2617, -788},
		{0xee359553035782e0ull, 0x65c51589fc3f33e2ull, -2591, -780},
		{0xb17ad1f3f81bc7d1ull, 0x98af0f70dd3d6673ull, -2564, -772},
		{0x843b94349b8d51b7ull, 0xc638411afd0426c8ull, -2537, -764},
		{0xc50ac88ea93763c0ull, 0x249494d1bf7c86edull, -2511, -756},
		{0x92cecd66836d35dcull, 0xc8b9282388c14385ull, -2484, -748},
		{0xdac2bd558d00c536ull, 0x23b0d9b5fd389e47ull, -2458, -740},
		{0xa2fd4ae0e124d08full, 0x1ae63beb0eccbab4ull, -2431, -732},
		{0xf2df87eb1b358befull, 0xf24109d27a306588ull, -2405, -724},
		{0xb4f461643fd71c95ull, 0x8b802b558e7bf62cull, -2378, -716},
		{0x86d25a5e792d112aull, 0xdeee43b957ff558cull, -2351, -708},
		{0xc8e664cd8d387df8ull, 0x1e2bd23627c69801ull, -2325, -700},
		{0x95aea11f625d54a1ull, 0xb859b1f80b915fa2ull, -2298, -692},
		{0xdf0b3512e462db4bull, 0xb054560001faca5dull, -2272, -684},
		{0xa62e3980660373e7ull, 0x93dd8bcdb439969dull, -2245, -676},
		{0xf7a0daca9a630e9eull, 0xd897ede210999d35ull, -2219, -668},
		{0xb87f5b7726b838e5ull, 0x033bad337812758full, -2192, -660},
		{0x89761a7985ac0e5aull, 0xc7b14ac6da904fd1ull, -2165, -652},
		{0xccd55720cb861b6eull, 0xd95729515330f114ull, -2139, -644},
		{0x989cdcf0fe071849ull, 0x2e2427d798a19545ull, -2112, -636},
		{0xe36924817243e1faull, 0x120bbbc7952ded0cull, -2086, -628},
		{0xa96f26bc066e79daull, 0x0949383d7ec2964full, -2059, -620},
		{0xfc7a031bc76470beull, 0x3c7cd558308a649bull, -2033, -612},
		{0xbc1c177826c6725cull, 0x8ab0d993bb75625eull, -2006, -604},
		{0x8c27158febf2c775ull, 0x7930acb188d140a4ull, -1979, -596},
		{0xd0d800731302e7a4ull, 0x064b9e215703f180ull, -1953, -588},
		{0x9b99c910bcac806eull, 0x36153fae52d4ff02ull, -1926, -580},
		{0xe7dcf73a9ae95b2bull, 0x5a4287419ae96814ull, -1900, -572},
		{0xacc062beacea4866ull, 0x4c391486560f3204ull, -1873, -564},
		{0x80b5bc2a14b4b366ull, 0x60bf20ade9174509ull, -1846, -556},
		{0xbfcaee68439528e0ull, 0xa2d36569ed985eeeull, -1820, -548},
		{0x8ee58df1d4875e53ull, 0x62ec8cdd84ced00dull, -1793, -540},
		{0xd4eec394d6258bf8ull, 0x28e54542d9b56dc9ull, -1767, -532},
		{0x9ea5af1df0afbc39ull, 0xaa1736e32354e3eeull, -1740, -524},
		{0xec671af31105d9adull, 0xfc3ab21793c6afdbull, -1714, -516},
		{0xb0223f45148139fbull, 0xb82e8acf9f40eee5ull, -1687, -508},
		{0x833ada2003db9a7full, 0x8ecd4439da0518e7ull, -1660, -500},
		{0xc38c3b069b47a4d7ull, 0x29f2527565a63ef3ull, -1634, -492},
		{0x91b1c73bc77a085dull, 0xb3bf8c65f9f6b584ull, -1607, -484},
		{0xd91a0545cdb51185ull, 0xe287c2ad77ead647ull, -1581, -476},
		{0xa1c0da28ee982db0ull, 0xdea3da80aeba0a7dull, -1554, -468},
		{0xf107ff8564d28efbull, 0xb1ae70a8e6fcf47eull, -1528, -460},
		{0xb3950fa5a6bafd5aull, 0x209569ede58e7c18ull, -1501, -452},
		{0x85cc99871159ee32ull, 0x520a166b0852116full, -1474, -444},
		{0xc76059d92284bdffull, 0x073c0c9a5328cc80ull, -1448, -436},
		{0x948c065d2e4f02cfull, 0x6667c25b4ca0a421ull, -1421, -428},
		{0xdd5a2c3eab3097cbull, 0xbd54467eec6dd2bbull, -1395, -420},
		{0xa4eb96ba469b9dbeull, 0xc31d0de0817f009bull, -1368, -412},
		{0xf5c016fcc815445dull, 0xc13d640ce56ed7cfull, -1342, -404},
		{0xb71928d88102515eull, 0x72d1317eb8425e41ull, -1315, -396},
		{0x886b39add3d98638ull, 0x24bf62b68c0069b9ull, -1288, -388},
		{0xcb47a9358c317faeull, 0xda9018cfcc9be08aull, -1262, -380},
		{0x9774919ef68662a3ull, 0xba954f8e758fecb4ull, -1235, -372},
		{0xe1afa13afbd14d6dull, 0x82189c09a3a1ec22ull, -1209, -364},
		{0xa82632da225da4a6ull, 0x4ca77e24d2078c9full, -1182, -356},
		{0xfa8fd5a0081c0288ull, 0x1732c869cd60e453ull, -1156, -348},
		{0xbaaee17fa23ebf76ull, 0x5d79bcf00d2df649ull, -1129, -340},
		{0x8b16fb203055ac76ull, 0x4c3bcb5021afcc31ull, -1102, -332},
		{0xcf42894a5dce35eaull, 0x52064cac828675b9ull, -1076, -324},
		{0x9a6bb0aa55653b2dull, 0x47b233c92125366eull, -1049, -316},
		{0xe61acf033d1a45dfull, 0x6fb92487298e33beull, -1023, -308},
		{0xab70fe17c79ac6caull, 0x6dbd630a48aaf407ull, -996, -300},
		{0xff77b1fcbebcdc4full, 0x25e8e89c13bb0f7bull, -970, -292},
		{0xbe5691ef416bd60cull, 0x23cc986bc656d553ull, -943, -284},
		{0x8dd01fad907ffc3bull, 0xae3da7d97f6792e3ull, -916, -276},
		{0xd3515c2831559a83ull, 0x0d5a5b44ca873e04ull, -890, -268},
		{0x9d71ac8fada6c9b5ull, 0x6f773fc3603db4a9ull, -863, -260},
		{0xea9c227723ee8bcbull, 0x465e15a979c1caddull, -837, -252},
		{0xaecc49914078536dull, 0x58fae9f773886e18ull, -810, -244},
		{0x823c12795db6ce57ull, 0x76c53d08d6b70859ull, -783, -236},
		{0xc21094364dfb5636ull, 0x985915fc12f542e4ull, -757, -228},
		{0x9096ea6f3848984full, 0x3ff0d2c85def7621ull, -730, -220},
		{0xd77485cb25823ac7ull, 0x7d633293366b828bull, -704, -212},
		{0xa086cfcd97bf97f3ull, 0x80e8a40eccd228a4ull, -677, -204},
		{0xef340a98172aace4ull, 0x86fb897116c87c35ull, -651, -196},
		{0xb23867fb2a35b28dull, 0xe99e619a4f23aa43ull, -624, -188},
		{0x84c8d4dfd2c63f3bull, 0x29ecd9f40041e073ull, -597, -180},
		{0xc5dd44271ad3cdbaull, 0x40eff1e1853f29fdull, -571, -172},
		{0x936b9fcebb25c995ull, 0xcab10dd900beec34ull, -544, -164},
		{0xdbac6c247d62a583ull, 0xdf45f746b74abf3aull, -518, -156},
		{0xa3ab66580d5fdaf5ull, 0xc13e60d0d2e0ebbaull, -491, -148},
		{0xf3e2f893dec3f126ull, 0x5a89dba3c3efccfbull, -465, -140},
		{0xb5b5ada8aaff80b8ull, 0x0d819992132456bbull, -438, -132},
		{0x87625f056c7c4a8bull, 0x11471cd764ad4973ull, -411, -124},
		{0xc9bcff6034c13052ull, 0xfc89b393dd02f0b5ull, -385, -116},
		{0x964e858c91ba2655ull, 0x3a6a07f8d510f870ull, -358, -108},
		{0xdff9772470297ebdull, 0x59787e2b93bc56f7ull, -332, -100},
		{0xa6dfbd9fb8e5b88eull, 0xcb4ccd500f6bb953ull, -305, -92},
		{0xf8a95fcf88747d94ull, 0x75a44c6397ce912aull, -279, -84},
		{0xb94470938fa89bceull, 0xf808e40e8d5b3e6aull, -252, -76},
		{0x8a08f0f8bf0f156bull, 0x1b8e9ecb641b5900ull, -225, -68},
		{0xcdb02555653131b6ull, 0x3792f412cb06794dull, -199, -60},
		{0x993fe2c6d07b7fabull, 0xe546a8038efe4029ull, -172, -52},
		{0xe45c10c42a2b3b05ull, 0x8cb89a7db77c506aull, -146, -44},
		{0xaa242499697392d2ull, 0xdde50bd1d5d0b9eaull, -119, -36},
		{0xfd87b5f28300ca0dull, 0x8bca9d6e188853fcull, -93, -28},
		{0xbce5086492111aeaull, 0x88f4bb1ca6bcf585ull, -66, -20},
		{0x8cbccc096f5088cbull, 0xf93f87b7442e45d3ull, -39, -12},
		{0xd1b71758e219652bull, 0xd3c36113404ea4a8ull, -13, -4},
		{0x9c40000000000000ull, 0x0000000000000000ull, 14, 4},
		{0xe8d4a51000000000ull, 0x0000000000000000ull, 40, 12},
		{0xad78ebc5ac620000ull, 0x0000000000000000ull, 67, 20},
		{0x813f3978f8940984ull, 0x4000000000000000ull, 94, 28},
		{0xc097ce7bc90715b3ull, 0x4b9f100000000000ull, 120, 36},
		{0x8f7e32ce7bea5c6full, 0xe4820023a2000000ull, 147, 44},
		{0xd5d238a4abe98068ull, 0x72a4904598d6d880ull, 173, 52},
		{0x9f4f2726179a2245ull, 0x01d762422c946591ull, 200, 60},
		{0xed63a231d4c4fb27ull, 0x4ca7aaa863ee4bdeull, 226, 68},
		{0xb0de65388cc8ada8ull, 0x3b25a55f43294bccull, 253, 76},
		{0x83c7088e1aab65dbull, 0x792667c6da79e0fbull, 280, 84},
		{0xc45d1df942711d9aull, 0x3ba5d0bd324f8395ull, 306, 92},
		{0x924d692ca61be758ull, 0x593c2626705f9c56ull, 333, 100},
		{0xda01ee641a708de9ull, 0xe80e6f4820cc9495ull, 359, 108},
		{0xa26da3999aef7749ull, 0xe3be5e330f38f09dull, 386, 116},
		{0xf209787bb47d6b84ull, 0xc0678c5dbd23a49aull, 412, 124},
		{0xb454e4a179dd1877ull, 0x29babe4598c311fbull, 439, 132},
		{0x865b86925b9bc5c2ull, 0x0b8a2392ba45a9b3ull, 466, 140},
		{0xc83553c5c8965d3dull, 0x6f92829494e5acc8ull, 492, 148},
		{0x952ab45cfa97a0b2ull, 0xdd945a747bf26184ull, 519, 156},
		{0xde469fbd99a05fe3ull, 0x6fca5f8ed9aef3bbull, 545, 164},
		{0xa59bc234db398c25ull, 0x43fab9837e699095ull, 572, 172},
		{0xf6c69a72a3989f5bull, 0x8aad549e57273d46ull, 598, 180},
		{0xb7dcbf5354e9beceull, 0x0c11ed6d538aeb2full, 625, 188},
		{0x88fcf317f22241e2ull, 0x441fece3bdf81f03ull, 652, 196},
		{0xcc20ce9bd35c78a5ull, 0x31ec038df7b441f4ull, 678, 204},
		{0x98165af37b2153deull, 0xc3727a337a8b704bull, 705, 212},
		{0xe2a0b5dc971f303aull, 0x2e44ae64840fd61dull, 731, 220},
		{0xa8d9d1535ce3b396ull, 0x7f1839a741a14d0dull, 758, 228},
		{0xfb9b7cd9a4a7443cull, 0x169840ef017da3b1ull, 784, 236},
		{0xbb764c4ca7a4440full, 0x9d6d1ad41abe37f1ull, 811, 244},
		{0x8bab8eefb6409c1aull, 0x1ad089b6c2f7548eull, 838, 252},
		{0xd01fef10a657842cull, 0x2d2b7569b0432d85ull, 864, 260},
		{0x9b10a4e5e9913128ull, 0xca7cf2b4191c8327ull, 891, 268},
		{0xe7109bfba19c0c9dull, 0x0cc512670a783ad5ull, 917, 276},
		{0xac2820d9623bf429ull, 0x546345fa9fbdcd45ull, 944, 284},
		{0x80444b5e7aa7cf85ull, 0x7980d163cf5b81b4ull, 971, 292},
		{0xbf21e44003acdd2cull, 0xe0470a63e6bd56c3ull, 997, 300},
		{0x8e679c2f5e44ff8full, 0x570f09eaa7ea7649ull, 1024, 308},
		{0xd433179d9c8cb841ull, 0x5fa60692a46151ecull, 1050, 316},
		{0x9e19db92b4e31ba9ull, 0x6c07a2c26a8346d2ull, 1077, 324},
		{0xeb96bf6ebadf77d8ull, 0xe41c5bd18c57e890ull, 1103, 332},
		{0xaf87023b9bf0ee6aull, 0xeb8fad7c7f8680b4ull, 1130, 340},
		{0x82c730bec1cac960ull, 0x8f26fdb7c3c30a3dull, 1157, 348},
		{0xc2dfe19c8c055535ull, 0xcca845ab2beafa9bull, 1183, 356},
		{0x91315e37db165aa9ull, 0x2c0de8dd3d020c0dull, 1210, 364},
		{0xd85aaca9a2ccee2dull, 0xdc87a519e5b7f425ull, 1236, 372},
		{0xa13249c808fcdb9eull, 0x91205bb76c267701ull, 1263, 380},
		{0xf0338faddd968ba1ull, 0x2738d3f310ddcc2eull, 1289, 388},
		{0xb2f6c887001a31e2ull, 0xf6e91ff127836288ull, 1316, 396},
		{0x8556ac6e4e486446ull, 0x5c2ab7c5946aac8eull, 1343, 404},
		{0xc6b0a096a95202bdull, 0xd1385e91e071b9eaull, 1369, 412},
		{0x940919bbd4620b6dull, 0x250535bcc387778full, 1396, 420},
		{0xdc971492e1906e0full, 0x4e1d64ac6ceb7552ull, 1422, 428},
		{0xa45a3bcb01d933f2ull, 0x3cf91811940c4b99ull, 1449, 436},
		{0xf4e77e5fb7dbe096ull, 0x2b47dcd3b798174aull, 1475, 444},
		{0xb677c868c88c2d23ull, 0xde79e41449582beeull, 1502, 452},
		{0x87f2fd83ead86983ull, 0x4b1a5499c9d7d686ull, 1529, 460},
		{0xca947f30e9bb252full, 0xf6b237071ea349e7ull, 1555, 468},
		{0x96ef14c6454aa840ull, 0x4cf76e8df8d89498ull, 1582, 476},
		{0xe0e8b7b8974228b9ull, 0x3ac066c2f98a78e3ull, 1608, 484},
		{0xa791ff5f02c941b7ull, 0xa6b918e8393c669aull, 1635, 492},
		{0xf9b2ff649b8695d7ull, 0x71a58a839043c752ull, 1661, 500},
		{0xba0a5836993cad7cull, 0xc34b40940f00b1beull, 1688, 508},
		{0x8a9c645298da6479ull, 0x9640dd9ab7f00a4eull, 1715, 516},
		{0xce8bdd450d49541eull, 0xa6bb5dc75bad25d7ull, 1741, 524},
		{0x99e396c13a3acff1ull, 0xb0c5560a402ac0b3ull, 1768, 532},
		{0xe550008523ed219aull, 0xe15cf9beebd044dfull, 1794, 540},
		{0xaad9e3cd32810593ull, 0x4533ddceba719d8bull, 1821, 548},
		{0xfe9688e1a54cfe67ull, 0x0c6b72b99bd62b54ull, 1847, 556},
		{0xbdaecff6cf80be04ull, 0xeeb1bf9b704e98ebull, 1874, 564},
		{0x8d53226fc140c983ull, 0x4c87d4944b743d40ull, 1901, 572},
		{0xd2971c8d3f9bf297ull, 0x10d2da2e4ef47083ull, 1927, 580},
		{0x9ce6e87cb0821c85ull, 0xc3bfbae0f3e130e3ull, 1954, 588},
		{0xe9cd5b7847438d90ull, 0xfa298bf05320b449ull, 1980, 596},
		{0xae3239ec1ca78196ull, 0x01d4aba453ad5c36ull, 2007, 604},
		{0x81c949a60de6e0c5ull, 0x6fb7efcc983ca835ull, 2034, 612},
		{0xc1658968faf0a438ull, 0xec027a09e26cc8d5ull, 2060, 620},
		{0x90177ab9848401f8ull, 0xed7e45e544904140ull, 2087, 628},
		{0xd6b6a0ad7fdcf5f1ull, 0x66a27c968bfc25deull, 2113, 636},
		{0x9ff95435986594c9ull, 0x6632249f8a06c2c7ull, 2140, 644},
		{0xee61373191fc0426ull, 0xe0b2e18f245ca58full, 2166, 652},
		{0xb19b542779a67267ull, 0x67d6a2dc3e4d5b75ull, 2193, 660},
		{0x8453ccba83cbce59ull, 0x914e7b79e456b27aull, 2220, 668},
		{0xc52ee00e80c2ba96ull, 0xfad44ee1e8d68959ull, 2246, 676},
		{0x92e9b15d2a38e595ull, 0x6c5ca5d022927498ull, 2273, 684},
		{0xdaeacf3d37d9c2e9ull, 0x39b6094ba67834d1ull, 2299, 692},
		{0xa31b259cfa50498full, 0x7478a3cbba44ec48ull, 2326, 700},
		{0xf30c047b0b8611b1ull, 0x0dfa017124aad287ull, 2352, 708},
		{0xb51586881d795907ull, 0x289bf26a487bda48ull, 2379, 716},
		{0x86eb0c4a82a6b8dfull, 0xdc86758466267005ull, 2406, 724},
		{0xc90b31336a71fff5ull, 0x825b730ebf783047ull, 2432, 732},
		{0x95ca0bddb0e934feull, 0x6f0a4ad8df51a807ull, 2459, 740},
		{0xdf340fd1032d6d69ull, 0xcfc5f9fa6cbb6d2dull, 2485, 748},
		{0xa64ca9df3fd42cf6ull, 0x8f96bee42fda4244ull, 2512, 756},
		{0xf7ce36540d1c8255ull, 0x12844ca8c5e74ee5ull, 2538, 764},
		{0xb8a126bc0e3f4b4eull, 0x6a826c5e18681b29ull, 2565, 772},
		{0x898f482c2984f227ull, 0x6a0cd185b8c2f3a9ull, 2592, 780},
		{0xccfadbf760895d17ull, 0x67d8a4596f0a2e36ull, 2618, 788},
		{0x98b8d11a7ecfc408ull, 0xe7e5fa0e26a247a6ull, 2645, 796},
		{0xe392cc04a76d47d9ull, 0x165032edc4a52a5dull, 2671, 804},
		{0xa98e2faba12ea481ull, 0x8af70b7be4ecb750ull, 2698, 812},
		{0xfca841fc5340e0fdull, 0x91ab32a288a69505ull, 2724, 820},
		{0xbc3e8c1ec356d509ull, 0x8873bb83371c4e04ull, 2751, 828},
		{0x8c40c1758d15be4cull, 0x60e74b378eb3e193ull, 2778, 836},
		{0xd0fe4156d270e1aeull, 0x4f98bf96c43a7eb8ull, 2804, 844},
		{0x9bb64956341d2cb3ull, 0x374b9c51b04e5d89ull, 2831, 852},
		{0xe8076f853e4d663aull, 0xf87a40e9c0c3b204ull, 2857, 860},
		{0xace0073bb807da80ull, 0x8480950470d805edull, 2884, 868},
		{0x80cd4f7fa35870f0ull, 0xd408cd96cda3c126ull, 2911, 876},
		{0xbfee0fc18dcf1a61ull, 0x78b0b31c0222cea0ull, 2937, 884},
		{0x8effba82fb5c0b65ull, 0x9a73f5c8688ae72full, 2964, 892},
		{0xd515c4344c1e8ef2ull, 0x915861c376f9d550ull, 2990, 900},
		{0x9ec2be3d9f6d1e0cull, 0xd3e0a7a1ea113aa9ull, 3017, 908},
		{0xec92681b92b788baull, 0x74dbdfc2b729d6e6ull, 3043, 916},
		{0xb042825b38276899ull, 0xbcc0502652e7e71dull, 3070, 924},
		{0x8352e39fbef5e397ull, 0xd810904f57b1ca32ull, 3097, 932},
		{0xc3b00c742afd66b9ull, 0x24a3e48ddec88b32ull, 3123, 940},
		{0x91cc76fd5ec629b4ull, 0x85ffb091a01a2e6eull, 3150, 948},
		{0xd941c961fa90b0a6ull, 0xc2797b236bae6b66ull, 3176, 956},
		{0xa1de7aeed517a4afull, 0x94e1d2e9e752f543ull, 3203, 964},
		{0xf13425b6b1d3c874ull, 0x6c0dc5f96fa55fbbull, 3229, 972},
		{0xb3b5f46fcedc9c88ull, 0x16c0208e3cc9e873ull, 3256, 980},
		{0x85e51b813c31a00bull, 0x886450197da44666ull, 3283, 988},
		{0xc784decd820a6180ull, 0x97681c548ff6c130ull, 3309, 996},
		{0x94a73be0c148e43eull, 0x56fe8868947d6e96ull, 3336, 1004},
		{0xdd82b7ab5ffc462dull, 0xfdc12e13381e6d80ull, 3362, 1012}
	}};

	static auto lookup_exp_10(int exp_10) -> entry
	{
		constexpr static double inv_log2_10 = 0.30102999566398114;
		constexpr static int bits = sizeof(T) * 8;

		double k(std::ceil(inv_log2_10 * (exp_10 + bits - 1)));
		auto idx(std::lround(k));
		idx -= pow_5_list.front().exp_5 + 1;
		idx /= pow_5_step;

		if ((idx < 0) || (idx > (pow_5_list.size() - 2)))
			return entry();
		else
			return entry(pow_5_list[idx + 1]);
	}
};

template <>
binary_pow_10<uint32_t>::entry::entry(
	binary_pow_10<uint32_t>::u_entry const &e
) : m(e.m_high >> 32), exp_2(e.exp_2 - 32), exp_5(e.exp_5)
{
	constexpr static uint64_t round_mask = uint64_t(1) << 31;

	if (e.m_high & round_mask)
		m += 1;
}

template <>
binary_pow_10<uint64_t>::entry::entry(
	binary_pow_10<uint64_t>::u_entry const &e
) : m(e.m_high), exp_2(e.exp_2 - 64), exp_5(e.exp_5)
{
	constexpr static uint64_t round_mask = uint64_t(1) << 63;

	if (e.m_low & round_mask)
		m += 1;
}

template <>
binary_pow_10<uint128_t>::entry::entry(
	binary_pow_10<uint128_t>::u_entry const &e
) : m(e.m_high), exp_2(e.exp_2 - 128), exp_5(e.exp_5)
{
	m <<= 64u;
	m |= e.m_low;
}

template <typename T>
constexpr std::array<
	typename binary_pow_10<T>::u_entry, 255
> binary_pow_10<T>::pow_5_list;

}}}
#endif
