
template<typename T>
struct C99
{
	template<class _Other>
	operator C99<_Other>()
	{
		return (C99<_Other>(*this));
	}
};

int f(void (*)(void));

void f(int(*())[1][1])
{
}

void f(void (*pf(int a)) (int hidden))
{
}


void overloaded(double)
{
}

void overloaded(int)
{
}

void f()
{
	overloaded(1 * 3 * (9 * 8.2));
}
