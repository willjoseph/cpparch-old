
#ifndef INCLUDED_CPPPARSE_SEMA_SEMA_H
#define INCLUDED_CPPPARSE_SEMA_SEMA_H

template<typename T>
struct Visitable;

namespace cpp
{
	struct declaration_seq;
};

typedef Visitable<cpp::declaration_seq> ParseTree;

struct ParserContext;

ParseTree* parseFile(ParserContext& lexer);


#endif


