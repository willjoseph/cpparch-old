
#ifndef INCLUDED_CPPPARSE_PARSER_H
#define INCLUDED_CPPPARSE_PARSER_H

#include "lexer.h"
#include "cpptree.h"

struct ParseError
{
	ParseError()
	{
	}
};

struct GeneralError
{
	GeneralError()
	{
	}
};

#define ASSERT(condition) if(!(condition)) { throw GeneralError(); }

inline bool isAlphabet(char c)
{
	return (c >= 'a' && c <= 'z')
		|| (c >= 'A' && c <= 'Z');
}

inline bool isNumber(char c)
{
	return (c >= '0' && c <= '9');
}

inline bool isIdentifier(char c)
{
	return isAlphabet(c) || isNumber(c) || c == '_';
}

inline void printSequence(Lexer& lexer)
{
	std::cout << "   ";
	bool space = false;
	bool endline = false;
	for( Lexer::Tokens::const_iterator i = lexer.position; i != lexer.history.end(); i = next(lexer.history, i))
	{
		if(space && isIdentifier(*(*i).value))
		{
			std::cout << " ";
		}
		std::cout << (*i).value;
		space = isIdentifier((*i).value[strlen((*i).value) - 1]);
	}
	std::cout << std::endl;
}

inline void printSequence(Lexer& lexer, size_t position)
{
	//position = std::min(std::min(lexer.history.size(), size_t(32)), position);
	//printSequence(lexer.position - position, lexer.position);
}

struct SymbolDebug
{
	bool disambiguated;
	SymbolDebug(bool disambiguated) : disambiguated(disambiguated)
	{
	}

	void visit(cpp::terminal_identifier symbol)
	{
		std::cout << symbol.value << " ";
	}

	void visit(cpp::terminal_string symbol)
	{
		std::cout << symbol.value << " ";
	}

	void visit(cpp::terminal_choice2 symbol)
	{
		std::cout << symbol.value << " ";
	}

	template<LexTokenId id>
	void visit(cpp::terminal<id> symbol)
	{
		if(symbol.value != 0)
		{
			std::cout << symbol.value << " ";
		}
	}

	template<typename T>
	void visit(T* symbol)
	{
		symbol->accept(*this);
	}

	template<typename T>
	void visit(cpp::symbol<T> symbol)
	{
		if(symbol.p != 0)
		{
			visit(symbol.p);
		}
	}

	void visit(cpp::template_argument_list* symbol)
	{
		if(disambiguated)
		{
			std::cout << "( ";
			symbol->accept(*this);
			std::cout << ") ";
		}
		else
		{
			symbol->accept(*this);
		}
	}
	void visit(cpp::relational_expression_default* symbol)
	{
		if(disambiguated)
		{
			std::cout << "( ";
			symbol->accept(*this);
			std::cout << ") ";
		}
		else
		{
			symbol->accept(*this);
		}
	}

	template<typename T>
	void visit(cpp::ambiguity<T>* symbol)
	{
		if(disambiguated)
		{
			std::cout << "[ ";
			symbol->accept(*this);
			std::cout << "] ";
		}
		else
		{
			symbol->accept(*this);
		}
	}
};

template<typename T>
void printSymbol(T* symbol, bool disambiguated = false)
{
	SymbolDebug walker(disambiguated);
	walker.visit(symbol);
}

//#define AMBIGUITY_DEBUG

struct TemplateIdAmbiguityContext
{
	size_t depth;
	size_t solution;
	size_t max;
	TemplateIdAmbiguityContext() : depth(0), solution(0), max(0)
	{
	}
	void nextDepth()
	{
		if(depth == 15) // limit to 2^16 permutations
		{
			throw ParseError();
		}
		++depth;
		max = std::max(max, depth);
	}
	bool ignoreTemplateId() const
	{
		return (solution & (1 << depth)) != 0;
	}
};

struct ParserState
{
	TemplateIdAmbiguityContext* ambiguity;
	bool inTemplateArgumentList;
	bool inGeneralAmbiguity; // for profiling!

	ParserState()
		: ambiguity(0), inTemplateArgumentList(false), inGeneralAmbiguity(false)
	{
	}
};

struct Parser : public ParserState
{
	Lexer& lexer;
	size_t position;
	size_t allocation;
	size_t ambiguityDepth;

