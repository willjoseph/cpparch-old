
#ifndef INCLUDED_CPPPARSE_REPORT_REPORT_H
#define INCLUDED_CPPPARSE_REPORT_REPORT_H

template<typename T>
struct Visitable;

namespace cpp
{
	struct declaration_seq;
};

typedef Visitable<cpp::declaration_seq> ParseTree;

struct IncludeDependencyGraph;

struct PrintSymbolArgs
{
	const char* inputRoot;
	const char* outputRoot;
	const IncludeDependencyGraph& includeGraph;
	PrintSymbolArgs(const char* inputRoot, const char* outputRoot, const IncludeDependencyGraph& includeGraph)
		: inputRoot(inputRoot), outputRoot(outputRoot), includeGraph(includeGraph)
	{
	}
};

void printSymbol(ParseTree* p, const PrintSymbolArgs& args);

#endif


