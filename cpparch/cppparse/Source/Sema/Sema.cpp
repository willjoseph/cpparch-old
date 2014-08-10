

#include "Sema.h"
#include "Common/Util.h"
#include "Common/Profile.h"
#include "Lex/Token.h"
#include "Parse/Grammar.h"
#include "Parse/Parse.h"
#include "Parse/ParseSpecial.h"
#include "SemaClass.h"
#include "SemaDeclaration.h"
#include "SemaDeclarator.h"
#include "SemaDeclaratorFunction.h"
#include "SemaEnum.h"
#include "SemaException.h"
#include "SemaExpression.h"
#include "SemaIdExpression.h"
#include "SemaNamespace.h"
#include "SemaNamespaceName.h"
#include "SemaNestedNameSpecifier.h"
#include "SemaOperator.h"
#include "SemaPostfixExpression.h"
#include "SemaPrimaryExpression.h"
#include "SemaSpecifier.h"
#include "SemaStatement.h"
#include "SemaTemplateDeclaration.h"
#include "SemaTemplateName.h"
#include "SemaTypeId.h"
#include "SemaTypeName.h"
#include "SemaTypeSpecifier.h"

#include <iostream>



// ----------------------------------------------------------------------------

// Wrapper for a reference to a newly pushed semantic context constructed with no arguments.
template<typename SemaT, typename Inner, typename Commit>
struct SemaInner<SemaT, SemaPush<Inner, Commit, Args0> >
{
	typedef Inner Type;
	Inner sema;
	SemaInner(SemaT& walker, const SemaPush<Inner, Commit, Args0>& args)
		: sema(walker.getState())
	{
	}
	Inner& get()
	{
		return sema;
	}
};

// Wrapper for a reference to a newly pushed semantic context constructed with one argument.
template<typename SemaT, typename Inner, typename Commit, typename A1>
struct SemaInner<SemaT, SemaPush<Inner, Commit, Args1<A1> > >
{
	typedef Inner Type;
	Inner sema;
	SemaInner(SemaT& walker, const SemaPush<Inner, Commit, Args1<A1> >& args)
		: sema(walker.getState(), args.a1)
	{
	}
	Inner& get()
	{
		return sema;
	}
};

// Wrapper for a reference to a newly pushed semantic context constructed with two arguments.
template<typename SemaT, typename Inner, typename Commit, typename A1, typename A2>
struct SemaInner<SemaT, SemaPush<Inner, Commit, Args2<A1, A2> > >
{
	typedef Inner Type;
	Inner sema;
	SemaInner(SemaT& walker, const SemaPush<Inner, Commit, Args2<A1, A2> >& args)
		: sema(walker.getState(), args.a1, args.a2)
	{
	}
	Inner& get()
	{
		return sema;
	}
};

// Wrapper for a reference to the current semantic context.
template<typename SemaT>
struct SemaInner<SemaT, SemaIdentity>
{
	typedef SemaT Type;
	SemaT& sema;
	SemaInner(SemaT& walker, const SemaIdentity&) : sema(walker)
	{
	}
	SemaT& get()
	{
		return sema;
	}
};

// ----------------------------------------------------------------------------


AstAllocator<int> getAllocator(ParserContext& context)
{
#ifdef AST_ALLOCATOR_LINEAR
	return context.allocator;
#else
	return DebugAllocator<int>();
#endif
}


ParseTree* parseFile(ParserContext& context)
{
	gUniqueNames.clear();
	gUniqueTypes.clear();
	gUniqueExpressions.clear();

	SemaContext& globals = *new SemaContext(context, getAllocator(context));
	SemaNamespace& walker = *new SemaNamespace(globals);
	ParserGeneric<SemaNamespace> parser(context, walker);

	cpp::symbol_sequence<cpp::declaration_seq> result(NULL);
	try
	{
		ProfileScope profile(gProfileParser);
		PROFILESCOPE_ENABLECOLLECTION(profile2);
		PARSE_SEQUENCE(parser, result);
	}
	catch(ParseError&)
	{
	}
	catch(SemanticError&)
	{
		printPosition(parser.context.getErrorPosition());
		std::cout << "caught SemanticError" << std::endl;
		throw;
	}
	catch(SymbolsError&)
	{
		printPosition(parser.context.getErrorPosition());
		std::cout << "caught SymbolsError" << std::endl;
		throw;
	}
	catch(TypeError& e)
	{
		e.report();
	}
	if(!context.finished())
	{
		printError(parser);
	}
	dumpProfile(gProfileIo);
	dumpProfile(gProfileWave);
	dumpProfile(gProfileParser);
	dumpProfile(gProfileLookup);
	dumpProfile(gProfileDiagnose);
	dumpProfile(gProfileAllocator);
	dumpProfile(gProfileIdentifier);
	dumpProfile(gProfileTemplateId);

	return result.get();
}


