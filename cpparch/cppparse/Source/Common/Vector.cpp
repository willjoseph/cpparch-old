
#include "Vector.h"

#include <vector>

void test()
{
	typedef SharedVector<int, DebugAllocator<int> > Vector;

	Vector tmp;
	tmp.reserve(2);
	tmp.push_back(0);
	tmp.push_back(1);

	Vector tmp2 = tmp;
}

