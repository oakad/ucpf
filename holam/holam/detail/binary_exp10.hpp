/*
 * Copyright (c) 2016 Alex Dubov <oakad@yahoo.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the  terms of  the GNU General Public License version 3 as publi-
 * shed by the Free Software Foundation.
 */

#if !defined(HPP_0D1745BD005AD4E81F25CC47C7DFB471)
#define HPP_0D1745BD005AD4E81F25CC47C7DFB471

#include <cmath>
#include <cstdio>

namespace ucpf { namespace holam { namespace detail {

template <typename Dummy = void>
struct binary_exp10 {
	struct u_entry {
		uint64_t m_high;
		uint64_t m_low;
		int32_t exp2;
		int32_t exp10;
	};

	constexpr static u_entry exp10_list[458] = {
		{0x9e2477054349c65aull, 0x4cf443523e69f6e2ull, -16496, -4966}, //0
		{0x82cff6e920effac8ull, 0xfb7d2c0d7a1729baull, -16416, -4942}, //1
		{0xd86930a1bbd65a5full, 0x5502304aa2cee7a1ull, -16337, -4918}, //2
		{0xb302ca4e1264ea1eull, 0xe0536cbb30700ae8ull, -16257, -4894}, //3
		{0x9413084d8f348794ull, 0x925af3e6cef5dd98ull, -16177, -4870}, //4
		{0xf4f7ecb3af1096dcull, 0xdfdbb75ac1af003full, -16098, -4846}, //5
		{0xcaa216958305b72dull, 0x3cddb9a62b4d5e97ull, -16018, -4822}, //6
		{0xa79d3d75215032eeull, 0x1f64ed863c32fc5cull, -15938, -4798}, //7
		{0x8aa5b10519680a7aull, 0x3fde7cf6e31b2e77ull, -15858, -4774}, //8
		{0xe55f630da79571d4ull, 0x88cf7ab7324ed27aull, -15779, -4750}, //9
		{0xbdbb89d7ab4a3e3aull, 0x9f908548b18fd0d7ull, -15699, -4726}, //10
		{0x9cf16f5743f239f3ull, 0x8f18ab6c9459c4b3ull, -15619, -4702}, //11
		{0x81d1fec78bca02a8ull, 0x577fe73a39721d9aull, -15539, -4678}, //12
		{0xd6c50877206aaf59ull, 0x03c7047f8a0d30f9ull, -15460, -4654}, //13
		{0xb1a73e9ee05c3de0ull, 0x445a546febc0a1c3ull, -15380, -4630}, //14
		{0x92f38ca68db75fdeull, 0xc34bd386c5b97b21ull, -15300, -4606}, //15
		{0xf31c52e884f187d9ull, 0x7a0e69e43f26eddcull, -15221, -4582}, //16
		{0xc918ae34d9ed4b50ull, 0x816f688b13d27b5eull, -15141, -4558}, //17
		{0xa657d221a0e1d18dull, 0xbbbb17c7df72890aull, -15061, -4534}, //18
		{0x899882d09813113dull, 0x76711575854bf960ull, -14981, -4510}, //19
		{0xe3a210ae982af50aull, 0xb3daf8ba949f6e08ull, -14902, -4486}, //20
		{0xbc4b2d4a861f008dull, 0xfe25ff437eaa975dull, -14822, -4462}, //21
		{0x9bc0bbc0c8052f2cull, 0x3e7013bec05ffc22ull, -14742, -4438}, //22
		{0x80d5f3b952866d08ull, 0x5e14dd946fc3d819ull, -14662, -4414}, //23
		{0xd52410062aec2bc3ull, 0xf4545eaaa21c1fa0ull, -14583, -4390}, //24
		{0xb04e55b035614fbcull, 0x8d8764ed156eddfcull, -14503, -4366}, //25
		{0x91d63f23dabc4592ull, 0x0fcfada4546956cdull, -14423, -4342}, //26
		{0xf144547b9c8540a1ull, 0x7d873e01ba37ae2bull, -14344, -4318}, //27
		{0xc792419f02b5dd8aull, 0xc62e77160357a258ull, -14264, -4294}, //28
		{0xa514de996e1a52ddull, 0x12dbd38cd31dafd6ull, -14184, -4270}, //29
		{0x888d5f37b4704dd8ull, 0xd9c5df7b2ebc34f6ull, -14104, -4246}, //30
		{0xe1e81ee494186984ull, 0x6f4a6a30fdd6111eull, -14025, -4222}, //31
		{0xbadd9be7c829fb9bull, 0x97eadc0bd4130909ull, -13945, -4198}, //32
		{0x9a9257bc82fb3dbcull, 0xd1941bd2d84c5e14ull, -13865, -4174}, //33
		{0xffb7a402531fd4c9ull, 0x952b06c385a08ff7ull, -13786, -4150}, //34
		{0xd386411f250c27abull, 0xd44dc66f7ff4d3ccull, -13706, -4126}, //35
		{0xaef80a640d5d1c17ull, 0x52799b519dadc8f7ull, -13626, -4102}, //36
		{0x90bb1b89d796da95ull, 0x6d5949a050073edcull, -13546, -4078}, //37
		{0xef6fea6c42f3b6a6ull, 0x6b1333194a0243b2ull, -13467, -4054}, //38
		{0xc60ecb091aad1a68ull, 0xd6c0c627bc43d380ull, -13387, -4030}, //39
		{0xa3d45e11ebbfdd22ull, 0xabe84d7fea1a94fbull, -13307, -4006}, //40
		{0x87844243ccd8ae8cull, 0x064ac20e02273d7full, -13227, -3982}, //41
		{0xe03187210a3af969ull, 0xdb7f1ee9fede2173ull, -13148, -3958}, //42
		{0xb972d042f70b2775ull, 0x499e38edabb6dc71ull, -13068, -3934}, //43
		{0x99663ecdef2bafcbull, 0xea88081af806b8c5ull, -12988, -3910}, //44
		{0xfdc72bd20fc4c23bull, 0xd089211452d54446ull, -12909, -3886}, //45
		{0xd1eb959e5b39d1ceull, 0x279e292950374371ull, -12829, -3862}, //46
		{0xada457a653967923ull, 0x47ee12f0ae682e4full, -12749, -3838}, //47
		{0x8fa21da51d6d94baull, 0x0d98d4689ce127ebull, -12669, -3814}, //48
		{0xed9f0dc75de0cd44ull, 0x8a09f5043b279b1dull, -12590, -3790}, //49
		{0xc48e44b37e1ddb8aull, 0xf44da6ef525ce9c9ull, -12510, -3766}, //50
		{0xa2964bc9ca0a3762ull, 0xa4b7e3ec4abb01edull, -12430, -3742}, //51
		{0x867d2805f1877bddull, 0x6b0e749eba1629caull, -12350, -3718}, //52
		{0xde7e42e2245636b9ull, 0x0484bf2463938076ull, -12271, -3694}, //53
		{0xb80ac4fa2015685bull, 0xd474f2c5cbee017full, -12191, -3670}, //54
		{0x983c6c813cc24293ull, 0x4bf11f2e8da28f9dull, -12111, -3646}, //55
		{0xfbda778431e51f45ull, 0x59b77223722acf97ull, -12032, -3622}, //56
		{0xd054076c0550a4edull, 0x8e7a93c68ffa3e50ull, -11952, -3598}, //57
		{0xac53386ccf683342ull, 0x5ca6e44dee8d33ceull, -11872, -3574}, //58
		{0x8e8b414a6d453707ull, 0xb6687e92458e9847ull, -11792, -3550}, //59
		{0xebd1b7a751070da3ull, 0x3d752735cd6f03dcull, -11713, -3526}, //60
		{0xc310a8e9b27aa994ull, 0x12712f4b032d52a0ull, -11633, -3502}, //61
		{0xa15aa308f493444eull, 0x30cf4ac992f51f37ull, -11553, -3478}, //62
		{0x85780c96d5a9dc34ull, 0x9ee976611dd06363ull, -11473, -3454}, //63
		{0xdcce4bb2ae5d067aull, 0xe32b3d65beb63514ull, -11394, -3430}, //64
		{0xb6a574b5c3dceb2eull, 0x85dfe4292d6de3a3ull, -11314, -3406}, //65
		{0x9714dc6b40d5fc39ull, 0x6cecd363eb888d1cull, -11234, -3382}, //66
		{0xf9f17fc95d621855ull, 0xb757828a0a9cb679ull, -11155, -3358}, //67
		{0xcebf907c2f7422e0ull, 0x39c5f327fd4d2fb3ull, -11075, -3334}, //68
		{0xab04a7b7111c9c32ull, 0xdf523a199991b30full, -10995, -3310}, //69
		{0x8d768256a02b450dull, 0x29b4d668a48f2ea9ull, -10915, -3286}, //70
		{0xea07e133e4059db1ull, 0xb481b321fbd2900full, -10836, -3262}, //71
		{0xc195f20250b2a1f8ull, 0xd7f8893bd21a568cull, -10756, -3238}, //72
		{0xa0215f20806a8da8ull, 0xd10f9babe55028deull, -10676, -3214}, //73
		{0x8474ec16c08b589cull, 0x052c5352353864eaull, -10596, -3190}, //74
		{0xdb219b29fdea87dbull, 0x37e5a22177d186b9ull, -10517, -3166}, //75
		{0xb542da28c1ed32d8ull, 0xf10c71059bef1855ull, -10437, -3142}, //76
		{0x95ef8a2964a0d6a4ull, 0xe5c7e7a6ad9c4862ull, -10357, -3118}, //77
		{0xf80c3d606750d9c0ull, 0x19a0bd285ce82f69ull, -10278, -3094}, //78
		{0xcd2e2acea3187d74ull, 0x59e71758b4aadff8ull, -10198, -3070}, //79
		{0xa9b8a08e5eee4455ull, 0x1f6648b0c8ce729dull, -10118, -3046}, //80
		{0x8c63dcae977f3410ull, 0xaac05af8ca4094b6ull, -10038, -3022}, //81
		{0xe84183a2286112c1ull, 0x80ba076655b79201ull, -9959, -2998}, //82
		{0xc01e1a5eefb06e61ull, 0xb7ea1991c5311deaull, -9879, -2974}, //83
		{0x9eea7b6a9a4b9c0full, 0xabbaa68c398322bcull, -9799, -2950}, //84
		{0x8373c2ad7edf2a59ull, 0xfcfb59d62dee0d6eull, -9719, -2926}, //85
		{0xd9782aebd9ea99faull, 0x5bcfb583eadffa16ull, -9640, -2902}, //86
		{0xb3e2f01044a689afull, 0x63691a18aa9e8a77ull, -9560, -2878}, //87
		{0x94cc716194d7ff8bull, 0xa31fbfd826d3a8bbull, -9480, -2854}, //88
		{0xf62aa9163a6cc777ull, 0x3492dfedb1205eb6ull, -9401, -2830}, //89
		{0xcb9fd06ed037d5efull, 0xf9b6babc779eda06ull, -9321, -2806}, //90
		{0xa86f1e05a22d95e2ull, 0x21c441f377da6f60ull, -9241, -2782}, //91
		{0x8b534c3f2d5a1e71ull, 0x7104d13f25b07b5eull, -9161, -2758}, //92
		{0xe67e98345fb6bd01ull, 0xc6db3910a82fbefcull, -9082, -2734}, //93
		{0xbea91c6c0f02fbf3ull, 0x3fb5d48d6ea5643aull, -9002, -2710}, //94
		{0x9db5f34a74f6d793ull, 0x97b2c76b0e86fb33ull, -8922, -2686}, //95
		{0x82748c8a5426197full, 0x2a64297ab718bfd7ull, -8842, -2662}, //96
		{0xd7d1f4a86271a405ull, 0x0251bcce4ed904bfull, -8763, -2638}, //97
		{0xb285b133ad428a40ull, 0x59a73ba7eaa7be4aull, -8683, -2614}, //98
		{0x93ab8dc231246e7bull, 0x8e22bcdf817b8792ull, -8603, -2590}, //99
		{0xf44cbbc5bbbf338bull, 0xa5afde2c1434efd5ull, -8524, -2566}, //100
		{0xca147b73b6b3bc9bull, 0x11714066d1bdbb74ull, -8444, -2542}, //101
		{0xa7281b39548b0a78ull, 0x17c4d41ebec9e948ull, -8364, -2518}, //102
		{0x8a44ccfd2514bdfeull, 0xb927745ecf03505aull, -8284, -2494}, //103
		{0xe4bf1839e22209ddull, 0x72634a7aceb09921ull, -8205, -2470}, //104
		{0xbd36f2a101afa16eull, 0x498cb17b2f8b5ce0ull, -8125, -2446}, //105
		{0x9c83c22c37acae19ull, 0x988b01b3d7b5453cull, -8045, -2422}, //106
		{0x817745e3ec30b4f3ull, 0xe7dbec0b1adc4ce2ull, -7965, -2398}, //107
		{0xd62ef21bf8c343d1ull, 0x64c83d86a192ad33ull, -7886, -2374}, //108
		{0xb12b18647fff73b5ull, 0xb46b90d841a19a13ull, -7806, -2350}, //109
		{0x928cdb01fbbb9215ull, 0x298bec571b9250ddull, -7726, -2326}, //110
		{0xf2726e57af7c2b90ull, 0xd396b93c4fa84312ull, -7647, -2302}, //111
		{0xc88c25ffcfe29a6aull, 0xaf1275c913448af1ull, -7567, -2278}, //112
		{0xa5e3934f6d85b3ecull, 0x4d284ef54becf759ull, -7487, -2254}, //113
		{0x89385ae51beb727cull, 0x23af49f24fa26b9aull, -7407, -2230}, //114
		{0xe302fd0f04d39bfdull, 0x13149667e2f4239bull, -7328, -2206}, //115
		{0xbbc7977fd92d63b7ull, 0x20242e784ad06dceull, -7248, -2182}, //116
		{0x9b53e384eccabcf7ull, 0xb5025d88d4b252e1ull, -7168, -2158}, //117
		{0x807beaf84cbdacfaull, 0x9bd1ad7f672d1a8eull, -7088, -2134}, //118
		{0xd48f1d0f278787f3ull, 0x7407c1643d26b9aaull, -7009, -2110}, //119
		{0xafd3207e5071fe73ull, 0xc5548615755928bfull, -6929, -2086}, //120
		{0x917054e00917d62aull, 0x856bec6e83976bfeull, -6849, -2062}, //121
		{0xf09bb9c29e13f5d5ull, 0xd9fc62ca4cbb6599ull, -6770, -2038}, //122
		{0xc706ca40f848af8aull, 0xdcaf3c8e305adb1aull, -6690, -2014}, //123
		{0xa4a181774ffdd1dcull, 0x3953360c07d1883cull, -6610, -1990}, //124
		{0x882df1fb79c019fdull, 0x31237a622ec343bbull, -6530, -1966}, //125
		{0xe14a401d00d9b869ull, 0xd96c179b2c9c6c06ull, -6451, -1942}, //126
		{0xba5b0595508908eeull, 0xcde9f087251015c4ull, -6371, -1918}, //127
		{0x9a2652d2708abbc5ull, 0x40d1e60ac85d6288ull, -6291, -1894}, //128
		{0xff04f0198e68330full, 0x13e03a2791acc463ull, -6212, -1870}, //129
		{0xd2f26f568b2e5aeaull, 0x742e7ecc2487536cull, -6132, -1846}, //130
		{0xae7dc466adfd6636ull, 0x885f788de468d795ull, -6052, -1822}, //131
		{0x9055f723afd0c4edull, 0xe1a39d718fc41aa1ull, -5972, -1798}, //132
		{0xeec8970ab978d828ull, 0x3127f37fd16641e2ull, -5893, -1774}, //133
		{0xc5846270597c522aull, 0x74bb1b44c99b02acull, -5813, -1750}, //134
		{0xa361e0e9b7eb2e00ull, 0x4c51d7971cbb9ba7ull, -5733, -1726}, //135
		{0x87258e4c61f98132ull, 0xed9c010db3d0e2a8ull, -5653, -1702}, //136
		{0xdf94dad9da19a90aull, 0x863b1761b5f81798ull, -5574, -1678}, //137
		{0xb8f13778b7b1aae4ull, 0xcf34ee4d85107a08ull, -5494, -1654}, //138
		{0x98fb0b9b5fe2e6d5ull, 0xe731dbb0149142b2ull, -5414, -1630}, //139
		{0xfd15d2dbd4f2ec71ull, 0x8845cf598839e90full, -5335, -1606}, //140
		{0xd158e2d2ba80c5b9ull, 0xd89705c5169272f4ull, -5255, -1582}, //141
		{0xad2aff0d10715f7eull, 0x7d301faa2a17fb55ull, -5175, -1558}, //142
		{0x8f3dbd9c789285d3ull, 0x61f07ff390a0e336ull, -5095, -1534}, //143
		{0xecf8ff41c298c29cull, 0x583c6e84c16a3b4bull, -5016, -1510}, //144
		{0xc404e8d254351990ull, 0x5a2301e35fc03c1bull, -4936, -1486}, //145
		{0xa224ace8a836f9caull, 0xf25a813d2723a1e2ull, -4856, -1462}, //146
		{0x861f2beba4803237ull, 0x444f85b7752a98f7ull, -4776, -1438}, //147
		{0xdde2c6c84679b56dull, 0x1500629c19be6ef4ull, -4697, -1414}, //148
		{0xb78a27cbdeed7a41ull, 0x2fa1ea21c20ba23cull, -4617, -1390}, //149
		{0x97d2096f0787a855ull, 0xff0fa3a122b1b896ull, -4537, -1366}, //150
		{0xfb2a76dee9c63c44ull, 0x3c113692c8903ad9ull, -4458, -1342}, //151
		{0xcfc271702f5fb0edull, 0x5281fbd654168046ull, -4378, -1318}, //152
		{0xabdacb6ac4cdaeaeull, 0x9976ba1db19cbe05ull, -4298, -1294}, //153
		{0x8e27a4220e347d40ull, 0x58566171bbda2cc4ull, -4218, -1270}, //154
		{0xeb2ceb86ef0a799dull, 0xe02e386a0b15577cull, -4139, -1246}, //155
		{0xc28857b66a85a224ull, 0x9133012768ed3b84ull, -4059, -1222}, //156
		{0xa0e9e0bf58b8e865ull, 0x22f23b41de0b3210ull, -3979, -1198}, //157
		{0x851ac6f4aed868d0ull, 0xfff0a1a5e4bb1c9full, -3899, -1174}, //158
		{0xdc33fd77953b5388ull, 0x7303c1307e908919ull, -3820, -1150}, //159
		{0xb625d13b0276645full, 0xc0c88058b1f8e5a8ull, -3740, -1126}, //160
		{0x96ab47e5530e3f7cull, 0xf67df440542d1e30ull, -3660, -1102}, //161
		{0xf942d4d88c862412ull, 0xef74b0ac2290edf5ull, -3581, -1078}, //162
		{0xce2f15272fafcbb4ull, 0xcfc6d7d89a808c88ull, -3501, -1054}, //163
		{0xaa8d2482da2b17cbull, 0xbbcd37117f561799ull, -3421, -1030}, //164
		{0x8d13a6942deed098ull, 0xe552b4b76663232eull, -3341, -1006}, //165
		{0xe9645506ceeddb4bull, 0xcd31e81906ecdc98ull, -3262, -982}, //166
		{0xc10ea9772a3f97b9ull, 0x80c57cd9e8eb4964ull, -3182, -958}, //167
		{0x9fb177c22457409full, 0xa573bec3f2f72304ull, -3102, -934}, //168
		{0x84185b8a7d58f39aull, 0x3062e3fd554accabull, -3022, -910}, //169
		{0xda887883968532d8ull, 0xb954f8aa8cd011c7ull, -2943, -886}, //170
		{0xb4c42e7cb63e4e6cull, 0x961880c8a735e0cbull, -2863, -862}, //171
		{0x9586c29ebc3025c7ull, 0xf2534cb6a8a1afeaull, -2783, -838}, //172
		{0xf75ee58ca41fa82eull, 0xb89667ae1d0109dfull, -2704, -814}, //173
		{0xcc9ec7fbb6724170ull, 0xa21cbb0b50eec10bull, -2624, -790}, //174
		{0xa94205620ec95e5bull, 0xdf005355bebda249ull, -2544, -766}, //175
		{0x8c01c0da97ae92b2ull, 0x025cb44b83576f8aull, -2464, -742}, //176
		{0xe79f34fb32fedce8ull, 0x60a40e81fe0f4f93ull, -2385, -718}, //177
		{0xbf97d87a1781b402ull, 0x33a602852d86e0d0ull, -2305, -694}, //178
		{0x9e7b6d4e7749a2f5ull, 0x057ff467b8b43ba6ull, -2225, -670}, //179
		{0x8317e5d78c7ec9e0ull, 0x8b1910340c48485full, -2145, -646}, //180
		{0xd8e03194831cc4eeull, 0x205db8a0a240c065ull, -2066, -622}, //181
		{0xb3653a51d1da98e8ull, 0x4934103f67723e44ull, -1986, -598}, //182
		{0x946475443a2ef2abull, 0x1b0bdbaf1b76a902ull, -1906, -574}, //183
		{0xf57ea1cd234e48cdull, 0x9d378c0cb53dea27ull, -1827, -550}, //184
		{0xcb1183fd5d09e6b0ull, 0x35e0464174e78a28ull, -1747, -526}, //185
		{0xa7f9691ebd420d70ull, 0x12f242d968fc48d4ull, -1667, -502}, //186
		{0x8af1eee4fe885d22ull, 0x7c7fd8c0f0964bb4ull, -1587, -478}, //187
		{0xe5dd84ab12dadde4ull, 0x00bf3fe8d3e9949bull, -1508, -454}, //188
		{0xbe23df2f976f5fc1ull, 0xcac24074dcf694a6ull, -1428, -430}, //189
		{0x9d47bccabd7e403cull, 0x00cb214feeff4270ull, -1348, -406}, //190
		{0x8219620dca5d1e84ull, 0x62eb94ffdf1c14d3ull, -1268, -382}, //191
		{0xd73b225ee44ee73bull, 0x4365c02f215157d6ull, -1189, -358}, //192
		{0xb208ef855c969f4full, 0xbdbd2d335e51a935ull, -1109, -334}, //193
		{0xa1ebfb4219491a1full, 0x1014ebe6c5f90bf9ull, -1069, -322}, //194
		{0x93445b8731587ea3ull, 0x7ab3ee6afbe0211eull, -1029, -310}, //195
		{0x85f0468293f0eb4eull, 0x25bbf56008c58ea6ull, -989, -298}, //196
		{0xf3a20279ed56d48aull, 0x6b43527578c11110ull, -950, -286}, //197
		{0xdd95317f31c7fa1dull, 0x40405643d711d584ull, -910, -274}, //198
		{0xc987434744ac874eull, 0xa327ffb266b56221ull, -870, -262}, //199
		{0xb749faed14125d36ull, 0xcef980ec671f667bull, -830, -250}, //200
		{0xa6b34ad8c9dfc06full, 0xf42faa48c0ea481eull, -790, -238}, //201
		{0x979cf3ca6cec5b5aull, 0xa705992ceecf9c43ull, -750, -226}, //202
		{0x89e42caaf9491b60ull, 0xf41686c49db57245ull, -710, -214}, //203
		{0xfad2a4b13d1b5d6cull, 0x796b805720085f82ull, -671, -202}, //204
		{0xe41f3d6a7377eecaull, 0x20caba5f1d9e4a94ull, -631, -190}, //205
		{0xcf79cc9db955c2ccull, 0x7182148d4066eeb4ull, -591, -178}, //206
		{0xbcb2b812db11a5deull, 0x7415d448f6b6f0e8ull, -551, -166}, //207
		{0xab9eb47c81f5114full, 0x066ea92f3f326565ull, -511, -154}, //208
		{0x9c1661a651213e2dull, 0x06bea10ca65c084full, -471, -142}, //209
		{0x8df5efabc5979c8full, 0xca8d3ffa1ef463c2ull, -431, -130}, //210
		{0x811ccc668829b887ull, 0x0806357d5a3f5260ull, -391, -118}, //211
		{0xeadab0aba3b2dbe5ull, 0x2b45ac74ccea842full, -352, -106}, //212
		{0xd59944a37c0752a2ull, 0x4be76d3346f0495full, -312, -94}, //213
		{0xc24452da229b021bull, 0xfbe85badce996169ull, -272, -82}, //214
		{0xb0af48ec79ace837ull, 0x2d835a9df0c6d851ull, -232, -70}, //215
		{0xa0b19d2ab70e6ed6ull, 0x5b6aceaeae9d0ec4ull, -192, -58}, //216
		{0x9226712162ab070dull, 0xcab3961304ca70e9ull, -152, -46}, //217
		{0xb267ed1940f1c61cull, 0x55f038b237591ed4ull, -139, -42}, //218
		{0xd9c7dced53c72255ull, 0x96e7bd358c904a22ull, -126, -38}, //219
		{0x84ec3c97da624ab4ull, 0xbd5af13bef0b113full, -112, -34}, //220
		{0xa2425ff75e14fc31ull, 0xa1258379a94d028eull, -99, -30}, //221
		{0xc612062576589ddaull, 0x95364afe032a819eull, -86, -26}, //222
		{0xf1c90080baf72cb1ull, 0x5324c68b12dd6338ull, -73, -22}, //223
		{0x9392ee8e921d5d07ull, 0x3aff322e62439fcfull, -59, -18}, //224
		{0xb424dc35095cd80full, 0x538484c19ef38c95ull, -46, -14}, //225
		{0xdbe6fecebdedd5beull, 0xb573440e5a884d1cull, -33, -10}, //226
		{0x8637bd05af6c69b5ull, 0xa63f9a49c2c1b10full, -19, -6}, //227
		{0xa3d70a3d70a3d70aull, 0x3d70a3d70a3d70a3ull, -6, -2}, //228
		{0xc800000000000000ull, 0x0000000000000000ull, 7, 2}, //229
		{0xf424000000000000ull, 0x0000000000000000ull, 20, 6}, //230
		{0x9502f90000000000ull, 0x0000000000000000ull, 34, 10}, //231
		{0xb5e620f480000000ull, 0x0000000000000000ull, 47, 14}, //232
		{0xde0b6b3a76400000ull, 0x0000000000000000ull, 60, 18}, //233
		{0x878678326eac9000ull, 0x0000000000000000ull, 74, 22}, //234
		{0xa56fa5b99019a5c8ull, 0x0000000000000000ull, 87, 26}, //235
		{0xc9f2c9cd04674edeull, 0xa400000000000000ull, 100, 30}, //236
		{0xf684df56c3e01bc6ull, 0xc732000000000000ull, 113, 34}, //237
		{0x96769950b50d88f4ull, 0x1314448000000000ull, 127, 38}, //238
		{0xb7abc627050305adull, 0xf14a3d9e40000000ull, 140, 42}, //239
		{0xe0352f62a19e306eull, 0xd50b2037ad200000ull, 153, 46}, //240
		{0xcbea6f8ceb02bb39ull, 0x9bf4f8a69f764490ull, 193, 58}, //241
		{0xb975d6b6ee39e436ull, 0xb3e2fd538e122b44ull, 233, 70}, //242
		{0xa8acd7c0222311bcull, 0xc40832ea0d68ce0cull, 273, 82}, //243
		{0x9968bf6abbe85f20ull, 0x7e998b13cf4e1eccull, 313, 94}, //244
		{0x8b865b215899f46cull, 0xbd79e0d20082ee74ull, 353, 106}, //245
		{0xfdcb4fa002162a63ull, 0x73d9732fc7c8f7f7ull, 392, 118}, //246
		{0xe6d3102ad96cec1dull, 0xa60dc059157491e5ull, 432, 130}, //247
		{0xd1ef0244af2364ffull, 0x3207d795430cd926ull, 472, 142}, //248
		{0xbeeefb584aff8603ull, 0xaafb550ffacfd8fbull, 512, 154}, //249
		{0xada72ccc20054ae9ull, 0xaf561aa79a10ae6aull, 552, 166}, //250
		{0x9defbf01b061adabull, 0x3a0888136afa64a7ull, 592, 178}, //251
		{0x8fa475791a569d10ull, 0xf96e017d694487bcull, 632, 190}, //252
		{0x82a45b450226b39cull, 0xecc0024661173474ull, 672, 202}, //253
		{0xeda2ee1c7064130cull, 0x1162def06f79df73ull, 711, 214}, //254
		{0xd8210befd30efa5aull, 0x3c47f7e05401aa4eull, 751, 226}, //255
		{0xc491798a08a2ad4eull, 0xf1a6f2bab92a27e2ull, 791, 238}, //256
		{0xb2c71d5bca9023f8ull, 0x743e20e9ef511013ull, 831, 250}, //257
		{0xa298f2c501f45f42ull, 0x8349f3ba91b47b90ull, 871, 262}, //258
		{0x93e1ab8252f33b45ull, 0xcabb90e5c942b503ull, 911, 274}, //259
		{0x867f59a9d4bed6c0ull, 0x49ed8eabcccc485eull, 951, 286}, //260
		{0xf4a642e14c6262c8ull, 0xcd27bb612758c0faull, 990, 298}, //261
		{0xde81e40a034bcf4full, 0xf8077f7ea65e58d2ull, 1030, 310}, //262
		{0xca5e89b18b602368ull, 0x385bb19cb14bdfc4ull, 1070, 322}, //263
		{0xb80dc58e81fe95a1ull, 0x723627bbb5a4adb1ull, 1110, 334}, //264
		{0x983ee8424d642a92ull, 0x07e3766dba4f93c9ull, 1190, 358}, //265
		{0xfbde93488e0b1727ull, 0xc2c28f4e98fc19f1ull, 1269, 382}, //266
		{0xd0576d6c5a511cadull, 0xf002bf24b7e6ad9eull, 1349, 406}, //267
		{0xac560812c038d5fbull, 0xf506f6a6b517f3a7ull, 1429, 430}, //268
		{0x8e8d9491dcad8344ull, 0x05cf3a2fd94ce252ull, 1509, 454}, //269
		{0xebd59075cc44a6e4ull, 0x78429cbdd4d2682eull, 1588, 478}, //270
		{0xc313d78699812510ull, 0x50c95436c8b4f3b8ull, 1668, 502}, //271
		{0xa15d44ddf26149b7ull, 0xf2426143bf9f4590ull, 1748, 526}, //272
		{0x857a39f84f74cc5bull, 0x0e10854981a8c314ull, 1828, 550}, //273
		{0xdcd1e5ce9df53addull, 0x27943c27b5e59fb5ull, 1907, 574}, //274
		{0xb6a86f75f7acc699ull, 0x43707553c0f18cd9ull, 1987, 598}, //275
		{0x9717535a040c004cull, 0x7861ef57b8e4fce7ull, 2067, 622}, //276
		{0xf9f59393be1eb87aull, 0xafae593947aa9057ull, 2146, 646}, //277
		{0xcec2efe36dee726bull, 0xf30aa0ee74c776dfull, 2226, 670}, //278
		{0xab0771e7d3a22046ull, 0x351637431a0de942ull, 2306, 694}, //279
		{0x8d78d11a5706cd8dull, 0x97b9d56308a0c288ull, 2386, 718}, //280
		{0xea0bb28a646c62cdull, 0x8d8014f2dcef968bull, 2465, 742}, //281
		{0xc1991a71aa3e45d2ull, 0x76775be3da9cb5a3ull, 2545, 766}, //282
		{0xa023fbd9436b50baull, 0x49214065debff419ull, 2625, 790}, //283
		{0x8477153e15ec7085ull, 0x349100f8cad8ef38ull, 2705, 814}, //284
		{0xdb252e47ac71f957ull, 0x57696586cef91076ull, 2784, 838}, //285
		{0xb545cf2018ac77deull, 0x121f82b69e98e312ull, 2864, 862}, //286
		{0x95f1fc4f36c9cc9cull, 0x264a0bede1ab316eull, 2944, 886}, //287
		{0xf8104940491a7ef4ull, 0x0b0f05ecfdbaebaaull, 3023, 910}, //288
		{0xcd3183a99a60339dull, 0x4199e17bf905cbf0ull, 3103, 934}, //289
		{0xa9bb65548bc10a02ull, 0x47eee833ac2043e7ull, 3183, 958}, //290
		{0x8c6626f7599c5a4aull, 0x51c6196f1f7dbd9aull, 3263, 982}, //291
		{0xe8454d8f2e01e4a1ull, 0x67edb5236803fccaull, 3342, 1006}, //292
		{0xc0213cacba4dc3fbull, 0xff9ad4279ffac09cull, 3422, 1030}, //293
		{0x9eed13110e65260full, 0x0e6aac449a164dafull, 3502, 1054}, //294
		{0x8375e7a2e4cadbe2ull, 0xd7f3b77bc95bd47bull, 3582, 1078}, //295
		{0xd97bb718db1e472dull, 0x02409a4ced3d9074ull, 3661, 1102}, //296
		{0xb3e5df49f9656cefull, 0xab9b9724c351acd2ull, 3741, 1126}, //297
		{0x94cedec7c0263d5cull, 0xb5d274fef7216beeull, 3821, 1150}, //298
		{0xf62ead1afba8e460ull, 0x2f06c7ae19d067ddull, 3900, 1174}, //299
		{0xcba322ca36c1c474ull, 0xc707adb0a9dfce54ull, 3980, 1198}, //300
		{0xa871dd6bbd538c13ull, 0x91e6d54ffea76a6full, 4060, 1222}, //301
		{0x8b559215ad82f739ull, 0xe4f436eb273c6143ull, 4140, 1246}, //302
		{0xe6825ac64e7b9809ull, 0xac80eee39cc0f242ull, 4219, 1270}, //303
		{0xbeac38a431f525cfull, 0x6c3838843cab95ccull, 4299, 1294}, //304
		{0x9db885e872cc09d3ull, 0x431484034d764372ull, 4379, 1318}, //305
		{0x8276ad55efa1e19aull, 0xb77ffd59abff116cull, 4459, 1342}, //306
		{0xd7d579f22fb27a2full, 0xf4269a06098c2577ull, 4538, 1366}, //307
		{0xb2889abae543616cull, 0x77fde922cf8c45f8ull, 4618, 1390}, //308
		{0x93adf671edc112faull, 0xe99f86b5b4a8cf2eull, 4698, 1414}, //309
		{0xf450b7fe9cfcb00cull, 0x6ffbc1b939259fd7ull, 4777, 1438}, //310
		{0xca17c75c2a464108ull, 0xa3282a39f1f3da34ull, 4857, 1462}, //311
		{0xa72ad549cd9f9f70ull, 0xcace12d98f04eb3aull, 4937, 1486}, //312
		{0x8a470e6a05301b34ull, 0x4c36b60067dd741aull, 5017, 1510}, //313
		{0xe4c2d37f0204adf0ull, 0xf8c22422f0c98ed2ull, 5096, 1534}, //314
		{0xbd3a08cf4d1d9119ull, 0x14f46242d4aee9f2ull, 5176, 1558}, //315
		{0x9c864fcb84c2a7b2ull, 0xa64809131821604full, 5256, 1582}, //316
		{0x8179628dd2720adfull, 0x36262fb30ca4cf14ull, 5336, 1606}, //317
		{0xd632708ff1494c14ull, 0x5b596457f6ff40cfull, 5415, 1630}, //318
		{0xb12dfc444ae10aa1ull, 0xf50c95b13b3e30daull, 5495, 1654}, //319
		{0x928f3f046fe9892eull, 0x6b764ffd76f30c4full, 5575, 1678}, //320
		{0xf27662d3d3ad52d7ull, 0xbc1f765aaf490d62ull, 5654, 1702}, //321
		{0xc88f6b81d5c5893eull, 0xf3b0366ebebac19bull, 5734, 1726}, //322
		{0xa5e648149fe1786aull, 0xef25f571a9728f18ull, 5814, 1750}, //323
		{0x893a97f0ed1dac89ull, 0xd11c59949e873939ull, 5894, 1774}, //324
		{0xe306b1158213ca67ull, 0x8ed7c8b7b690682eull, 5973, 1798}, //325
		{0xbbcaa7b0064eb7efull, 0xc96b9dc6947e95f0ull, 6053, 1822}, //326
		{0x9b566c2f3badf7e9ull, 0x5deae9358a4accffull, 6133, 1846}, //327
		{0x807e03888348b561ull, 0xbbb77b1217fbaab2ull, 6213, 1870}, //328
		{0xd49294ba9094b129ull, 0x3f4828ae387c02faull, 6292, 1894}, //329
		{0xafd5fec1a85999fbull, 0xd23802d659f8279aull, 6372, 1918}, //330
		{0x9172b43e4956991cull, 0xeb18fd20777439fdull, 6452, 1942}, //331
		{0xf09fa6910ac7f3f3ull, 0x4df62de14e8c5c99ull, 6531, 1966}, //332
		{0xc70a0968fd74e24full, 0x5be37caba63642c5ull, 6611, 1990}, //333
		{0xa4a430fb82ddcf49ull, 0xe0e36da6264c5c8cull, 6691, 2014}, //334
		{0x88302aaebc8b99b6ull, 0xb5a08db943049c3eull, 6771, 2038}, //335
		{0xe14decf2ec330877ull, 0x25213defc2a85470ull, 6850, 2062}, //336
		{0xba5e0fd301d294b2ull, 0x03fc0691cfa12d1aull, 6930, 2086}, //337
		{0x9a28d69160f3e206ull, 0x09125ea8e59e549bull, 7010, 2110}, //338
		{0xff09191687f43c20ull, 0x57dc1f3f3bf4a5fbull, 7089, 2134}, //339
		{0xd2f5e0469040e0ebull, 0x48c67661c087fb5bull, 7169, 2158}, //340
		{0xae809d1877c07590ull, 0xc98a27fe4b380c05ull, 7249, 2182}, //341
		{0x905851e6befed710ull, 0xe8fb1798e7acee76ull, 7329, 2206}, //342
		{0xeecc7c3a5714c807ull, 0x75d6bcab6fdd41b5ull, 7408, 2230}, //343
		{0xc5879b4ab2cad93bull, 0x1920e811f26bf0d5ull, 7488, 2254}, //344
		{0xa3648b371e97f040ull, 0x2bee58c31fb7f783ull, 7568, 2278}, //345
		{0x8727c2af865f0a84ull, 0xa366e667b0ada1cbull, 7648, 2302}, //346
		{0xdf98808d28f8f41dull, 0x1d973b7339d80f45ull, 7727, 2326}, //347
		{0xb8f43bcf7901a604ull, 0x9fb5abed8d052021ull, 7807, 2350}, //348
		{0x98fd8a787edb5fe2ull, 0x7817aade2d880707ull, 7887, 2374}, //349
		{0xfd19f3c527ee43ccull, 0x69f12fe7e3400820ull, 7966, 2398}, //350
		{0xd15c4d146d8536c2ull, 0x3ac204a49f77d3dfull, 8046, 2422}, //351
		{0xad2dd2381bbedc9dull, 0x7340623032e0ddffull, 8126, 2446}, //352
		{0x8f4013cd480fae66ull, 0x8f6c9a8568fa03deull, 8206, 2470}, //353
		{0xecfcdce15c904d37ull, 0x19bd5641f077b5e7ull, 8285, 2494}, //354
		{0xc4081b6b3e8e1241ull, 0xbb8aebf78ab49f47ull, 8365, 2518}, //355
		{0xa2275209622b4d3full, 0x4703d1db0b5fa394ull, 8445, 2542}, //356
		{0x86215c070a1ef024ull, 0x68fc16a74a54002full, 8525, 2566}, //357
		{0xdde66566d3321b5cull, 0x5f9bbae5d158d172ull, 8604, 2590}, //358
		{0xb78d264725b75c31ull, 0xd045af7cdda5302eull, 8684, 2614}, //359
		{0x97d48373cf8ddfc8ull, 0x83ad7c7035f52c16ull, 8764, 2638}, //360
		{0xfb2e8fc4447cfe0aull, 0xef1014f475126167ull, 8843, 2662}, //361
		{0xcfc5d51088e291e7ull, 0x9a6470b2f14a8634ull, 8923, 2686}, //362
		{0xabdd9919cc572e4dull, 0x59db4ac3d965776dull, 9003, 2710}, //363
		{0x8e29f5c97e03bec8ull, 0x20b820a4d947c6f5ull, 9083, 2734}, //364
		{0xeb30c1a534180741ull, 0xa497195b9fe6b3feull, 9162, 2758}, //365
		{0xc28b841a0b0eb1fbull, 0xa3be4d6040fb7562ull, 9242, 2782}, //366
		{0xa0ec80bd71c84cbcull, 0x86410623582d696full, 9322, 2806}, //367
		{0x851cf2d0a50dbb3aull, 0x1bd00f56ea359693ull, 9402, 2830}, //368
		{0xdc37950f1f3ad810ull, 0x9c4ca16fb74e8361ull, 9481, 2854}, //369
		{0xb628c9e62dee69dfull, 0xc42e89468b3f85e7ull, 9561, 2878}, //370
		{0x96adbd1b2c39a536ull, 0xc2f51723373b37cfull, 9641, 2902}, //371
		{0xf946e5c97ed2c15full, 0xfca6e08843a875f8ull, 9720, 2926}, //372
		{0xce3272330f0edc21ull, 0xd315d0616dcd6afeull, 9800, 2950}, //373
		{0xaa8fecc083cd89c1ull, 0x84e08a6877d48cd5ull, 9880, 2974}, //374
		{0x8d15f3bb0cd81e49ull, 0xbd3ee8ec9ae50a97ull, 9960, 2998}, //375
		{0xe96823b2514a57c6ull, 0xb94c527c548e81cdull, 10039, 3022}, //376
		{0xc111cfb18e8a0d65ull, 0x33b6902c83dd5dddull, 10119, 3046}, //377
		{0x9fb412a794d410fcull, 0xcb66c031aedb1846ull, 10199, 3070}, //378
		{0x841a832f435fff44ull, 0x6b21539ca68627a9ull, 10279, 3094}, //379
		{0xda8c0921c288f4e3ull, 0xec87c05ed3b2aba7ull, 10358, 3118}, //380
		{0xb4c721630f84aaadull, 0xd9bab3111060fc3dull, 10438, 3142}, //381
		{0x9589330efc54e836ull, 0x3207f185a457ec52ull, 10518, 3166}, //382
		{0xf762ee989fa60250ull, 0x485d72a1ad5a5f22ull, 10597, 3190}, //383
		{0xcca21e7fe20d5faaull, 0xa0e24377bc2219a6ull, 10677, 3214}, //384
		{0xa944c838ebb57ed1ull, 0x8b77bf2016522697ull, 10757, 3238}, //385
		{0x8c040989a3604469ull, 0x9b9e2decac1e211cull, 10837, 3262}, //386
		{0xe7a2fc4268990f71ull, 0x1d2c0901a04b6f3full, 10916, 3286}, //387
		{0xbf9af89735b84ec6ull, 0x340953adfa694d5eull, 10996, 3310}, //388
		{0x9e7e0325242af401ull, 0xd2d81d3473118004ull, 11076, 3334}, //389
		{0x831a094d518fcb35ull, 0xcea584812ebc3710ull, 11156, 3358}, //390
		{0xd8e3bb46db64d07dull, 0x7b53e02c23bbaf5dull, 11235, 3382}, //391
		{0xb368277e8c2652ddull, 0xa90f9ac2c29abee8ull, 11315, 3406}, //392
		{0x9466e0f8250173aeull, 0x3892e93bf1e971f5ull, 11395, 3430}, //393
		{0xf582a3037bb659bfull, 0xcf73cb0e0783800full, 11474, 3454}, //394
		{0xcb14d40682739541ull, 0xbcdf217a34bfe9c2ull, 11554, 3478}, //395
		{0xa7fc26994a248791ull, 0x07900b7cdd66fc09ull, 11634, 3502}, //396
		{0x8af43324e3b8626eull, 0x4019fe9af2c812ddull, 11714, 3526}, //397
		{0xe5e1449c558e558eull, 0xf3d592c1d6b30174ull, 11793, 3550}, //398
		{0xbe26f93b4e83be00ull, 0x49ebaf505aa59c7eull, 11873, 3574}, //399
		{0x9d4a4d9c788574b7ull, 0xcd6929e1686a2b17ull, 11953, 3598}, //400
		{0x821b815cadcc7f83ull, 0x44fffa235c2bab49ull, 12033, 3622}, //401
		{0xd73ea532d8d1a2b4ull, 0x5203bc96f9aa8a4aull, 12112, 3646}, //402
		{0xb20bd70395601bc2ull, 0x5bd7ef1ce7b4c05cull, 12192, 3670}, //403
		{0x9346c287f890839full, 0xecd38ad7ba705c6full, 12272, 3694}, //404
		{0xf3a5fbe9d886e58full, 0x87b6ccd42d258c07ull, 12351, 3718}, //405
		{0xc98a8ce1f8da1415ull, 0xadda65275318f02cull, 12431, 3742}, //406
		{0xa6b603016f09032eull, 0xf86e7b0f04f0b065ull, 12511, 3766}, //407
		{0x89e66c8453d5ee0full, 0xd1a4b7dc3dca4df9ull, 12591, 3790}, //408
		{0xe422f6140143845cull, 0x97305b60f0621458ull, 12670, 3814}, //409
		{0xbcb5cc18f2e95b6dull, 0x805b4230b0947e1full, 12750, 3838}, //410
		{0x9c18ed7cd8ff5284ull, 0xc8392306dd03053dull, 12830, 3862}, //411
		{0x811ee7969986ea58ull, 0x8bab19bc1f69492cull, 12910, 3886}, //412
		{0xd59cc0a662a48844ull, 0x5fd2461cb500ef11ull, 12989, 3910}, //413
		{0xb0b22ac738d82144ull, 0xdf5cdd3530e5a7beull, 13069, 3934}, //414
		{0x9228d3782626a4abull, 0xec5afd069d580df9ull, 13149, 3958}, //415
		{0xf1ccf239514d9be4ull, 0xec5ed9877b591f5aull, 13228, 3982}, //416
		{0xc8034338bf794dd4ull, 0x3e620d8546efa536ull, 13308, 4006}, //417
		{0xa572589aa1a56b4full, 0x4a78b88f1bf1954bull, 13388, 4030}, //418
		{0x88dab1a74e3623ccull, 0x5b965d930953e8e6ull, 13468, 4054}, //419
		{0xe2680a0a49099905ull, 0x1344f00765565671ull, 13547, 4078}, //420
		{0xbb476bb5f4027d14ull, 0xd9a0d2f38334d986ull, 13627, 4102}, //421
		{0x9ae9de3e69c094ddull, 0x3700a3e9a1bb2afbull, 13707, 4126}, //422
		{0x8024383bab19730dull, 0x1d76f2d15166ec20ull, 13787, 4150}, //423
		{0xd3fe076e41c9fa35ull, 0x7f931391840a9990ull, 13866, 4174}, //424
		{0xaf5b1da88cad255eull, 0x72c814d63b10d89dull, 13946, 4198}, //425
		{0x910d0f8aa97f56bfull, 0xb33e237394a31bf2ull, 14026, 4222}, //426
		{0xeff77eed3c172a4full, 0x53ad562c25679fa6ull, 14105, 4246}, //427
		{0xc67ef13cabf1c1daull, 0x7285fafe41953c58ull, 14185, 4270}, //428
		{0xa43122978e2f7d88ull, 0x53d23cb1a36da96aull, 14265, 4294}, //429
		{0x87d0fe94f2aa46b8ull, 0x9d37fc4ebfd8ada3ull, 14345, 4318}, //430
		{0xe0b079ece542d717ull, 0x9aad5e426645275dull, 14424, 4342}, //431
		{0xb9dbd2a2c5371e96ull, 0xa0882cd7e2718bf5ull, 14504, 4366}, //432
		{0x99bd1b621ac83d01ull, 0xcc5b42ba7927e7c5ull, 14584, 4390}, //433
		{0xfe56df277f183cbfull, 0xaf7ed7fc590c388full, 14663, 4414}, //434
		{0xd262736348b956d4ull, 0x8e0174ad7efd123eull, 14743, 4438}, //435
		{0xae06aa909bfbeefcull, 0xeada8fcd2b2b4f75ull, 14823, 4462}, //436
		{0x8ff37289bad0353full, 0xfd9d373859b26a98ull, 14903, 4486}, //437
		{0xee259b0e8f1efac6ull, 0x8e2fb5245f9771d6ull, 14982, 4510}, //438
		{0xc4fd912ad93f5320ull, 0xee1526c091af62e0ull, 15062, 4534}, //439
		{0xa2f25c3433931336ull, 0xa3fec112ec9a6decull, 15142, 4558}, //440
		{0x86c94f5c1741640full, 0xfd5d405c921123a8ull, 15222, 4582}, //441
		{0xdefc3f36506d3fecull, 0x27a13e6894009876ull, 15301, 4606}, //442
		{0xb872fb7a679894ccull, 0xc467f38074fa98ceull, 15381, 4630}, //443
		{0x9892a07196d8608aull, 0xa88f53f24dc1215bull, 15461, 4654}, //444
		{0xfc6913dbd8e86ad1ull, 0xd525628e10995b3eull, 15540, 4678}, //445
		{0xd0c9fe6a3c1622cdull, 0x9cd9b498821b59afull, 15620, 4702}, //446
		{0xacb4cc72538a7a54ull, 0xc0d011d7b9458d31ull, 15700, 4726}, //447
		{0x8edbf847becb677dull, 0x787894244d8a7252ull, 15780, 4750}, //448
		{0xec573fb3c65af944ull, 0x61260fd51ea2ddf3ull, 15859, 4774}, //449
		{0xc37f1d4b91d76d25ull, 0x91f1f9c45089f209ull, 15939, 4798}, //450
		{0xa1b600b5d1586261ull, 0x0a3a0afe82eefc53ull, 16019, 4822}, //451
		{0x85c3a013394f6136ull, 0x520e93177283a39full, 16099, 4846}, //452
		{0xdd4b536dae5d7f2aull, 0xf2211b49226bb7c0ull, 16178, 4870}, //453
		{0xb70ce0e2556456b4ull, 0xf2b78b3a09fe163bull, 16258, 4894}, //454
		{0x976a68ff32836be2ull, 0xcc136cbd950fea25ull, 16338, 4918}, //455
		{0xfa7f0740e3deb335ull, 0x5d7ec38e60213c99ull, 16417, 4942}, //456
		{0xcf34a273bb7ea9a3ull, 0xf4d36655a35ec0d4ull, 16497, 4966}  //457
	};

