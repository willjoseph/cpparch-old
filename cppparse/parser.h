
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
	for( Lexer::Tokens::const_iterator i = lexer.position; i != lexer.history.end(); i = lexer.history.next(i))
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
};

template<typename T>
void printSymbol(T* symbol)
{
	SymbolDebug walker;
	walker.visit(symbol);
}

struct ParserState
{
	Lexer::Tokens::const_iterator ambiguityPos;
	bool inTemplateArgumentList;
	bool ignoreTemplateId;
	bool ignoreRelationalLess;
	ParserState()
		: ambiguityPos(0), inTemplateArgumentList(false), ignoreTemplateId(false), ignoreRelationalLess(false)
	{
	}
};

struct Parser : public ParserState
{
	Lexer& lexer;
	size_t position;
	size_t allocation;

	Parser(Lexer& lexer)
		: lexer(lexer), position(0), allocation(0)
	{
	}
	Parser(const Parser& other)
		: ParserState(other), lexer(other.lexer), position(0), allocation(lexer.allocator.position)
	{
		if(!getAmbiguity())
		{
			ambiguityPos = 0;
		}
	}

	void setAmbiguity()
	{
		ambiguityPos = lexer.position;
	}
	bool getAmbiguity() const
	{
		return ambiguityPos == lexer.position;
	}

	LexTokenId get_id()
	{
		return lexer.get_id();
	}
	const char* get_value()
	{
		return lexer.get_value();
	}
	LexFilePosition get_position()
	{
		return lexer.get_position();
	}

	void increment()
	{
		++position;
		lexer.increment();
	}

	void backtrack(const char* symbol)
	{
		lexer.backtrack(position, symbol);
		lexer.allocator.position = allocation;
	}
};

inline void printPosition(const LexFilePosition& position)
{
	std::cout << position.get_file() << "(" << position.get_line() << "): ";
}

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

#define SYMBOL_NAME(T) (typeid(T).name() + 12)

inline bool checkBacktrack(Parser& parser)
{
	if(parser.lexer.maxBacktrack)
	{
		printError(parser);
		return true;
	}
	return false;
};

template<typename T>
cpp::symbol<T> parseSymbolRequired(Parser& parser, cpp::symbol<T> symbol, size_t best = 0)
{
	T* p = symbol.p;
	PARSE_ASSERT(!checkBacktrack(parser));
	Parser tmp(parser);
	parser.lexer.push();
	p = SymbolAllocator<T>(parser.lexer.allocator).allocate(p);
	p = parseSymbol(tmp, p);
	parser.lexer.pop();
	if(p != 0
		&& tmp.position >= best)
	{
		parser.position += tmp.position - best;
		return cpp::symbol<T>(p);
	}

	tmp.backtrack(SYMBOL_NAME(T));
	return cpp::symbol<T>(0);
}

template<typename T>
cpp::symbol_optional<T> parseSymbolOptional(Parser& parser, cpp::symbol_optional<T> symbol)
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


DECLARE_AMBIGUOUS(cpp::expression);
DECLARE_AMBIGUOUS(cpp::conditional_expression);
DECLARE_AMBIGUOUS(cpp::assignment_expression);
DECLARE_AMBIGUOUS(cpp::logical_or_expression);
DECLARE_AMBIGUOUS(cpp::logical_and_expression);
DECLARE_AMBIGUOUS(cpp::inclusive_or_expression);
DECLARE_AMBIGUOUS(cpp::exclusive_or_expression);
DECLARE_AMBIGUOUS(cpp::and_expression);
DECLARE_AMBIGUOUS(cpp::equality_expression);
DECLARE_AMBIGUOUS(cpp::relational_expression);
DECLARE_AMBIGUOUS(cpp::shift_expression);
DECLARE_AMBIGUOUS(cpp::additive_expression);
DECLARE_AMBIGUOUS(cpp::multiplicative_expression);
DECLARE_AMBIGUOUS(cpp::pm_expression);
DECLARE_AMBIGUOUS(cpp::cast_expression);
DECLARE_AMBIGUOUS(cpp::unary_expression);

#define SYMBOLP_NAME(p) (typeid(*p).name() + 12)

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


