#ifndef INCLUDED_TEST_INCOMPLETE_PTR_ADD_H
#define INCLUDED_TEST_INCOMPLETE_PTR_ADD_H

struct S;
namespace PtrAdd
{
	void f()
	{
		S* p;

		// [expr.add] the left operand is a pointer to a completely defined object type.
		S* q = p + 1;
	}
}

#endif