	constexpr static int32_t inv_log2_10 = 646456993;
	constexpr static int32_t log2_10 = 1783446566;

	template <typename T>
	struct entry {
		constexpr static int m_bit_count = sizeof(T) * 8 - 1;

		entry(int32_t exp10_, int32_t exp2_);

		T m;
		int32_t exp10;
		int32_t exp2;

	};

	template <typename T>
	static entry<T> lookup_exp10(int32_t exp2)
	{
		exp2 += entry<T>::m_bit_count;
		int64_t acc(inv_log2_10);
		acc *= exp2;
		return entry<T>((int32_t)(acc >> 31), exp2);
	}

	static void lookup_exp10_r0(u_entry &e, int32_t &exp10, int32_t &exp2)
	{
		auto idx((exp10 + 4966) / 24);
		e = exp10_list[idx];

		exp10 = (idx * 24) - 4942;
		int64_t acc(log2_10);
		acc *= exp10;
		exp2 = int32_t(acc >> 29);
	}

	static void lookup_exp10_r1(u_entry &e, int32_t &exp10, int32_t &exp2)
	{
		auto idx((exp10 + 334) / 12);
		e = exp10_list[idx + 194];

		exp10 = (idx * 12) - 322;
		int64_t acc(log2_10);
		acc *= exp10;
		exp2 = int32_t(acc >> 29);
	}

