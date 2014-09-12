#include <yesod/string_map.hpp>
#include <iostream>

using ucpf::yesod::string_map;

int main(int argc, char **argv)
{
	std::string s_in;
	string_map<char, int> x_map;
	int ord(0);

	x_map.emplace_at("abcd", 1);
	x_map.dump(std::cout);
	std::cout << "---------\n";
	x_map.emplace_at("abcdefghijkl", 2);
	x_map.dump(std::cout);
	std::cout << "---------\n";
	x_map.emplace_at("abcdefghijklmnopqrst", 3);
	x_map.dump(std::cout);
	std::cout << "---------\n";
	x_map.emplace_at("abcdefghijklmnopqrstuvwxyz", 4);
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
	auto p(x_map.find("abcd"));
	printf("x1 %p, %d\n", p, p ? *p : 0);
	p = x_map.find("abcdefghijkl");
	printf("x2 %p, %d\n", p, p ? *p : 0);
	p = x_map.find("abcdefghijklmnopqrst");
	printf("x3 %p, %d\n", p, p ? *p : 0);
	p = x_map.find("abcdefghijklmnopqrstuvwxyz");
	printf("x4 %p, %d\n", p, p ? *p : 0);

	auto r_idx(x_map.make_index());

	r_idx.for_each([](std::string const &key, int val) -> void {
		printf("key |%s| val %d\n", key.c_str(), val);
	});

	r_idx.for_each_prefix(
		std::string("abcdefghijklmnopqrstu"),
		[](std::string const &key, int val) -> void {
			printf("key_p1 |%s| val %d\n", key.c_str(), val);
		}
	);
	r_idx.for_each_prefix(
		std::string("abcdefghijkl"),
		[](std::string const &key, int val) -> void {
			printf("key_p2 |%s| val %d\n", key.c_str(), val);
		}
	);
	r_idx.for_each_prefix(
		std::string("abc"),
		[](std::string const &key, int val) -> void {
			printf("key_p3 |%s| val %d\n", key.c_str(), val);
		}
	);

	return 0;
}
