#ifndef INCLUDED_TEST_INCOMPLETE_BASE_H
#define INCLUDED_TEST_INCOMPLETE_BASE_H

struct S;
struct U;
namespace BaseSpec
{
	// [class.derived] The class-name in a base-specifier shall not be an incompletely defined class.
	struct R : S
	{
	};
}

#endif
