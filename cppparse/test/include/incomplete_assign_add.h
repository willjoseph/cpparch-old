#ifndef INCLUDED_TEST_INCOMPLETE_ASSIGN_ADD_H
#define INCLUDED_TEST_INCOMPLETE_ASSIGN_ADD_H

struct S;
namespace AssignAdd
{
	void f()
	{
		S* p;

		// [expr.ass] In += and -= [the left operand] shall have either arithmetic type or be a pointer to a possibly cv-qualified completely defined object type.
		p += 1;
	}
}

#endif
