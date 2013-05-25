#ifndef INCLUDED_TEST_INCOMPLETE_CAST_H
#define INCLUDED_TEST_INCOMPLETE_CAST_H

struct S;
struct U;
namespace Cast
{
	// [basic.lval] An expression which holds a temporary object resulting from a cast to a non-reference type is an rvalue
	const S& s1 = S(0); // function-style-cast creates a temporary
	const S& s2 = (S)0; // C-style-cast creates a temporary
	const S& s3 = static_cast<S>(0); // static-cast creates a temporary

	const U& u2 = *(U*)0;
	const U& u3 = *static_cast<U*>(0);
}

#endif