	Parser(Lexer& lexer)
		: lexer(lexer), position(0), allocation(0), ambiguityDepth(0)
	{
	}
	Parser(const Parser& other)
		: ParserState(other), lexer(other.lexer), position(0), allocation(lexer.allocator.position), ambiguityDepth(0)
	{
	}

	LexTokenId get_id()
	{
		return lexer.get_id();
	}
	const char* get_value()
	{
		return lexer.get_value();
	}
	FilePosition get_position()
	{
		return lexer.get_position();
	}

	void increment()
	{
		++position;
		lexer.increment();
	}

	void backtrack(const char* symbol, bool preserveAllocation = false)
	{
		lexer.backtrack(position, symbol);
		if(!preserveAllocation)
		{
			lexer.allocator.backtrack(allocation);
		}
		if(ambiguity != 0
			&& ambiguityDepth != 0)
		{
			ASSERT(ambiguityDepth <= ambiguity->depth);
			ambiguity->depth -= ambiguityDepth;
#ifdef AMBIGUITY_DEBUG
			std::cout << "ambiguity backtrack: " << ambiguity->depth << "/" << ambiguityDepth << std::endl;
#endif
		}
	}
	void advance()
	{
		if(position != 0)
		{
			lexer.advance(position);
		}
		if(ambiguity != 0
			&& ambiguityDepth != 0)
		{
			ambiguity->depth += ambiguityDepth;
#ifdef AMBIGUITY_DEBUG
			std::cout << "ambiguity advance: " << ambiguity->depth << "/" << ambiguityDepth << std::endl;
#endif
		}
	}
	void nextDepth()
	{
		ambiguity->nextDepth();
		++ambiguityDepth;
#ifdef AMBIGUITY_DEBUG
		std::cout << "ambiguity next-depth: " << ambiguity->depth << "/" << ambiguityDepth << std::endl;
#endif
	}
};

inline void printError(Parser& parser)
{
#if 0
	for(Lexer::Positions::const_iterator i = parser.lexer.backtrace.begin(); i != parser.lexer.backtrace.end(); ++i)
	{
	}
	printPosition(parser.lexer, lexer.history[parser.lexer.stacktrace.back()].position);
#endif
	printPosition(get_position(dereference(parser.lexer.first)));
	std::cout << "syntax error: '" << get_value(dereference(parser.lexer.first)) << "'" << std::endl;
	printSequence(parser.lexer); // rejected tokens
}

inline void printSequence(Parser& parser)
{
	printSequence(parser.lexer, parser.position);
}

#define PARSE_ERROR() throw ParseError()
#define PARSE_ASSERT(condition) if(!(condition)) { PARSE_ERROR(); }

inline bool isToken(LexTokenId token, boost::wave::token_id id)
{
	return token == id;
}

inline bool isIdentifier(LexTokenId token)
{
	return isToken(token, boost::wave::T_IDENTIFIER);
}

inline void parseToken(Parser& parser, boost::wave::token_id id)
{
	PARSE_ASSERT(isToken(parser.get_id(), id));
	parser.increment();
};

template<typename T>
struct NullPtr
{
	static cpp::symbol<T> VALUE;
};

template<typename T>
cpp::symbol<T> NullPtr<T>::VALUE = cpp::symbol<T>(NULL);

template<typename T>
struct SymbolAllocator
{
	LinearAllocator& allocator;
	SymbolAllocator(LinearAllocator& allocator)
		: allocator(allocator)
	{
	}

	T* allocate(cpp::terminal_choice*)
	{
		return NULL;
	}
	T* allocate(cpp::choice<T>*)
	{
		return NULL;
	}
	T* allocate(void*)
	{
		return new(allocator.allocate(sizeof(T))) T;
	}
};

template<typename T>
T* createSymbol(Parser& parser, T*)
{
	return new(parser.lexer.allocator.allocate(sizeof(T))) T;
}

inline bool checkBacktrack(Parser& parser)
{
	if(parser.lexer.maxBacktrack)
	{
		printError(parser);
		return true;
	}
	return false;
};

