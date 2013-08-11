#ifndef INCLUDED_TEST_INCOMPLETE_PTR_DIFF_H
#define INCLUDED_TEST_INCOMPLETE_PTR_DIFF_H

struct S;
namespace PtrDiff
{
	void f()
	{
		S* p;

		// [expr.add] both operands are pointers to the same completely defined object type.
		int d = p - p;
	}
}

#endif
