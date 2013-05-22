#ifndef INCLUDED_TEST_INCOMPLETE_SIZEOF_H
#define INCLUDED_TEST_INCOMPLETE_SIZEOF_H

struct S;
namespace Sizeof
{
	S* p;
	// [expr.sizeof] The sizeof operator shall not be applied to an expression that has function or incomplete type.
	int size = sizeof(*p);
}

#endif
