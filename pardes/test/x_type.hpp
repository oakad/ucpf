#if !defined(_X_TYPE_HPP)
#define _X_TYPE_HPP

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

bool operator==(x_type const &x, x_type const &y)
{
	return x.a == y.a;
}

#endif
