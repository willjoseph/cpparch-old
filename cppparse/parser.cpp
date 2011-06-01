
#include "parser.h"

#include "parser/symbols.h"

#if 0

template<void skipFunc(Parser&)>
struct ScopedSkip
{
	Parser& parser;
	const Token* position;
	BacktrackBuffer buffer;
	ScopedSkip(Parser& parser)
		: parser(parser), position(0)
	{
		const Token* first = parser.lexer.position;
		skipFunc(parser);
		size_t count = ::distance(parser.lexer.history, first, parser.lexer.position);
		if(count != 0)
		{
			printPosition(first->position);
			std::cout << "skipped: " << count << std::endl;
			buffer.resize(count + 2); // adding 1 for EOF and 1 to allow use as circular buffer
			for(const Token* p = first; p != parser.lexer.position; p = ::next(parser.lexer.history, p))
			{
				*buffer.position++ = *first++;
			}
			FilePosition nullPos = { "$null.cpp", 0, 0 };
			*buffer.position++ = Token(boost::wave::T_EOF, "", nullPos, "");

			parser.position -= count;
			parser.lexer.backtrack(count);
			position = parser.lexer.position;
			parser.lexer.history.swap(buffer);
			parser.lexer.position = parser.lexer.history.tokens;
		}
	}
	~ScopedSkip()
	{
		if(position != 0)
		{
			size_t count = (parser.lexer.position - parser.lexer.history.tokens);
			parser.lexer.history.swap(buffer);
			parser.lexer.position = position;
			parser.lexer.advance(count);
		}
	}
};

typedef std::list< DeferredParse<struct ContextDefer, struct ContextDefer> > DeferredParseList;

struct ContextDefer : public ContextBase
{
	typedef ContextDefer State;
	typedef ContextDefer Base;

	DeferredParseList* deferred;

	ContextDefer()
		: deferred(0)
	{
	}

	void hit(Base& other)
	{
	}
};

struct ContextTest
{
	struct ScopeGuard
	{
		ScopeGuard(ContextDefer& base)
		{
		}
		void hit()
		{
		}
	};

	struct DefaultContext : public ContextDefer
	{
		PARSERCONTEXT_DEFAULT;
		DefaultContext(const ContextDefer& base)
			: ContextDefer(base)
		{
		}
		void visit(cpp::class_specifier* symbol)
		{
			ClassSpecifierContext walker(*this);
			SYMBOL_WALK(walker, symbol);
			parseDeferred(walker.deferred, *this);
		}
	};
	struct ClassSpecifierContext : public ContextDefer
	{
		PARSERCONTEXT_DEFAULT;
		DeferredParseList deferred;
		ClassSpecifierContext(const ContextDefer& base)
			: ContextDefer(base)
		{
			if(ContextDefer::deferred == 0) // first class-specifier takes priority
			{
				ContextDefer::deferred = &deferred;
			}
		}
		void visit(cpp::member_declaration_named* symbol)
		{
			MemberDeclarationContext walker(*this);
			SYMBOL_WALK(walker, symbol);
			deferred.splice(deferred.end(), walker.deferred);
		}
		void visit(cpp::function_definition* symbol)
		{
			MemberDeclarationContext walker(*this);
			SYMBOL_WALK(walker, symbol);
			deferred.splice(deferred.end(), walker.deferred);
		}
	};
	struct MemberDeclarationContext : public ContextDefer
	{
		PARSERCONTEXT_DEFAULT;
		DeferredParseList deferred;
		MemberDeclarationContext(const ContextDefer& base)
			: ContextDefer(base)
		{
			ContextDefer::deferred = &deferred;
		}
		void visit(cpp::function_body* symbol)
		{
			FunctionBodyContext walker(*this);
			SYMBOL_WALK(walker, symbol);
		}
		struct DeclareEts
		{
			ContextBase& context;
			DeclareEts(ContextBase& context) : context(context)
			{
			}
			void operator()(cpp::terminal_identifier& id) const
			{
				// todo: record ETS declaration
			}
		};
		void visit(cpp::parameter_declaration_clause* symbol)
		{
#if 0
			skipParenthesised(*parser);
			result = symbol;
#elif 1
			DefaultContext walker(*this);
			defer(*ContextDefer::deferred, walker, makeSkipParenthesised(DeclareEts(walker)), symbol);
			result = symbol; // always succeeds!
#else
			ScopedSkip<skipParenthesised> skip(*parser);
			DefaultContext walker(*this);
			SYMBOL_WALK(walker, symbol);
#endif
		}
		void visit(cpp::mem_initializer_clause* symbol)
		{
#if 0
			skipMemInitializerClause(*parser);
			result = symbol;
#elif 1
			DefaultContext walker(*this);
			result = defer(*ContextDefer::deferred, walker, skipMemInitializerClause, symbol);
#else
			ScopedSkip<skipMemInitializerClause> skip(*parser);
			DefaultContext walker(*this);
			SYMBOL_WALK(walker, symbol);
#endif
		}
	};

	struct FunctionBodyContext : public ContextDefer
	{
		PARSERCONTEXT_DEFAULT;
		FunctionBodyContext(const ContextDefer& base)
			: ContextDefer(base)
		{
		}
		void visit(cpp::statement_seq* symbol)
		{
#if 0
			skipBraced(*parser);
			result = symbol;
#elif 1
			DefaultContext walker(*this);
			result = defer(*ContextDefer::deferred, walker, skipBraced, symbol);
#else
			ScopedSkip<skipBraced> skip(*parser);
			DefaultContext walker(*this);
			SYMBOL_WALK(walker, symbol);
#endif
		}
	};
};

struct ContextN
{
	struct ScopeGuard
	{
		ScopeGuard(ContextBase& base)
		{
		}
		void hit()
		{
		}
	};

	struct Context1 : public ContextBase
	{
		PARSERCONTEXT_DEFAULT;
		Context1(const ContextBase& base)
			: ContextBase(base)
		{
		}
		void visit(cpp::declaration* symbol)
		{
			Context2 walker(*this);
			SYMBOL_WALK(walker, symbol);
		}
	};

	struct Context2 : public ContextBase
	{
		PARSERCONTEXT_DEFAULT;
		Context2(const ContextBase& base)
			: ContextBase(base)
		{
		}
		void visit(cpp::declaration* symbol)
		{
			Context1 walker(*this);
			SYMBOL_WALK(walker, symbol);
		}
	};
};


#endif