
typedef int* A[1]; // array of pointer to

void f(A(*x)[1]) // pointer to array of
{
}
void f(int*(*x)[1][1])// pointer to array of array of pointer to
{
}

void f(int*(*x)[1]);

typedef void (*Pf) (float hidden);

void f(Pf pf(int a));


void f(void (*pf(int a)) (float hidden))
{
}

void f(int*) // pointer to
{
}
void f(int**) // pointer to pointer to
{
}
void f(int*()) // function returning pointer to
{
}
void f(int**()) // function returning pointer to pointer to
{
}
void f(int*[]) // array of pointer to
{
}
void f(int(*)()) // pointer to function returning
{
}
void f(int*(*)()) // pointer to function returning pointer to
{
}
void f(int(*)[]) // pointer to array of
{
}

void f(int(*())[]) // function returning pointer to array of
{
}
void f(int(*[])()) // array of pointer to function returning
{
}

struct S
{
};

void f(int(S::*())[]) // function returning member-pointer to array of
{
}
void f(int(S::*[])()) // array of member-pointer to function returning
{
}

// not allowed:
// *[](): array of function returning pointer to
// (*)[](): pointer to array of function returning
// *()[]: function returning array of pointer to
// (*)()[]: pointer to function returning array of


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
