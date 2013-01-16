#include <rasham/validating_test_stream.hpp>
#include <iostream>

int main(int argc, char **argv)
{
	rasham::validating_test_cstream os("p1.txt", "l1.txt");

	for (int cnt = 0; cnt < 24; ++cnt)
		os << "aaaa" << "bbbb" << "cccc\n";

	std::cout << "dump: " << os.rdbuf()->get_rope().dump() << std::endl;
	std::cout << "match: " << os.match_pattern() << std::endl;
	return 0;
}
