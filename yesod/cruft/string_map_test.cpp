#include <yesod/string_map.hpp>
#include <iostream>

using ucpf::yesod::string_map;

int main(int argc, char **argv)
{
	std::vector<char> s_in;
	string_map<char, int> x_map;
	int ord(0);

	for (auto c(getchar()); c != EOF; c = getchar()) {
		if (c == '\n' || c == '\r') {
			if (!s_in.empty()) {
				std::string x(s_in.begin(), s_in.end());
				printf("in %s\n", x.c_str());
				x_map.emplace_at(s_in.begin(), s_in.end(), ord);
				++ord;
				s_in.clear();
			}
		} else
			s_in.push_back(c);
	}

	x_map.dump(std::cout);
	return 0;
}
