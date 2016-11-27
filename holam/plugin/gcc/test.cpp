#include <cstdio>

template<typename... Args>
int abbap(Args&&... args)
{
	return 0;
}

int main(int argc, char **argv)
{
	printf("aaaaa! %d\n", abbap(23, 45, 3.7, "aaa"));
	return 0;
}
