
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
	TemplateIdAmbiguityContext() : depth(0), solution(0)
	{
	}
	void nextDepth()
	{
		if(depth == 15) // limit to 2^16 permutations
		{
			throw ParseError();
		}
		++depth;
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

	ParserState()
		: ambiguity(0), inTemplateArgumentList(false)
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
	LexFilePosition get_position()
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
			lexer.allocator.position = allocation;
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
		parser.position += tmp.position;
		parser.ambiguityDepth += tmp.ambiguityDepth;
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
DECLARE_AMBIGUOUS(cpp::cast_expression);
DECLARE_AMBIGUOUS(cpp::unary_expression);
DECLARE_AMBIGUOUS(cpp::postfix_expression_prefix);
DECLARE_AMBIGUOUS(cpp::nested_name);
DECLARE_AMBIGUOUS(cpp::init_declarator);
DECLARE_AMBIGUOUS(cpp::mem_initializer_id);

DECLARE_AMBIGUOUS(cpp::expression);
DECLARE_AMBIGUOUS(cpp::constant_expression);
DECLARE_AMBIGUOUS(cpp::assignment_expression);


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
		parser.backtrack(SYMBOL_NAME(T), true);
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
#if 0
		return cpp::symbol<OtherT>(other); // for now, ignore alternative interpretations
#else
		return cpp::symbol<OtherT>(resolveAmbiguity(parser.lexer.allocator, other, alt, IsAmbiguous<OtherT>::Result()));
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


#define TOKEN_EQUAL(parser, token) isToken(parser.get_id(), token)
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


// Returns true if the current symbol should be ignored
inline bool checkTemplateIdAmbiguity(Parser& parser, bool templateId)
{
	if(parser.ambiguity != 0
		&& peekTemplateIdAmbiguity(parser))
	{
#ifdef AMBIGUITY_DEBUG
		std::cout << "ambiguity check: " << templateId << std::endl;
#endif
		bool result = parser.ambiguity->ignoreTemplateId() == templateId;
#if 0
		if(!result)
		{
			parser.nextDepth();
		}
#endif
		return result;
	}
	return false;
}

template<typename T, typename OtherT>
cpp::symbol<OtherT> parseSymbolAmbiguous(Parser& parser, cpp::symbol<T> symbol, OtherT* result)
{
	TemplateIdAmbiguityContext context;
	if(parser.ambiguity == 0)
	{
		parser.ambiguity = &context;
#ifdef AMBIGUITY_DEBUG
		std::cout << "ambiguity begin: " << SYMBOL_NAME(T) << std::endl;
#endif
	}
	result = parseSymbolChoice(parser, symbol, result);
	if(parser.ambiguity == &context)
	{
#ifdef AMBIGUITY_DEBUG
		std::cout << "ambiguity end: " << SYMBOL_NAME(T) << std::endl;
#endif
		if(context.depth != 0)
		{
			// debug
			size_t depth = context.depth;
			OtherT* original = result;

			size_t width = 1 << context.depth;
			while(++context.solution != width)
			{
#ifdef AMBIGUITY_DEBUG
				std::cout << "ambiguity solution: " << context.solution << std::endl;
#endif
				result = parseSymbolChoice(parser, symbol, result);
				width = std::max(width, size_t(1 << context.depth)); // handle failure of initial parse, increase depth when a solution is found
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


template<typename T>
inline cpp::symbol<T> makeSymbol(T* p)
{
	return cpp::symbol<T>(p);
}


template<typename T, typename Base>
inline cpp::symbol<Base> parseExpression(Parser& parser, cpp::symbol<T> symbol, Base* result)
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

template<typename T>
inline cpp::symbol_optional<T> parseSequence(Parser& parser, cpp::symbol_optional<T>)
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



cpp::declaration_seq* parseFile(Lexer& lexer);
cpp::statement_seq* parseFunction(Lexer& lexer);

#endif


