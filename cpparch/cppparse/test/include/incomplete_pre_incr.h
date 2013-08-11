#ifndef INCLUDED_TEST_INCOMPLETE_PRE_INCR_H
#define INCLUDED_TEST_INCOMPLETE_PRE_INCR_H

struct S;
namespace PreIncr
{
	void f()
	{
		S* p;

		// [expr.pre.incr] The type of the operand shall be an arithmetic type or a pointer to a completely-defined object type.
		++p;
	}
}

#endif
