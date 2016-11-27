#include <yesod/rope.hpp>
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

	ucpf::yesod::crope pattern =  ucpf::yesod::crope_file_reader(argv[1]);

	ucpf::yesod::crope t1(pattern.cbegin(), pattern.cbegin() + 20);
	ucpf::yesod::crope t2(pattern.cbegin() + 20, pattern.cend() - 20);
	ucpf::yesod::crope t3(pattern.cend() - 20, pattern.cend());
	printf("\nx1\n");
	std::cout << t2;
	printf("\nx2\n");
	std::cerr << "\n ------ \n" << t2.dump<char>() << std::endl;
//	std::cout << "pat ======\n" << pattern << '\n';
//	std::cout << "t1 ======\n" << t1 << '\n';
//	std::cout << "t2 ======\n" << t2 << '\n';
//	std::cout << "t3 ======\n" << t3 << '\n';

	ucpf::yesod::crope t(t1 + t2 + t3);
	printf("\nc1\n");
	std::cout << t;
	printf("\nc2\n");
	std::cerr << "\n ------ \n" << t.dump<char>() << std::endl;
	return 0;
}
