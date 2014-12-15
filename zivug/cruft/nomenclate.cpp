#include <set>
#include <vector>
#include <random>
#include <iostream>
#include <yesod/bitops.hpp>
#include <yesod/coder/xxhash.hpp>

using namespace ucpf;

template <typename Map>
int check_collisions(int order, int limit_order, Map const &map)
{
	static std::vector<bool> b_set;

	while (order <= limit_order) {
		b_set.clear();
		b_set.resize(std::size_t(1) << order, false);
		uint32_t mask((uint32_t(1) << order) - 1);
		bool clean(true);

		for (auto &p: map) {
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
void rehash(int seed, Map &map)
{
	yesod::coder::xxhash<> h(seed);

	for (auto &p: map) {
		h.update(
			reinterpret_cast<uint8_t const *>(p.second.c_str()),
			p.second.size()
		);
		p.first = h.digest();
		h.reset(seed);
	}
}

#define FIND_RANDOM 0

int main(int argc, char **argv)
{
	uint32_t seed(~0u);
	yesod::coder::xxhash<> h(seed);
	std::string s_in;
	std::vector<std::pair<uint32_t, std::string>> x_map;

	while (std::getline(std::cin, s_in)) {
		if (s_in.empty())
			continue;

		h.update(
			reinterpret_cast<uint8_t const *>(s_in.c_str()),
			s_in.size()
		);
		auto d(h.digest());
		x_map.emplace_back(d, s_in);
		h.reset(seed);
	}

	{
		std::set<uint32_t> check;
		for (auto p: x_map) {
			if (!check.emplace(p.first).second) {
				printf(
					"full collision at %s\n",
					p.second.c_str()
				);
				return -1;
			}
		}
	}

	auto bit_ord(yesod::order_base_2(x_map.size()));
	auto min_ord(check_collisions(bit_ord, 31, x_map));
	auto n_map(x_map);

#if FIND_RANDOM
	static std::random_device src;
	std::mt19937 gen(src());
	std::uniform_int_distribution<uint32_t> dis;

	for (int retries = 100000; retries; --retries) {
		auto x_seed(dis(gen));
		rehash(x_seed, n_map);

		auto n_ord(check_collisions(bit_ord, min_ord + 1, n_map));
		if (n_ord < min_ord) {
			min_ord = n_ord;
			x_map.swap(n_map);
			seed = x_seed;
		}
	}
#else
	for (
		auto x_seed(0u); x_seed < (~0u); ++x_seed
	) {
		if (!(x_seed % (1 << 26)))
			fprintf(stderr, "x_seed at %x\n", x_seed);

		rehash(x_seed, n_map);

		auto n_ord(check_collisions(bit_ord, min_ord, n_map));
		if (n_ord < min_ord) {
			min_ord = n_ord;
			x_map.swap(n_map);
			seed = x_seed;
			if (n_ord < 7)
				break;
		}
	}
#endif
	printf("no collisions at order %d, seed %08x\n", min_ord, seed);
	std::vector<std::string> out(std::size_t(1) << min_ord, std::string());
	uint32_t mask((uint32_t(1) << min_ord) - 1);

	for (auto const &p: x_map)
		out[p.first & mask] = p.second;

	for (auto const &s: out) {
		std::string s_up;
		for (auto cc: s)
			s_up.push_back(std::toupper(cc));

		printf(
			"\t\t{\"%s\", %zd, &ption_entry<int, level, SO_%s>::impl},\n",
			s.c_str(), s.size(), s_up.c_str()
		);
	}

	return 0;
}
