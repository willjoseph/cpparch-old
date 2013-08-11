#ifndef INCLUDED_TEST_INCOMPLETE_POST_INCR_H
#define INCLUDED_TEST_INCOMPLETE_POST_INCR_H

struct S;
namespace PostIncr
{
	void f()
	{
		S* p;

		// [expr.post.incr] The type of the operand shall be an arithmetic type or a pointer to a complete object type.
		p--;
	}
}

#endif