template<typename ParserType, typename T>
cpp::symbol<T> parseSymbolRequired(ParserType& parser, cpp::symbol<T> symbol, size_t best = 0)
{
	T* p = symbol.p;
	PARSE_ASSERT(!checkBacktrack(parser));
	ParserType tmp(parser);
	parser.lexer.push();
	p = SymbolAllocator<T>(parser.lexer.allocator).allocate(p);
#if 0
	p = parseSymbol(tmp, p);
#else
	p = tmp.parse(p);
#endif
	parser.lexer.pop();
	if(p != 0
		&& tmp.position >= best)
	{
		parser.position += tmp.position;
		parser.ambiguityDepth += tmp.ambiguityDepth;
		return cpp::symbol<T>(p);
	}

	tmp.backtrack(SYMBOL_NAME(T));
	return cpp::symbol<T>(0);
}

template<typename ParserType, typename T>
cpp::symbol_optional<T> parseSymbolOptional(ParserType& parser, cpp::symbol_optional<T> symbol)
{
	return cpp::symbol_optional<T>(parseSymbolRequired(parser, symbol));
}

template<typename T>
bool isAmbiguous(T* symbol)
{
	return false;
}

inline void breakpoint()
{
}

struct False
{
};

struct True
{
};

template<typename T>
T* resolveAmbiguity(LinearAllocator& allocator, T* first, T* second, const True&)
{
	typedef cpp::ambiguity<T> Result;
	Result* result =  new(allocator.allocate(sizeof(Result))) Result;
	result->first = first;
	result->second = second;
	return result;
}

template<typename T>
T* resolveAmbiguity(LinearAllocator& allocator, T* first, T* second, const False&)
{
	breakpoint();
	return first;
}

template<typename T>
struct IsAmbiguous
{
	typedef False Result;
};

#define DECLARE_AMBIGUOUS(T) \
	template<> \
	struct IsAmbiguous<T> \
	{ \
		typedef True Result; \
	}

DECLARE_AMBIGUOUS(cpp::template_argument);
DECLARE_AMBIGUOUS(cpp::parameter_declaration);
DECLARE_AMBIGUOUS(cpp::statement);
DECLARE_AMBIGUOUS(cpp::for_init_statement);
DECLARE_AMBIGUOUS(cpp::member_declaration);
DECLARE_AMBIGUOUS(cpp::template_parameter);
DECLARE_AMBIGUOUS(cpp::cast_expression);
DECLARE_AMBIGUOUS(cpp::unary_expression);
DECLARE_AMBIGUOUS(cpp::postfix_expression_prefix);
DECLARE_AMBIGUOUS(cpp::nested_name);
DECLARE_AMBIGUOUS(cpp::init_declarator);
DECLARE_AMBIGUOUS(cpp::mem_initializer_id);

DECLARE_AMBIGUOUS(cpp::expression);
DECLARE_AMBIGUOUS(cpp::constant_expression);
DECLARE_AMBIGUOUS(cpp::assignment_expression);

#ifdef _DEBUG
#define SYMBOLP_NAME(p) (typeid(*p).name() + 12)
#else
#define SYMBOLP_NAME(p) "cpp::$symbol"
#endif

template<typename T>
T* pruneSymbol(T* symbol)
{
	return symbol;
}

template<typename T>
inline T* pruneBinaryExpression(T* symbol)
{
	return symbol->right == 0 ? symbol->left : symbol;
}


template<typename ParserType, typename T, typename OtherT>
cpp::symbol<OtherT> parseSymbolChoice(ParserType& parser, cpp::symbol<T> symbol, OtherT* other)
{
	if(other != 0)
	{
		parser.inGeneralAmbiguity = true;
	}
	ProfileScope profile(parser.inGeneralAmbiguity ? gProfileAmbiguity : gProfileParser);
	if(parser.position != 0)
	{
		if(!parser.lexer.canBacktrack(parser.position))
		{
			return cpp::symbol<OtherT>(other);
		}
		parser.backtrack(0 /*SYMBOL_NAME(T)*/, true);
	}
	size_t ambiguityDepth = parser.ambiguityDepth;
	parser.ambiguityDepth = 0;
	size_t position = parser.position;
	parser.position = 0;
	T* p = parseSymbolRequired(parser, NullPtr<T>::VALUE, position);
	if(p != 0
		&& position != 0
		&& position == parser.position) // successfully parsed an alternative interpretation
	{
		OtherT* alt = pruneSymbol(p);
		{
#if 0
			ProfileScope profile(gProfileDiagnose);
			if(!isAmbiguous(other)) // debug: check that this is a known ambiguity
			{
				// if not, print diagnostic
				printPosition(parser.get_position());
				std::cout << std::endl;
				std::cout << "  " << SYMBOL_NAME(OtherT) << ": ";
				printSymbol(alt);
				std::cout << std::endl;
				std::cout << (!isAmbiguous(other) ? "  UNKNOWN: " : "  known: ");
				std::cout << SYMBOLP_NAME(other) << std::endl;
				std::cout << "    ";
				printSymbol(other, true);
				std::cout << std::endl;
				std::cout << (!isAmbiguous(alt) ? "  UNKNOWN: " : "  known: ");
				std::cout << SYMBOLP_NAME(alt) << std::endl;
				std::cout << "    ";
				printSymbol(alt, true);
				std::cout << std::endl;
				breakpoint();
			}
#endif
		}
#if 0
		return cpp::symbol<OtherT>(other); // for now, ignore alternative interpretations
#else
		return cpp::symbol<OtherT>(resolveAmbiguity(parser.lexer.allocator, other, alt, typename IsAmbiguous<OtherT>::Result()));
#endif
	}
	if(p == 0)
	{
		parser.ambiguityDepth = ambiguityDepth;
		parser.position = position;
		parser.advance();
		return cpp::symbol<OtherT>(other);
	}
	return cpp::symbol<OtherT>(pruneSymbol(p));
}

