#include <stdio.h>

struct incfile {
	const char *data;
	size_t size;
} bindata = {
#include "bindata.h"
};

int main(int argc, char **argv)
{
	size_t cnt;

	for (cnt = 0; cnt < bindata.size; ++cnt)
		fputc(bindata.data[cnt], stdout);

	return 0;
}