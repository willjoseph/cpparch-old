#ifndef INCLUDED_TEST_INCOMPLETE_NEW_H
#define INCLUDED_TEST_INCOMPLETE_NEW_H

struct S;
struct U;
namespace New
{
	// [expr.new] The new expression attempts to create an object of the type-id or new-type-id to which it is applied. The type shall be a complete type...
	S* x = new S;
	S* y = new S[1];

	U** z = new U*;
}

#endif
