#include <rasham/rope.hpp>
#include <iostream>

int main(int argc, char **argv)
{
	using namespace rasham;

	crope a("aaaa");
	crope b("bbbb");

	crope c(a + b);
	std::cout << c << std::endl;
	std::cout << c.dump() << std::endl;
}
