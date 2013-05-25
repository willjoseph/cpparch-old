#ifndef INCLUDED_TEST_INCOMPLETE_SIZEOFTYPE_H
#define INCLUDED_TEST_INCOMPLETE_SIZEOFTYPE_H

struct S;
struct U;
namespace SizeOfType
{
	int size = sizeof(S);

	int size2 = sizeof(U*);
}

#endif
