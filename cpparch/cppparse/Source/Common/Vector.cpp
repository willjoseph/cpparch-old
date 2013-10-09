
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

#if 0 // experimental
#include <set>

typedef std::vector<int> Vector;
typedef std::set<Vector> UniqueVectors;

template<typename A>
UniqueVectors::iterator insert(UniqueVectors& elements, const std::vector<int, A>& value)
{
	UniqueVectors::iterator i = elements.lower_bound(value); // first element not less than value
	if(i != elements.end()
		&& !elements.key_comp()(value, *i)) // if value is not less than lower bound
	{
		// lower bound is equal to value
		return i;
	}
	i = elements.insert(i, Vector());
	(*i).insert(value.begin(), value.end()); // copy the value
	return i;
}

void test2()
{
	typedef std::vector<int, DebugAllocator<int> > Vector2;
	Vector2 tmp;
	tmp.reserve(2);
	tmp.push_back(0);
	tmp.push_back(1);

	UniqueVectors elements;
#if 0
	UniqueVectors::iterator i = elements.insert(tmp).first;
#else
	UniqueVectors::iterator i = insert(elements, tmp);
#endif
}
#endif

