#include <rasham/rasham.hpp>
#include <rasham/sink.hpp>
#include <unistd.h>

void xtry()
{
	for (int cnt = 0; cnt < 5; ++cnt)
		rasham_print("/info/c4", "Test xtry %d: %f\n", cnt, 4.0);
}

int main(int argc, char **argv)
{
	int repeat(0);
	rasham::fd_sink o_sink(STDOUT_FILENO, false);
	rasham::bind_sink("/", &o_sink);
again:
	if (repeat > 2)
		return 0;

	rasham_print("/debug/c1", "Test me %d\n", 5);
	for (int cnt = 0; cnt < 5; ++cnt) {
		rasham_print("/info/c2", "Test again %d: %f\n", cnt, 3.0);
	}
	xtry();
	++repeat;
goto again;

	return 0;
}
