
#ifndef INCLUDED_CPPPARSE_PARSER_H
#define INCLUDED_CPPPARSE_PARSER_H

#include "lexer.h"
#include "cpptree.h"

#include <vector>
#include <set>
#include <iostream>
#include <string.h> // strlen
#include <algorithm> // std::min

struct ParseError
{
	ParseError()
	{
	}
};


inline bool isEOF(LexTokenId token)
{
	return IS_CATEGORY(token, boost::wave::EOFTokenType);
}

inline bool isWhiteSpace(LexTokenId token)
{
	return IS_CATEGORY(token, boost::wave::WhiteSpaceTokenType)
		|| IS_CATEGORY(token, boost::wave::EOLTokenType)
		|| isEOF(token);
}

struct Token
{
	LexTokenId id;
	const char* value;
	LexFilePosition position;

	Token()
		: id(boost::wave::T_UNKNOWN)
	{
	}
	Token(LexTokenId id, const char* value, const LexFilePosition& position)
		: id(id), value(value), position(position)
	{
	}
};

struct Page
{
	enum { SIZE = 128 * 1024 };
	char buffer[SIZE]; // debug padding

	Page()
	{
#ifdef _DEBUG
		std::uninitialized_fill(buffer, buffer + SIZE, 0xbabababa);
#endif
	}
};

struct LinearAllocator
{
	typedef std::vector<Page*> Pages;
	Pages pages;
	size_t position;
	LinearAllocator()
		: position(0)
	{
	}
	~LinearAllocator()
	{
		for(Pages::iterator i = pages.begin(); i != pages.end(); ++i)
		{
			Page* p = *i;
			delete p; // TODO: fix heap-corruption assert
		}
	}
	Page* getPage(size_t index)
	{
		if(index == pages.size())
		{
			pages.push_back(new Page);
		}
		return pages[index];
	}
	void* allocate(size_t size)
	{
		size_t available = sizeof(Page) - position % sizeof(Page);
		if(size > available)
		{
			position += available;
		}
		Page* page = getPage(position / sizeof(Page));
		void* p = page->buffer + position % sizeof(Page);
		position += size;
		return p;
	}
};

struct BacktrackStats
{
	size_t count;
	const char* symbol;
	LexFilePosition position;
	BacktrackStats()
		: count(0)
	{
	}
};

struct TokenBuffer
{
	enum { SIZE = 1024 };
	Token tokens[SIZE];
	Token* position;

	typedef Token* iterator;
	typedef const Token* const_iterator;

	TokenBuffer() : position(tokens)
	{
	}

	iterator next(iterator i)
	{
		++i;
		return i == tokens + SIZE ? tokens : i;
	}
	const_iterator next(const_iterator i) const
	{
		++i;
		return i == tokens + SIZE ? tokens : i;
	}

	size_t distance(const_iterator i, const_iterator other)
	{
		return (i > other) ? SIZE - (i - other) : other - i;
	}

	const_iterator backtrack(const_iterator i, size_t count)
	{
		return (count > size_t(i - tokens)) ? i + (SIZE - count) : i - count;
	}

	iterator begin()
	{
		return next(position);
	}
	iterator end()
	{
		return position;
	}
	const_iterator begin() const
	{
		return next(position);
	}
	const_iterator end() const
	{
		return position;
	}
	void push_back(const Token& token)
	{
		*position = token;
		position = next(position);
	}
};

struct Scanner
{
	LinearAllocator allocator;

	LexIterator& first;
	LexIterator& last;

	typedef TokenBuffer Tokens;
	Tokens history;
	Tokens::const_iterator position;

	typedef std::set<std::string> Identifiers;
	Identifiers identifiers;

	typedef std::vector<size_t> Positions;
	Positions stacktrace;
	Positions::iterator stackpos;

	BacktrackStats stats;
	bool maxBacktrack;

	Scanner(LexContext& context)
		: first(createBegin(context)), last(createEnd(context)), position(history.end()), stackpos(stacktrace.end()), maxBacktrack(false)
	{
		if(isWhiteSpace(get_id()))
		{
			increment();
		}
	}
	~Scanner()
	{
		release(first);
		release(last);
	}
	void backtrack(size_t count, const char* symbol)
	{
		if(count == 0)
		{
			return;
		}
		if(history.distance(position, history.end()) + count >= TokenBuffer::SIZE)
		{
			maxBacktrack = true;
		}
		else
		{
			position = history.backtrack(position, count);
			if(count > stats.count)
			{
				stats.count = count;
				stats.symbol = symbol;
				stats.position = (*position).position;
			};
		}
	}
	void push()
	{
		//stacktrace.erase(stackpos, stacktrace.end());
		//stacktrace.push_back(position - history.begin());
		//stackpos = stacktrace.end();
	}
	void pop()
	{
		//*--stackpos = position - history.begin();
	}

	const char* makeIdentifier(const char* value)
	{
		return (*identifiers.insert(value).first).c_str();
	}

	bool finished() const
	{
		return position == history.end() && first == last;
	}

	LexTokenId get_id()
	{
		return position != history.end() ? (*position).id : ::get_id(dereference(first));
	}
	const char* get_value()
	{
		return position != history.end() ? (*position).value : ::get_value(dereference(first));
	}
	LexFilePosition get_position()
	{
		return position != history.end() ? (*position).position : ::get_position(dereference(first));
	}