template<LexTokenId id>
inline LexTokenId getTokenId(cpp::terminal<id>)
{
	return id;
}

inline const char* parseTerminal(Parser& parser, LexTokenId id)
{
	if(isToken(parser.get_id(), id))
	{
		const char* value = parser.get_value();
		parser.increment();
		return value;
	}
	return NULL;
}

enum ParseResult
{
	PARSERESULT_PASS,
	PARSERESULT_FAIL,
	PARSERESULT_SKIP,
};

template<LexTokenId id>
inline ParseResult parseTerminal(Parser& parser, cpp::terminal<id>& result)
{
	result.value = parseTerminal(parser, getTokenId(result));
	return result.value != 0 ? PARSERESULT_PASS : PARSERESULT_FAIL;
}

template<LexTokenId id>
inline ParseResult parseTerminal(Parser& parser, cpp::terminal_optional<id>& result)
{
	result.value = parseTerminal(parser, getTokenId(result));
	return PARSERESULT_PASS;
}

template<LexTokenId id>
inline ParseResult parseTerminal(Parser& parser, cpp::terminal_suffix<id>& result)
{
	result.value = parseTerminal(parser, getTokenId(result));
	return result.value != 0 ? PARSERESULT_PASS : PARSERESULT_SKIP;
}


#define TOKEN_EQUAL(parser, token) isToken((parser).get_id(), token)
// TODO: avoid dependency on 'result'
#define PARSE_TERMINAL(parser, t) switch(parseTerminal(parser, t)) { case PARSERESULT_FAIL: return 0; case PARSERESULT_SKIP: return result; default: break; }

#define PARSE_TOKEN_REQUIRED(parser, token_) if(TOKEN_EQUAL(parser, token_)) { parser.increment(); } else { return 0; }
#define PARSE_TOKEN_OPTIONAL(parser, result, token) result = false; if(TOKEN_EQUAL(parser, token)) { result = true; parser.increment(); }
#define PARSE_SELECT_TOKEN(parser, p, token, value_) if(TOKEN_EQUAL(parser, token)) { p = createSymbol(parser, p); p->id = value_; p->value.id = token; p->value.value = parser.get_value(); parser.increment(); return p; }
#define PARSE_OPTIONAL(parser, p) (p) = parseSymbolOptional(parser, p)
#define PARSE_REQUIRED(parser, p) if(((p) = parseSymbolRequired(parser, p)) == 0) { return 0; }
#if 1
#define PARSE_SELECT(parser, Type) result = parseSymbolChoice(parser, NullPtr<Type>::VALUE, result)
#define PARSE_SELECT_UNAMBIGUOUS(parser, Type) if(cpp::symbol<Type> p = parseSymbolRequired(parser, NullPtr<Type>::VALUE)) { return p; }
#else
#define PARSE_SELECT(parser, Type) if(cpp::symbol<Type> p = parseSymbolRequired(parser, NullPtr<Type>::VALUE)) { return p; }
#endif
// Type must have members 'left' and 'right', and 'typeof(left)' must by substitutable for 'Type'
#define PARSE_PREFIX(parser, Type) if(cpp::symbol<Type> p = parseSymbolRequired(parser, NullPtr<Type>::VALUE)) { if(p->right == 0) return p->left; return p; }


