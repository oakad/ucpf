#include <rasham/rope_stream.hpp>
#include <iostream>

int main(int argc, char **argv)
{
	rasham::crope_ostream os;
	for (int cnt = 0; cnt < 24; ++cnt)
		os << "aaaa" << "bbbb" << "cccc\n";

	std::cout << "c1\n";
	std::cout << os.get_rope();
	std::cout << "c2\n";
	std::cout << os.get_rope().dump();
	return 0;
}
