
void f(void (*pf(int a)) (float hidden))
{
}

#if 0
void f(int(*(int))[1][1])
{
}
#else
void f(int((*((int)))[]))
{
}
#endif

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
