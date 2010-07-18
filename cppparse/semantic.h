
#ifndef INCLUDED_CPPPARSE_SEMANTIC_H
#define INCLUDED_CPPPARSE_SEMANTIC_H

namespace cpp
{
	struct declaration_seq;
	struct statement_seq;
};

struct IncludeDependencyGraph;

struct PrintSymbolArgs
{
	const char* path;
	const IncludeDependencyGraph& includeGraph;
};

void printSymbol(cpp::declaration_seq* p, const PrintSymbolArgs& args);
void printSymbol(cpp::statement_seq* p, const PrintSymbolArgs& args);


#endif