	static void lookup_exp10_r2(u_entry &e, int32_t &exp10, int32_t &exp2)
	{
		auto idx((exp10 + 46) >> 2);
		e = exp10_list[idx + 218];

		exp10 = (idx << 2) - 42;
		int64_t acc(log2_10);
		acc *= exp10;
		exp2 = int32_t(acc >> 29);
	}

	static void lookup_exp10_r3(u_entry &e, int32_t &exp10, int32_t &exp2)
	{
		auto idx((exp10 - 46) / 12);
		e = exp10_list[idx + 241];

		exp10 = (idx * 12) + 58;
		int64_t acc(log2_10);
		acc *= exp10;
		exp2 = int32_t(acc >> 29);
	}

	static void lookup_exp10_r4(u_entry &e, int32_t &exp10, int32_t &exp2)
	{
		auto idx((exp10 - 334) / 24);
		e = exp10_list[idx + 265];

		exp10 = (idx * 24) + 358;
		int64_t acc(log2_10);
		acc *= exp10;
		exp2 = int32_t(acc >> 29);
	}
};

template <>
template <>
inline binary_exp10<void>::entry<uint32_t>::entry(
	int32_t exp10_, int32_t exp2_
) : exp10(exp10_), exp2(exp2_)
{
	constexpr static uint64_t round_mask = uint64_t(1) << 31;

	u_entry e;
	lookup_exp10_r2(e, exp10, exp2);
	m = (int32_t)(e.m_high >> 32);
	if (e.m_high & round_mask)
		m += 1;

	exp2 -= 31;
}

template <>
template <>
inline binary_exp10<void>::entry<uint64_t>::entry(
	int32_t exp10_, int32_t exp2_
) : exp10(exp10_), exp2(exp2_)
{
	constexpr static uint64_t round_mask = uint64_t(1) << 63;

	u_entry e;
	if (exp10 >= -46) {
		if (exp10 < 46)
			lookup_exp10_r2(e, exp10, exp2);
		else
			lookup_exp10_r3(e, exp10, exp2);
	} else
		lookup_exp10_r1(e, exp10, exp2);

	m = e.m_high;
	if (e.m_low & round_mask)
		m += 1;

	exp2 -= 63;
}

template <>
template <>
inline binary_exp10<void>::entry<uint128_t>::entry(
	int32_t exp10_, int32_t exp2_
) : exp10(exp10_), exp2(exp2_)
{
	u_entry e;

	if (exp10 >= -46) {
		if (exp10 < 46)
			lookup_exp10_r2(e, exp10, exp2);
		else if (exp10 < 334)
			lookup_exp10_r3(e, exp10, exp2);
		else
			lookup_exp10_r4(e, exp10, exp2);
	} else {
		if (exp10 >= -334)
			lookup_exp10_r1(e, exp10, exp2);
		else
			lookup_exp10_r0(e, exp10, exp2);
	}

	m = soft_uint128_t(e.m_high, e.m_low);
	exp2 -= 127;
}

template <typename Dummy>
constexpr typename binary_exp10<Dummy>::u_entry
binary_exp10<Dummy>::exp10_list[458];

}}}
#endif

