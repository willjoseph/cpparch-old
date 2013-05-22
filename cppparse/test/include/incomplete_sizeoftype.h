#ifndef INCLUDED_TEST_INCOMPLETE_SIZEOFTYPE_H
#define INCLUDED_TEST_INCOMPLETE_SIZEOFTYPE_H

struct S;
namespace SizeOfType
{
	int size = sizeof(S);
}

#endif
