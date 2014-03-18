#include <cstdio>
#include <array>
#include <chrono>
#include <yesod/coder/blowfish.hpp>

#include <openssl/blowfish.h>

using namespace ucpf::yesod::coder;

int main()
{
	uint8_t x_key[] = "abbbap";
	blowfish cd;

	cd.set_key(x_key, 6);

	std::array<uint64_t, 4> in = {{
		0x12345678ull, 0x984674ull, 0x34985ull, 0x348573908ull
	}};

	BF_KEY r_key;
	BF_set_key(&r_key, 6, x_key);

	for (auto c(0); c < 18; ++c) {
		printf("m %08x, s %08x, eq %d\n", cd.k_box[c], r_key.P[c], cd.k_box[c] == r_key.P[c]);
	}

	for (auto d: in) {
		auto x1(cd.encrypt(d));
		auto x2(cd.decrypt(x1));
		auto x3(d);
		BF_encrypt(reinterpret_cast<uint32_t *>(&x3), &r_key);
		auto x4(x3);
		BF_decrypt(reinterpret_cast<uint32_t *>(&x4), &r_key);
		printf("in_x %zx, out1 %zx, out2 %zx, eq %d\n", d, x1, x2, d == x2);
		printf("in_r %zx, out1 %zx, out2 %zx, eq %d\n", d, x3, x4, d == x4);
	}

	return 0;
}