inline bool peekTemplateIdAmbiguity(Parser& parser)
{
	bool result = false;
	if(isToken(parser.lexer.get_id(), boost::wave::T_IDENTIFIER))
	{
		parser.lexer.increment();
		if(isToken(parser.lexer.get_id(), boost::wave::T_LESS))
		{
			result = true;
		}
		parser.lexer.backtrack(1);
	}
	return result;
}

inline bool peekTemplateIdAmbiguityPrev(Parser& parser)
{
	bool result = false;
	if(isToken(parser.lexer.get_id(), boost::wave::T_LESS))
	{
		parser.lexer.backtrack(1);
		if(isToken(parser.lexer.get_id(), boost::wave::T_IDENTIFIER))
		{
			result = true;
		}
		parser.lexer.increment();
	}
	return result;
}



template<typename ParserType, typename T, typename OtherT>
cpp::symbol<OtherT> parseSymbolAmbiguous(ParserType& parser, cpp::symbol<T> symbol, OtherT* result)
{
	static size_t gTemplateIdAmbiguityNest = 0;
	TemplateIdAmbiguityContext context;
	if(parser.ambiguity == 0)
	{
		parser.ambiguity = &context;
#ifdef AMBIGUITY_DEBUG
		std::cout << "ambiguity begin: " << SYMBOL_NAME(T) << std::endl;
#endif
		++gTemplateIdAmbiguityNest;
	}
	result = parseSymbolChoice(parser, symbol, result);
	if(parser.ambiguity == &context)
	{
		--gTemplateIdAmbiguityNest;
		// temp hack
#ifdef AMBIGUITY_DEBUG
		std::cout << "ambiguity end: " << SYMBOL_NAME(T) << std::endl;
#endif
		if(context.max != 0)
		{
			ProfileScope profile(gProfileTemplateId);

#if 0
			for(size_t i = 0; i != gTemplateIdAmbiguityNest; ++i)
			{
				std::cout << " ";
			}
			std::cout << "ambiguous expression " << SYMBOL_NAME(T) << ": depth=" << context.depth << ": ";
			std::cout << std::endl;
			printSymbol(result);
			std::cout << std::endl;
#endif

			// debug
			size_t depth = context.depth;
			OtherT* original = result;

			size_t width = 1 << context.max;
			while(++context.solution != width)
			{
#ifdef AMBIGUITY_DEBUG
				std::cout << "ambiguity solution: " << context.solution << std::endl;
#endif
				result = parseSymbolChoice(parser, symbol, result);
				width = std::max(width, size_t(1 << context.depth)); // handle failure of initial parse, increase depth when a solution is found

				// TEMP HACK
				if(depth > 8)
				{
					// don't bother to brute-force check complex ambiguities for now.

					break;
				}
			}
		}
		parser.ambiguity = 0;
	}
	return cpp::symbol<OtherT>(result);
}
#define PARSE_EXPRESSION_SPECIAL(parser, Type) result = parseSymbolAmbiguous(parser, NullPtr<Type>::VALUE, result)


#if 1
#define PARSE_EXPRESSION PARSE_PREFIX
#else
#define PARSE_EXPRESSION PARSE_EXPRESSION_SPECIAL
#endif


template<typename ParserType, typename T, typename Base>
inline cpp::symbol<Base> parseExpression(ParserType& parser, cpp::symbol<T> symbol, Base* result)
{
	// HACK: create temporary copy of expression-symbol to get RHS-symbol
	result = parseSymbolRequired(parser, T().right);
	if(result != 0)
	{
		for(;;)
		{
			// parse suffix of expression-symbol
			symbol = parseSymbolRequired(parser, symbol);
			if(symbol == 0
				|| symbol->right == 0)
			{
				break;
			}
			symbol->left = makeSymbol(result);
			result = symbol;
		}
	}
	return makeSymbol(result);
}

#define PARSE_EXPRESSION_LEFTASSOCIATIVE(parser, Type) result = parseExpression(parser, NullPtr<Type>::VALUE, result)

