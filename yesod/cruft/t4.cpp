#include <cstdio>
#include <array>
#include <chrono>
#include <yesod/coder/detail/pi_word.hpp>

using namespace ucpf::yesod::coder::detail;

int main()
{
	std::array<uint32_t, 16> v_bell;
	std::array<uint32_t, 16> v_bbp;
	uint32_t base(12484), cnt;

	auto t1(std::chrono::high_resolution_clock::now());
	cnt = base;
	for (auto &d: v_bell) {
		d = bellard_pi_word(cnt++);
	}

	auto t2(std::chrono::high_resolution_clock::now());
	cnt = base;
	for (auto &d: v_bbp) {
		d = bbp_pi_word(cnt++);
	}

	auto t3(std::chrono::high_resolution_clock::now());
	cnt = 0;
	bool f(true);
	for (auto &d: v_bell) {
		if (d != v_bbp[cnt++]) {
			printf(
				"not equal at %d, bpp %08x, bell %08x\n",
				cnt - 1, d, v_bbp[cnt - 1]
			);
			f = false;
		}
		printf("bell %08x, bbp %08x\n", d, v_bbp[cnt - 1]);
	}
	if (f)
		printf("all equal\n");

	printf(
		"delta bell %ld\n", std::chrono::duration_cast<
			std::chrono::nanoseconds
		>(t2 - t1).count()
	);
	printf(
		"delta bbp %ld\n", std::chrono::duration_cast<
			std::chrono::nanoseconds
		>(t3 - t2).count()
	);
	return 0;
}
