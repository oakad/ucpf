#include <map>
#include <vector>
#include <random>
#include <iostream>
#include <yesod/bitops.hpp>
#include <yesod/coder/xxhash.hpp>

using namespace ucpf;

template <typename Map>
int check_collisions(int order, Map const &map)
{
	while (order < 31) {
		std::vector<bool> b_set(std::size_t(1) << order, false);
		uint32_t mask((uint32_t(1) << order) - 1);
		bool clean(true);

		for (auto p: map) {
			if (b_set[p.first & mask]) {
				clean = false;
				break;
			}

			b_set[p.first & mask] = true;
		}
		if (clean)
			return order;

		++order;
	}
	return order;
}

template <typename Map>
Map rehash(int seed, Map const &map)
{
	yesod::coder::xxhash<> h(seed);
	Map x_map;

	for (auto p: map) {
		h.update(p.second.begin(), p.second.end());
		auto d(h.digest());
		if (!(x_map.insert(std::make_pair(d, p.second))).second) {
			x_map.clear();
			break;
		}

		h.reset(seed);
	}

	return x_map;
}

#define FIND_RANDOM 1

int main(int argc, char **argv)
{
	uint32_t seed(0);
	yesod::coder::xxhash<> h(seed);
	std::string s_in;
	std::map<uint32_t, std::string> x_map;

	while (std::getline(std::cin, s_in)) {
		if (s_in.empty())
			continue;

		h.update(s_in.begin(), s_in.end());
		auto d(h.digest());
		if (!(x_map.insert(std::make_pair(d, s_in)).second)) {
			printf("full collision at %s\n", s_in.c_str());
			return -1;
		}

		h.reset(seed);
	}

	auto bit_ord(yesod::order_base_2(x_map.size()));
	auto min_ord(check_collisions(bit_ord, x_map));
#if FIND_RANDOM
	static std::random_device src;
	std::mt19937 gen(src());
	std::uniform_int_distribution<uint32_t> dis;

	for (int retries = 1000000; retries; --retries) {
		auto x_seed(dis(gen));
		auto n_map(std::move(rehash(x_seed, x_map)));
		if (n_map.empty())
			continue;

		auto n_ord(check_collisions(bit_ord, n_map));
		if (n_ord < min_ord) {
			min_ord = n_ord;
			x_map.swap(n_map);
			seed = x_seed;
		}
	}
#else
	for (uint32_t x_seed(0); x_seed < 0x10000000; ++x_seed) {
		auto n_map(std::move(rehash(x_seed, x_map)));
		if (n_map.empty())
			continue;

		auto n_ord(check_collisions(bit_ord, n_map));
		if (n_ord < 7) {
			min_ord = n_ord;
			x_map.swap(n_map);
			seed = x_seed;
			break;
		}
	}
#endif
	printf("no collisions at order %d, seed %08x\n", min_ord, seed);
	std::vector<std::string> out(std::size_t(1) << min_ord, std::string());
	uint32_t mask((uint32_t(1) << min_ord) - 1);

	for (auto p: x_map)
		out[p.first & mask] = p.second;

	for (auto s: out)
		printf(
			"{\"%s\", %zd, &entry<int, level, >::set},\n",
			s.c_str(), s.size()
		);

	return 0;
}