template<typename T, typename OtherT>
cpp::symbol<OtherT> parseSymbolChoice(Parser& parser, cpp::symbol<T> symbol, OtherT* other)
{
	if(parser.position != 0)
	{
		if(!parser.lexer.canBacktrack(parser.position))
		{
			return cpp::symbol<OtherT>(other);
		}
		parser.lexer.backtrack(parser.position);
	}
	size_t position = parser.position;
	T* p = parseSymbolRequired(parser, NullPtr<T>::VALUE, parser.position);
	if(p != 0
		&& position != 0
		&& position == parser.position) // successfully parsed an alternative interpretation
	{
		OtherT* alt = pruneSymbol(p);
		if(!isAmbiguous(other)) // debug: check that this is a known ambiguity
		{
			// if not, print diagnostic
			printPosition(parser.get_position());
			std::cout << std::endl;
			std::cout << "  " << SYMBOL_NAME(OtherT) << ": ";
			printSymbol(alt);
			std::cout << std::endl;
			std::cout << (!isAmbiguous(alt) ? "  UNKNOWN: " : "  known: ");
			std::cout << SYMBOLP_NAME(alt) << std::endl;
			std::cout << (!isAmbiguous(other) ? "  UNKNOWN: " : "  known: ");
			std::cout << SYMBOLP_NAME(other) << std::endl;
			breakpoint();
		}
#if 0
		return cpp::symbol<OtherT>(other); // for now, ignore alternative interpretations
#else
		return cpp::symbol<OtherT>(resolveAmbiguity(parser.lexer.allocator, other, alt, IsAmbiguous<OtherT>::Result()));
#endif
	}
	if(p == 0)
	{
		if(parser.position != 0)
		{
			parser.lexer.advance(parser.position);
		}
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


#define TOKEN_EQUAL(parser, token) isToken(parser.get_id(), token)
// TODO: avoid dependency on 'result'
#define PARSE_TERMINAL(parser, t) switch(parseTerminal(parser, t)) { case PARSERESULT_FAIL: return NULL; case PARSERESULT_SKIP: return result; default: break; }

#define PARSE_TOKEN_REQUIRED(parser, token_) if(TOKEN_EQUAL(parser, token_)) { parser.increment(); } else { return NULL; }
#define PARSE_TOKEN_OPTIONAL(parser, result, token) result = false; if(TOKEN_EQUAL(parser, token)) { result = true; parser.increment(); }
#define PARSE_SELECT_TOKEN(parser, p, token, value_) if(TOKEN_EQUAL(parser, token)) { p = createSymbol(parser, p); p->id = value_; p->value.id = token; p->value.value = parser.get_value(); parser.increment(); return p; }
#define PARSE_OPTIONAL(parser, p) (p) = parseSymbolOptional(parser, p)
#define PARSE_REQUIRED(parser, p) if(((p) = parseSymbolRequired(parser, p)) == 0) { return NULL; }
#if 1
#define PARSE_SELECT(parser, Type) result = parseSymbolChoice(parser, NullPtr<Type>::VALUE, result)
#define PARSE_SELECT_UNAMBIGUOUS(parser, Type) if(cpp::symbol<Type> p = parseSymbolRequired(parser, NullPtr<Type>::VALUE)) { return p; }
#else
#define PARSE_SELECT(parser, Type) if(cpp::symbol<Type> p = parseSymbolRequired(parser, NullPtr<Type>::VALUE)) { return p; }
#endif
// Type must have members 'left' and 'right', and 'typeof(left)' must by substitutable for 'Type'
#define PARSE_PREFIX(parser, Type) if(cpp::symbol<Type> p = parseSymbolRequired(parser, NullPtr<Type>::VALUE)) { if(p->right == NULL) return p->left; return p; }

inline cpp::simple_template_id* parseSymbol(Parser& parser, cpp::simple_template_id*);

inline bool peekTemplateIdAmbiguity(Parser& parser)
{
	Parser tmp(parser);
#if 0 // TEMP HACK: check for full template-id: this avoids false-positives when parsing 'X - Y < Z;'
	cpp::symbol_optional<cpp::simple_template_id> symbol;
	PARSE_OPTIONAL(tmp, symbol);
	bool result = symbol != 0;
#else
	bool result = false;
	if(TOKEN_EQUAL(tmp, boost::wave::T_COLON_COLON))
	{
		tmp.increment();
	}
	if(TOKEN_EQUAL(tmp, boost::wave::T_IDENTIFIER))
	{
		tmp.increment();
		if(TOKEN_EQUAL(tmp, boost::wave::T_LESS))
		{
			result = true;
		}
	}
#endif
	tmp.backtrack("peekTemplateIdAmbiguity");
	return result;
}

// if the next tokens look like a template-id
	// first try parsing for a template-id
	// then try parsing for a relational-expression
#define PARSE_EXPRESSION_SPECIAL(parser, Type) \
	if(!parser.getAmbiguity() \
		&& peekTemplateIdAmbiguity(parser)) \
	{ \
		parser.setAmbiguity(); \
		parser.ignoreTemplateId = false; \
		parser.ignoreRelationalLess = true; \
		PARSE_SELECT(parser, Type); \
		parser.ignoreRelationalLess = false; \
		parser.ignoreTemplateId = true; \
	} \
	PARSE_SELECT(parser, Type);

#if 1
#define PARSE_EXPRESSION PARSE_PREFIX
#else
#define PARSE_EXPRESSION PARSE_EXPRESSION_SPECIAL
#endif

cpp::declaration_seq* parseFile(Lexer& lexer);
cpp::statement_seq* parseFunction(Lexer& lexer);

#endif


