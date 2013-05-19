#ifndef INCLUDED_TEST_INCOMPLETE_SIZEOF_H
#define INCLUDED_TEST_INCOMPLETE_SIZEOF_H

struct S;
namespace SizeOf
{
	S* p;
	// [expr.sizeof] The sizeof operator shall not be applied to an expression that has function or incomplete type.
	size_t size = sizeof(*p);
}

#endif
