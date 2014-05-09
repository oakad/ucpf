#include <array>
#include <tuple>
#include <bitset>
#include <iostream>
#include <yesod/flat_map.hpp>

using ucpf::yesod::flat_map;

int main()
{
	flat_map<int, int> m0{
		std::make_tuple(1, 10),
		std::make_tuple(2, 20),
		std::make_tuple(3, 30)
	};

	printf("s %zd, v[2] %d\n", m0.size(), m0[2]);

	for (auto &r: m0) {
		printf("k: %d, v: %d\n", std::get<0>(r), std::get<1>(r));
		printf("xx %d\n", std::get<0>(*m0.lower_bound(std::get<0>(r))));
	}


	auto i_p1(m0.insert(std::make_tuple(4, 40)));
	auto i_p2(m0.insert(std::make_pair(3, 30)));
	m0.erase(1);

	return 0;
}
