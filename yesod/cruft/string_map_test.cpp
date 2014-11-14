#include <iostream>
#include <yesod/string_map.hpp>

using ucpf::yesod::string_map;

int main(int argc, char **argv)
{
	std::string s_in;
	string_map<char, int> x_map;
	int ord(0);
#if 0
	printf("--1-----\n");
	x_map.emplace(std::string("/.git/refs/heads/master"), 111);
	x_map.emplace(std::string("/.git/refs/heads"), 222);
	x_map.emplace(std::string("/.git/refs/tags/REL8_4_22"), 333);
	x_map.emplace(std::string("/.git/refs/tags"), 444);
	x_map.emplace(std::string("/.git/refs/remotes/origin/HEAD"), 555);
	x_map.emplace(std::string("/.git/refs/remotes/origin"), 666);
	printf("--2-----\n");
	x_map.dump(std::cout);
	x_map.emplace(std::string("/.git/refs/remotes"), 777);
	x_map.emplace(std::string("/.git/refs"), 888);
	printf("--3-----\n");
	x_map.dump(std::cout);

	x_map.for_each(
		[](char *first, char *last, int val) -> bool {
			std::string key(first, last);
			printf("x_key %s, val %d\n", key.c_str(), val);
			return false;
		}
	);
	printf("----------\n");
	x_map.dump_internal(std::cout);
#endif
#if 1
	while (std::getline(std::cin, s_in)) {
		x_map.emplace(s_in, ord);
		++ord;
		s_in.clear();
	}

	auto r0(x_map.search_root());
	auto r1(x_map.locate_rel(r0, std::string("/contrib")));
	auto r2(x_map.locate_rel(r1, std::string("/btree_gin")));
	auto r3(x_map.locate_rel(r2, std::string("/expected")));
	auto v(x_map.find_rel(r3, std::string("/money.out")));
	printf("val %p, %d\n", v, v ? *v : -1);
/*
	x_map.dump(std::cout);

	x_map.erase_prefix(std::string("/src/tutorial"));

	printf("=========\n");
	x_map.for_each([](char *first, char *last, int val) -> bool {
		std::string key(first, last);
		printf("%s\n", key.c_str());
		return false;
	});

	r_idx.for_each_prefix(
		std::string("abcdefghijklmnopqrstu"),
		[](std::string const &key, int val) -> void {
			printf("key_p1 |%s| val %d\n", key.c_str(), val);
		}
	);

	x_map.dump(std::cerr);
*/
#endif

	return 0;
}
