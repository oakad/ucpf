#include <limits>
#include <cstdio>
#include <iostream>

int main(int argc, char **argv)
{
	double n(std::numeric_limits<double>::quiet_NaN());
	double i(std::numeric_limits<double>::infinity());

	printf("n %f, i %f\n", n, i);
	std::cout << "n " << n << ", i " << i << '\n';
	return 0;
}
