
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

struct ParserState
{
	bool inTemplateArgumentList;
	ParserState()
		: inTemplateArgumentList(false)
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

inline void printMatch(const char* symbol)
{
	std::cout << "matched: " << symbol << std::endl;
}

inline void printRejected(const char* symbol)
{
	std::cout << "rejected: " << symbol << std::endl;
}

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

template<typename T>
T* parseSymbolInternal(Parser& parser, cpp::symbol<T> symbol, bool print)
{
	T* p = symbol.p;
	PARSE_ASSERT(!parser.lexer.maxBacktrack);
	Parser tmp(parser);
	parser.lexer.push();
	p = SymbolAllocator<T>(parser.lexer.allocator).allocate(p);
	p = parseSymbol(tmp, p);
	parser.lexer.pop();
	if(p != NULL)
	{
#if 0
		if(print)
		{
			printMatch(SYMBOL_NAME(T));
			printSequence(tmp);
		}
#endif
		parser.position += tmp.position;
	}
	else
	{
#if 0
		if(tmp.position != 0
			&& print)
		{
			printRejected(SYMBOL_NAME(T)); 
			printSequence(tmp);
		}
#endif

		tmp.backtrack(SYMBOL_NAME(T));
	}
	return p;
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
#define PARSE_OPTIONAL(parser, p) (p) = parseSymbolInternal(parser, p, true)
#define PARSE_REQUIRED(parser, p) if(((p) = parseSymbolInternal(parser, p, true)) == 0) { return NULL; }
#define PARSE_SELECT(parser, Type) if(Type* p = parseSymbolInternal(parser, NullPtr<Type>::VALUE, true)) { return p; }
// Type must have members 'left' and 'right', and 'typeof(left)' must by substitutable for 'Type'
#define PARSE_PREFIX(parser, Type) if(Type* p = parseSymbolInternal(parser, NullPtr<Type>::VALUE, true)) { if(p->right == NULL) return p->left; return p; }

cpp::declaration_seq* parseFile(Lexer& lexer);
cpp::statement_seq* parseFunction(Lexer& lexer);

#endif


