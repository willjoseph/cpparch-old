
#include "Test.h"


#if 1

namespace N
{
	template<typename T>
	class C
	{
		template<typename X>
		void f()
		{
			N::undeclared<X> x;
		}
	};
}

struct S
{
};

template<typename First, typename Second>
int f(First x, Second y);

int x = f(S(), int());

template<typename Second, typename First>
int f(Second a, First b)
{
	return b;
}

#endif