	void increment()
	{
		if(position != history.end())
		{
			position = history.next(position);
		}
		else
		{
			history.push_back(Token(get_id(), makeIdentifier(get_value()), get_position()));
			position = history.end();
			for(;;)
			{
				if(first == last)
				{
					throw ParseError();
				}
				::increment(first);
				if(!isWhiteSpace(get_id())
					|| first == last)
				{
					break;
				}
			}
		}
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

inline void printSequence(Scanner& scanner)
{
	std::cout << "   ";
	bool space = false;
	bool endline = false;
	for( Scanner::Tokens::const_iterator i = scanner.position; i != scanner.history.end(); i = scanner.history.next(i))
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

inline void printSequence(Scanner& scanner, size_t position)
{
	//position = std::min(std::min(scanner.history.size(), size_t(32)), position);
	//printSequence(scanner.position - position, scanner.position);
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
	Scanner& scanner;
	size_t position;
	size_t allocation;

	Parser(Scanner& scanner)
		: scanner(scanner), position(0), allocation(0)
	{
	}
	Parser(const Parser& other)
		: ParserState(other), scanner(other.scanner), position(0), allocation(scanner.allocator.position)
	{
	}

	LexTokenId get_id()
	{
		return scanner.get_id();
	}
	const char* get_value()
	{
		return scanner.get_value();
	}
	LexFilePosition get_position()
	{
		return scanner.get_position();
	}

	void increment()
	{
		++position;
		scanner.increment();
	}

	void backtrack(const char* symbol)
	{
		scanner.backtrack(position, symbol);
		scanner.allocator.position = allocation;
	}
};

inline void printPosition(const LexFilePosition& position)
{
	std::cout << position.get_file() << "(" << position.get_line() << "): ";
}

inline void printError(Parser& parser)
{
#if 0
	for(Scanner::Positions::const_iterator i = parser.scanner.backtrace.begin(); i != parser.scanner.backtrace.end(); ++i)
	{
	}
	printPosition(parser.scanner, scanner.history[parser.scanner.stacktrace.back()].position);
#endif
	printPosition(get_position(dereference(parser.scanner.first)));
	std::cout << "syntax error: '" << get_value(dereference(parser.scanner.first)) << "'" << std::endl;
	printSequence(parser.scanner); // rejected tokens
}

inline void printSequence(Parser& parser)
{
	printSequence(parser.scanner, parser.position);
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
	static T* VALUE;
};

template<typename T>
T* NullPtr<T>::VALUE = NULL;

#define SYMBOL_NAME(T) (typeid(T).name() + 12)

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
	return new(parser.scanner.allocator.allocate(sizeof(T))) T;
}

template<typename T>
T* parseSymbolInternal(Parser& parser, T* p, bool print)
{
	PARSE_ASSERT(!parser.scanner.maxBacktrack);
	Parser tmp(parser);
	parser.scanner.push();
	p = SymbolAllocator<T>(parser.scanner.allocator).allocate(p);
	p = parseSymbol(tmp, p);
	parser.scanner.pop();
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

inline bool parseTerminal(Parser& parser, LexTokenId id)
{
	if(isToken(parser.get_id(), id))
	{
		parser.increment();
		return true;
	}
	return false;
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
	return parseTerminal(parser, getTokenId(result)) ? PARSERESULT_PASS : PARSERESULT_FAIL;
}

template<LexTokenId id>
inline ParseResult parseTerminal(Parser& parser, cpp::terminal_optional<id>& result)
{
	result.parsed = parseTerminal(parser, getTokenId(result));
	return PARSERESULT_PASS;
}

template<LexTokenId id>
inline ParseResult parseTerminal(Parser& parser, cpp::terminal_suffix<id>& result)
{
	return parseTerminal(parser, getTokenId(result)) ? PARSERESULT_PASS : PARSERESULT_SKIP;
}


#define TOKEN_EQUAL(parser, token) isToken(parser.get_id(), token)
// TODO: avoid dependency on 'result'
#define PARSE_TERMINAL(parser, t) switch(parseTerminal(parser, t)) { case PARSERESULT_FAIL: return NULL; case PARSERESULT_SKIP: return result; default: break; }

#define PARSE_TOKEN_REQUIRED(parser, token_) if(TOKEN_EQUAL(parser, token_)) { parser.increment(); } else { return NULL; }
#define PARSE_TOKEN_OPTIONAL(parser, result, token) result = false; if(TOKEN_EQUAL(parser, token)) { result = true; parser.increment(); }
#define PARSE_SELECT_TOKEN(parser, p, token, value_) if(TOKEN_EQUAL(parser, token)) { p = createSymbol(parser, p); p->value = value_; parser.increment(); return p; }
#define PARSE_OPTIONAL(parser, p) (p) = parseSymbolInternal(parser, p, true)
#define PARSE_REQUIRED(parser, p) if(((p) = parseSymbolInternal(parser, p, true)) == 0) { return NULL; }
#define PARSE_SELECT(parser, Type) if(Type* p = parseSymbolInternal(parser, NullPtr<Type>::VALUE, true)) { return p; }
// Type must have members 'left' and 'right', and 'typeof(left)' must by substitutable for 'Type'
#define PARSE_PREFIX(parser, Type) if(Type* p = parseSymbolInternal(parser, NullPtr<Type>::VALUE, true)) { if(p->right == NULL) return p->left; return p; }

cpp::declaration_seq* parseFile(Scanner& scanner);
cpp::statement_seq* parseFunction(Scanner& scanner);

#endif


