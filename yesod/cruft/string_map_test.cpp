#include <iostream>
#include <yesod/string_map.hpp>

using ucpf::yesod::string_map;

int main(int argc, char **argv)
{
	std::string s_in;
	string_map<char, int> x_map;
	int ord(0);

	printf("--1-----\n");
	x_map.emplace(std::string("bachelor"), 111);
	x_map.emplace(std::string("jar"), 222);
	x_map.emplace(std::string("badge"), 333);
	printf("--2-----\n");
	x_map.dump(std::cout);
	x_map.emplace(std::string("baby"), 444);
	printf("--3-----\n");
	x_map.dump(std::cout);
	x_map.emplace(std::string("bache"), 555);
	printf("--4-----\n");
	x_map.dump(std::cout);

	printf("1: %d\n", *x_map.find(std::string("bachelor")));
	printf("2: %d\n", *x_map.find(std::string("jar")));
	printf("3: %d\n", *x_map.find(std::string("badge")));
	printf("4: %d\n", *x_map.find(std::string("baby")));
	printf("5: %d\n", *x_map.find(std::string("bache")));

	x_map.for_each(
		std::string("bachel"),
		[](char *first, char *last, int val) -> bool {
			std::string key(first, last);
			printf("x_key %s, val %d\n", key.c_str(), val);
			return false;
		}
	);

/*
	
	x_map.dump(std::cout);

	printf("----\n");
	x_map.dump(std::cout);

	for (auto c(getchar()); c != EOF; c = getchar()) {
		if (c == '\n' || c == '\r') {
			if (!s_in.empty()) {
				printf("in %s\n", s_in.c_str());
				x_map.emplace(s_in, ord);
				++ord;
				s_in.clear();
			}
		} else
			s_in.push_back(c);
	}
*/
	//x_map.dump(std::cout);

	//auto r_idx(x_map.make_index());

	//r_idx.for_each([](std::string const &key, int val) -> void {
	//	printf("%s %d\n", key.c_str(), val);
	//});
/*
	r_idx.for_each_prefix(
		std::string("abcdefghijklmnopqrstu"),
		[](std::string const &key, int val) -> void {
			printf("key_p1 |%s| val %d\n", key.c_str(), val);
		}
	);
*/

	return 0;
}
