#include <cstdio>
#include <array>
#include <chrono>
#include <yesod/coder/blowfish.hpp>

using namespace ucpf::yesod::coder;

int main()
{
	uint8_t x_key[] = "abbbap";
	blowfish cd;

	cd.set_key(x_key, 6);

	std::array<uint64_t, 4> in = {{
		0x12345678ull, 0x984674ull, 0x34985ull, 0x348573908ull
	}};

	for (auto d: in) {
		auto x1(cd.encrypt(d));
		auto x2(cd.decrypt(x1));
		printf("in %zx, out1 %zx, out2 %zx, eq %d\n", d, x1, x2, d == x2);
	}

	return 0;
}
