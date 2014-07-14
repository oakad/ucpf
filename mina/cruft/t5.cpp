#include <cstdio>
#include <cstdint>

int main(void)
{
	uint16_t x(0x0fff);
	uint16_t y(0xf000);
	uint16_t c(1);
	uint16_t z(x + y + c);
	uint16_t a(((x & y) | ((x | y) & ~(x + y + c))) >> 15);
	printf("aa a: %hx, z: %hx\n", a, z);
	return 0;
}