template<typename ParserType, typename T>
inline cpp::symbol_optional<T> parseSequence(ParserType& parser, cpp::symbol_optional<T>)
{
	T tmp;
	cpp::symbol_optional<T> p(&tmp);
	for(;;)
	{
		p->next = parseSymbolOptional(parser, p->next);
		if(p->next == 0)
		{
			break;
		}
		p = p->next;
	}
	return tmp.next;
}

#define PARSE_SEQUENCE(parser, p) p = parseSequence(parser, p)


struct ParseResultFail
{
};

struct ParseResultSkip
{
};

struct ParserOpaque : public Parser
{
	void* context;
	ParserOpaque(Lexer& lexer, void* context)
		: Parser(lexer), context(context)
	{
	}
};

template<typename Context>
struct ParserGeneric : public ParserOpaque
{
	ParserGeneric(Lexer& lexer, Context& context)
		: ParserOpaque(lexer, &context)
	{
	}
	Context& makeContext()
	{
		Context& context = *static_cast<Context*>(ParserOpaque::context);
		context.parser = this;
		return context;
	}
	template<typename T>
	T* parse(T* symbol)
	{
		Context& context = makeContext();
		context.visit(symbol);
		return static_cast<T*>(context.result);
	}
};

#define SYMBOL_WALK(walker, symbol) if((result = parseSymbol(getParser(walker), symbol)) == 0) return
#define PARSERCONTEXT_DEFAULT \
	template<typename T> \
	void visit(T* symbol) \
	{ \
		SYMBOL_WALK(*this, symbol); \
	}

template<typename ParserType>
struct ParsingVisitor
{
	ParserType& parser;
	bool skip;
	ParsingVisitor(ParserType& parser) : parser(parser), skip(false)
	{
	}
	template<typename T>
	bool visit(cpp::symbol<T>& s)
	{
		return (s = parseSymbolRequired(parser, s)) != 0;
	}
	template<typename T>
	bool visit(cpp::symbol_optional<T>& s)
	{
		s = cpp::symbol_optional<T>(parseSymbolRequired(parser, s));
		return true;
	}
	template<LexTokenId ID>
	bool visit(cpp::terminal<ID>& t)
	{
		return parseTerminal(parser, t) == PARSERESULT_PASS;
	}
	template<LexTokenId ID>
	bool visit(cpp::terminal_optional<ID>& t)
	{
		parseTerminal(parser, t);
		return true;
	}
	template<LexTokenId ID>
	bool visit(cpp::terminal_suffix<ID>& t)
	{
		skip = parseTerminal(parser, t) != PARSERESULT_PASS;
		return !skip;
	}
};

template<typename ParserType, typename T>
inline T* parseSymbol(ParserType& parser, T* result, const False&, const False&)
{
	ParsingVisitor<ParserType> visitor(parser);
	if(!result->parse(visitor))
	{
#if 0
		std::cout << "rejected: '" << SYMBOL_NAME(T) << "'" << std::endl;
		printSequence(parser.lexer); // rejected tokens
#endif
		return visitor.skip ? result : 0;
	}
	return result;
}


#define GENERIC_ITERATE1(i, op) op(i);
#define GENERIC_ITERATE2(i, op) op(i); GENERIC_ITERATE1(i + 1, op)
#define GENERIC_ITERATE3(i, op) op(i); GENERIC_ITERATE2(i + 1, op)
#define GENERIC_ITERATE4(i, op) op(i); GENERIC_ITERATE3(i + 1, op)
#define GENERIC_ITERATE5(i, op) op(i); GENERIC_ITERATE4(i + 1, op)
#define GENERIC_ITERATE6(i, op) op(i); GENERIC_ITERATE5(i + 1, op)
#define GENERIC_ITERATE7(i, op) op(i); GENERIC_ITERATE6(i + 1, op)
#define GENERIC_ITERATE8(i, op) op(i); GENERIC_ITERATE7(i + 1, op)
#define GENERIC_ITERATE9(i, op) op(i); GENERIC_ITERATE8(i + 1, op)
#define GENERIC_ITERATE10(i, op) op(i); GENERIC_ITERATE9(i + 1, op)
#define GENERIC_ITERATE11(i, op) op(i); GENERIC_ITERATE10(i + 1, op)
#define GENERIC_ITERATE12(i, op) op(i); GENERIC_ITERATE11(i + 1, op)
#define GENERIC_ITERATE13(i, op) op(i); GENERIC_ITERATE12(i + 1, op)
#define GENERIC_ITERATE14(i, op) op(i); GENERIC_ITERATE13(i + 1, op)
#define GENERIC_ITERATE15(i, op) op(i); GENERIC_ITERATE14(i + 1, op)
#define GENERIC_ITERATE16(i, op) op(i); GENERIC_ITERATE15(i + 1, op)
#define GENERIC_ITERATE17(i, op) op(i); GENERIC_ITERATE16(i + 1, op)

