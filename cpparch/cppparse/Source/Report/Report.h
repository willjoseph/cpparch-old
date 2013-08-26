
#ifndef INCLUDED_CPPPARSE_REPORT_REPORT_H
#define INCLUDED_CPPPARSE_REPORT_REPORT_H

namespace cpp
{
	struct declaration_seq;
	struct statement_seq;
};

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

void printSymbol(cpp::declaration_seq* p, const PrintSymbolArgs& args);
void printSymbol(cpp::statement_seq* p, const PrintSymbolArgs& args);

#endif


