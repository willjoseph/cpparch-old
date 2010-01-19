
#ifndef INCLUDED_CPPPARSE_SEMANTIC_H
#define INCLUDED_CPPPARSE_SEMANTIC_H

namespace cpp
{
	struct declaration_seq;
	struct statement_seq;
};

void printSymbol(cpp::declaration_seq* p, const char* path);
void printSymbol(cpp::statement_seq* p, const char* path);


#endif


