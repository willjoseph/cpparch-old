
#ifndef INCLUDED_CPPPARSE_AST_ASTALLOCATOR_H
#define INCLUDED_CPPPARSE_AST_ASTALLOCATOR_H

#include "Parse/Parse.h"

// ----------------------------------------------------------------------------
// Allocator

#define AST_ALLOCATOR_LINEAR

#ifdef AST_ALLOCATOR_LINEAR
#define AstAllocator ParserAllocatorWrapper
#define AST_ALLOCATOR_NULL AstAllocator<int>(NullParserAllocator())
#else
#define AstAllocator DebugAllocator
#define AST_ALLOCATOR_NULL AstAllocator<int>()
#endif


#endif
