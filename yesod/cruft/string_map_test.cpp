#include <yesod/string_map.hpp>
#include <iostream>

using ucpf::yesod::string_map;

int main(int argc, char **argv)
{
	std::string s_in;
	string_map<char, int> x_map;
	int ord(0);

	x_map.emplace_at(std::string("abcd"), 0);
	x_map.dump(std::cout);
	std::cout << "---------\n";
	x_map.emplace_at(std::string("abcdefghijkl"), 0);
	x_map.dump(std::cout);
	std::cout << "---------\n";
	x_map.emplace_at(std::string("abcdefghijklmnopqrst"), 0);
	x_map.dump(std::cout);
	std::cout << "---------\n";
	x_map.emplace_at(std::string("abcdefghijklmnopqrstuvwxyz"), 0);
	x_map.dump(std::cout);
#if 0
	for (auto c(getchar()); c != EOF; c = getchar()) {
		if (c == '\n' || c == '\r') {
			if (!s_in.empty()) {
				printf("in %s\n", s_in.c_str());
				x_map.emplace_at(s_in, ord);
				++ord;
				s_in.clear();
			}
		} else
			s_in.push_back(c);
	}

	x_map.dump(std::cout);
#endif
	return 0;
}
