
#ifndef INCLUDED_CPPPARSE_SEMANTIC_H
#define INCLUDED_CPPPARSE_SEMANTIC_H

namespace cpp
{
	struct declaration_seq;
	struct statement_seq;
};

struct ParserContext;

cpp::declaration_seq* parseFile(ParserContext& lexer);
cpp::statement_seq* parseFunction(ParserContext& lexer);


#endif


