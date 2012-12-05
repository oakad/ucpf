#include <pardes/internal/restricted_array.hpp>
#include <stdio.h>

struct x_type {
	x_type() : a(-1)
	{
		printf("construct %p empty\n", this);
	}

	x_type(int a_) : a(a_)
	{
		printf("construct %p, %d\n", this, a);
	}

	x_type(x_type &&v)
	{
		a = v.a;
		printf("move construct %p, %d\n", this, a);
	}

	x_type &operator=(x_type const &v)
	{
		a = v.a;
		printf("copy assign %p, %d\n", this, a);
		return *this;
	}

	x_type &operator=(x_type &&v)
	{
		a = v.a;
		printf("move assign %p, %d\n", this, a);
		return *this;
	}

	x_type(x_type const &v)
	{
		a = v.a;
		printf("copy construct %p, %d\n", this, a);
	}

	virtual ~x_type()
	{
		printf("destruct %p, %d\n", this, a);
	}

	int a;
};

bool operator<(x_type const &x, x_type const &y)
{
	return x.a < y.a;
}

int main(int argc, char **argv)
{
	pardes::restricted_array<x_type, 10> b({8, 2, 6, 3, 9, 0});

	b.insert(b.cend(), 12);
	b.emplace(b.cbegin() + 3, 8);

	std::for_each(
		b.begin(), b.end(), [](x_type const &v) -> void {
			printf("value 1: %d\n", v.a);
		}
	);

	printf("erase 1: %d\n", b.erase(b.cbegin() + 2, b.cend() - 2)->a);
	std::for_each(
		b.cbegin(), b.cend(), [](x_type const &v) -> void {
			printf("value 2: %d\n", v.a);
		}
	);

	auto p(b.insert(b.cbegin() + 1, {13, 15, 17, 19}));
	printf("insert 2: %d\n", p->a);

	std::for_each(
		b.cbegin(), b.cend(), [](x_type const &v) -> void {
			printf("value 3: %d\n", v.a);
		}
	);
	return 0;
}
