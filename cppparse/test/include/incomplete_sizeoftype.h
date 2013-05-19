#ifndef INCLUDED_TEST_INCOMPLETE_SIZEOFTYPE_H
#define INCLUDED_TEST_INCOMPLETE_SIZEOFTYPE_H

struct S;
namespace SizeOfType
{
	size_t size = sizeof(S);
}

#endif