template<typename T>
struct TypeListCount<TYPELIST1(cpp::ambiguity<T>)>
{
	enum { RESULT = 0 };
};

template<typename ParserType, typename T, size_t N>
struct ChoiceParserN
{
};

#define DEFINE_CHOICEPARSER(N) \
	template<typename ParserType, typename T> \
	struct ChoiceParserN<ParserType, T, N> \
	{ \
		static T* parseSymbol(ParserType& parser, T* result) \
		{ \
			GENERIC_ITERATE##N(0, CHOICEPARSER_OP) \
			return result; \
		} \
	}

#define CHOICEPARSER_OP(N) result = parseSymbolChoice(parser, NullPtr<typename TypeListNth<typename T::Choices, N>::Result>::VALUE, result)
DEFINE_CHOICEPARSER(1);
DEFINE_CHOICEPARSER(2);
DEFINE_CHOICEPARSER(3);
DEFINE_CHOICEPARSER(4);
DEFINE_CHOICEPARSER(5);
DEFINE_CHOICEPARSER(6);
DEFINE_CHOICEPARSER(7);
DEFINE_CHOICEPARSER(8);
DEFINE_CHOICEPARSER(9);
DEFINE_CHOICEPARSER(10);
DEFINE_CHOICEPARSER(11);
DEFINE_CHOICEPARSER(12);
DEFINE_CHOICEPARSER(13);
DEFINE_CHOICEPARSER(14);
DEFINE_CHOICEPARSER(15);
DEFINE_CHOICEPARSER(16);
DEFINE_CHOICEPARSER(17);
#undef CHOICEPARSER_OP

template<typename ParserType, typename T, size_t N>
struct ChoiceParserUnambiguousN
{
};

#define DEFINE_CHOICEPARSERUNAMBIGUOUS(N) \
	template<typename ParserType, typename T> \
	struct ChoiceParserUnambiguousN<ParserType, T, N> \
	{ \
		static T* parseSymbol(ParserType& parser, T* result) \
		{ \
			GENERIC_ITERATE##N(0, CHOICEPARSER_OP) \
			return result; \
		} \
	}

#define CHOICEPARSER_OP(N) if(result = parseSymbolRequired(parser, NullPtr<typename TypeListNth<typename T::Choices, N>::Result>::VALUE)) return result
DEFINE_CHOICEPARSERUNAMBIGUOUS(1);
DEFINE_CHOICEPARSERUNAMBIGUOUS(2);
DEFINE_CHOICEPARSERUNAMBIGUOUS(3);
DEFINE_CHOICEPARSERUNAMBIGUOUS(4);
DEFINE_CHOICEPARSERUNAMBIGUOUS(5);
DEFINE_CHOICEPARSERUNAMBIGUOUS(6);
DEFINE_CHOICEPARSERUNAMBIGUOUS(7);
DEFINE_CHOICEPARSERUNAMBIGUOUS(8);
DEFINE_CHOICEPARSERUNAMBIGUOUS(9);
DEFINE_CHOICEPARSERUNAMBIGUOUS(10);
DEFINE_CHOICEPARSERUNAMBIGUOUS(11);
DEFINE_CHOICEPARSERUNAMBIGUOUS(12);
DEFINE_CHOICEPARSERUNAMBIGUOUS(13);
DEFINE_CHOICEPARSERUNAMBIGUOUS(14);
DEFINE_CHOICEPARSERUNAMBIGUOUS(15);
DEFINE_CHOICEPARSERUNAMBIGUOUS(16);
DEFINE_CHOICEPARSERUNAMBIGUOUS(17);
#undef CHOICEPARSER_OP


template<typename ParserType, typename T, typename Choices>
struct ChoiceParser
{
	static T* parseSymbol(ParserType& parser, T* result)
	{
		result = parseSymbolChoice(parser, NullPtr<typename Choices::Item>::VALUE, result);
		return ChoiceParser<ParserType, T, typename Choices::Next>::parseSymbol(parser, result);
	}
};

