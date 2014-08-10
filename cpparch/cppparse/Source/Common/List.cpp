
#include "List.h"

#include "Allocator.h"

struct TestList
{
	TestList()
	{
		typedef List<char, DebugAllocator<int> > IntList;

		IntList a;
		a.push_back('1');
		a.push_back('2');
		a.push_back('3');
		LIST_ASSERT(*a.begin() == '1');
		LIST_ASSERT(*++a.begin() == '2');
		LIST_ASSERT(*++++a.begin() == '3');
		LIST_ASSERT(++++++a.begin() == a.end());

		IntList b;
		b.push_back('a');
		b.push_back('b');
		b.push_back('c');
		LIST_ASSERT(*b.begin() == 'a');
		LIST_ASSERT(*++b.begin() == 'b');
		LIST_ASSERT(*++++b.begin() == 'c');
		LIST_ASSERT(++++++b.begin() == b.end());

		// swap full lists
		a.swap(b);
		LIST_ASSERT(*a.begin() == 'a');
		LIST_ASSERT(*++a.begin() == 'b');
		LIST_ASSERT(*++++a.begin() == 'c');
		LIST_ASSERT(++++++a.begin() == a.end());
		LIST_ASSERT(*b.begin() == '1');
		LIST_ASSERT(*++b.begin() == '2');
		LIST_ASSERT(*++++b.begin() == '3');
		LIST_ASSERT(++++++b.begin() == b.end());

		// swap to/from empty list
		IntList c;
		c.swap(b);
		LIST_ASSERT(b.empty());
		LIST_ASSERT(*c.begin() == '1');
		LIST_ASSERT(*++c.begin() == '2');
		LIST_ASSERT(*++++c.begin() == '3');
		LIST_ASSERT(++++++c.begin() == c.end());

		c.swap(b);
		LIST_ASSERT(c.empty());
		LIST_ASSERT(*b.begin() == '1');
		LIST_ASSERT(*++b.begin() == '2');
		LIST_ASSERT(*++++b.begin() == '3');
		LIST_ASSERT(++++++b.begin() == b.end());

		IntList d;
		c.swap(d);
		LIST_ASSERT(c.empty());
		LIST_ASSERT(d.empty());

		c.splice(c.end(), d);
		LIST_ASSERT(c.empty());
		LIST_ASSERT(d.empty());

		// splice full with empty
		b.splice(b.end(), c);
		LIST_ASSERT(c.empty());
		LIST_ASSERT(*b.begin() == '1');
		LIST_ASSERT(*++b.begin() == '2');
		LIST_ASSERT(*++++b.begin() == '3');
		LIST_ASSERT(++++++b.begin() == b.end());

		// splice empty with full
		c.splice(c.end(), b);
		LIST_ASSERT(b.empty());
		LIST_ASSERT(*c.begin() == '1');
		LIST_ASSERT(*++c.begin() == '2');
		LIST_ASSERT(*++++c.begin() == '3');
		LIST_ASSERT(++++++c.begin() == c.end());

		// splice full with full
		c.splice(c.end(), a);
		LIST_ASSERT(a.empty());
		LIST_ASSERT(*c.begin() == '1');
		LIST_ASSERT(*++c.begin() == '2');
		LIST_ASSERT(*++++c.begin() == '3');
		LIST_ASSERT(*++++++c.begin() == 'a');
		LIST_ASSERT(*++++++++c.begin() == 'b');
		LIST_ASSERT(*++++++++++c.begin() == 'c');
		LIST_ASSERT(++++++++++++c.begin() == c.end());


		c.pop_back();
		LIST_ASSERT(!c.empty());
		c.pop_back();
		LIST_ASSERT(!c.empty());
		c.pop_back();
		LIST_ASSERT(!c.empty());
		c.pop_back();
		LIST_ASSERT(!c.empty());
		c.pop_back();
		LIST_ASSERT(!c.empty());
		c.pop_back();
		LIST_ASSERT(c.empty());
	}
}
#ifdef _DEBUG
gTestList
#endif
;
