#include <rasham/rope.hpp>
#include <iostream>

int main(int argc, char **argv)
{
	if (argc < 2)
		return 0;

	/*char const tst[]
	= "0: abcdefghijklmnoprstuvwxyz\n"
	  "1: abcdefghijklmnoprstuvwxyz\n"
	  "2: abcdefghijklmnoprstuvwxyz\n"
	  "3: abcdefghijklmnoprstuvwxyz\n";*/

	rasham::crope pattern(std::move(rasham::crope_file_reader(argv[1])));

	rasham::crope t1(pattern.cbegin(), pattern.cbegin() + 20);
	rasham::crope t2(pattern.cbegin() + 20, pattern.cend() - 20);
	rasham::crope t3(pattern.cend() - 20, pattern.cend());
	rasham::crope t(t1 + t2 + t3);
	printf("c1\n");
	std::cout << t;
	printf("c2\n");
	std::cerr << "\n ------ \n" << t.dump() << std::endl;
	return 0;
}