template<typename ParserType, typename T>
struct ChoiceParser<ParserType, T, TypeListEnd>
{
	static T* parseSymbol(ParserType& parser, T* result)
	{
		return result;
	}
};

template<typename ParserType, typename T>
struct ChoiceParser<ParserType, T, TYPELIST1(cpp::ambiguity<T>)>
{
	static T* parseSymbol(ParserType& parser, T* result)
	{
		return result;
	}
};

template<typename ParserType, typename T>
inline T* parseSymbol(ParserType& parser, T* result, const True&, const True&)
{
#if 1
	return ChoiceParserN<ParserType, T, TypeListCount<typename T::Choices>::RESULT>::parseSymbol(parser, result);
#else
	return ChoiceParser<ParserType, T, typename T::Choices>::parseSymbol(parser, result);
#endif
}

template<typename ParserType, typename T, typename Choices>
struct ChoiceParserUnambiguous
{
	static T* parseSymbol(ParserType& parser, T* result)
	{
#if 0
		result = parseSymbolChoice(parser, NullPtr<typename Choices::Item>::VALUE, result);
#else
		if(result = parseSymbolRequired(parser, NullPtr<typename Choices::Item>::VALUE))
		{
#if 0
			std::cout << "matched: " << SYMBOL_NAME(typename Choices::Item) << std::endl;
			printSymbol(result);
			std::cout << std::endl;
#endif
			return result;
		}
#endif
		return ChoiceParserUnambiguous<ParserType, T, typename Choices::Next>::parseSymbol(parser, result);
	}
};

template<typename ParserType, typename T>
struct ChoiceParserUnambiguous<ParserType, T, TypeListEnd>
{
	static T* parseSymbol(ParserType& parser, T* result)
	{
		return result;
	}
};

template<typename ParserType, typename T>
inline T* parseSymbol(ParserType& parser, T* result, const True&, const False&)
{
#if 1
	return ChoiceParserUnambiguousN<ParserType, T, TypeListCount<typename T::Choices>::RESULT>::parseSymbol(parser, result);
#else
	return ChoiceParserUnambiguous<ParserType, T, typename T::Choices>::parseSymbol(parser, result);
#endif
}

template<typename T>
struct IsChoice
{
	typedef True Result;
};

template<>
struct IsChoice<TypeListEnd>
{
	typedef False Result;
};

template<typename ParserType, typename T>
inline T* parseSymbol(ParserType& parser, T* result)
{
	return parseSymbol(parser, result, typename IsChoice<typename T::Choices>::Result(), typename IsAmbiguous<T>::Result());
}


// skips a braced token sequence
inline void skipBraced(Parser& parser)
{
	while(!TOKEN_EQUAL(parser, boost::wave::T_RIGHTBRACE))
	{
		PARSE_ASSERT(!TOKEN_EQUAL(parser, boost::wave::T_EOF));
		if(TOKEN_EQUAL(parser, boost::wave::T_LEFTBRACE))
		{
			parser.increment();
			skipBraced(parser);
			parser.increment();
		}
		else
		{
			parser.increment();
		}
	}
}

// skips a parenthesised expression
inline void skipParenthesised(Parser& parser)
{
	while(!TOKEN_EQUAL(parser, boost::wave::T_RIGHTPAREN))
	{
		PARSE_ASSERT(!TOKEN_EQUAL(parser, boost::wave::T_EOF));
		PARSE_ASSERT(!TOKEN_EQUAL(parser, boost::wave::T_SEMICOLON));
		if(TOKEN_EQUAL(parser, boost::wave::T_LEFTPAREN))
		{
			parser.increment();
			skipParenthesised(parser);
			parser.increment();
		}
		else
		{
			parser.increment();
		}
	}
}

// skips a mem-initializer-list
inline void skipMemInitializerList(Parser& parser)
{
	while(!TOKEN_EQUAL(parser, boost::wave::T_LEFTBRACE))
	{
		PARSE_ASSERT(!TOKEN_EQUAL(parser, boost::wave::T_EOF));
		PARSE_ASSERT(!TOKEN_EQUAL(parser, boost::wave::T_SEMICOLON));
		parser.increment();
	}
}

cpp::declaration_seq* parseFile(Lexer& lexer);
cpp::statement_seq* parseFunction(Lexer& lexer);

#endif


