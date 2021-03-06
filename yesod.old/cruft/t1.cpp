#include <array>
#include <tuple>
#include <bitset>
#include <iostream>
#include <yesod/rope.hpp>

using ucpf::yesod::crope;

struct alignas(16) xx {
	int a;
	int b;
};

struct alignas(8) yy {
	int a;
	int b;
};

int main()
{
	char const *xa = "aaaaa";
	std::cout << std::distance(std::begin("aaaaa"), std::end("aaaaa")) << '\n';
	crope aa(std::string("aaaaa"));
	crope bb(std::string("bbbbb"));
	crope cc(aa + bb);
	std::cout << aa.dump<char>() << '\n';
	std::cout << bb.dump<char>() << '\n';
	std::cout << cc.dump<char>() << '\n';
	printf("xx %zd - %zd\n", sizeof(xx), sizeof(yy));
	return 0;
}
