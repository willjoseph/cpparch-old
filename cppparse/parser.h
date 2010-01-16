
#ifndef INCLUDED_CPPPARSE_PARSER_H
#define INCLUDED_CPPPARSE_PARSER_H

#include "lexer.h"
#include "cpptree.h"

#include <vector>
#include <set>
#include <iostream>
#include <string.h> // strlen
#include <algorithm> // std::min

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

	Token(LexTokenId id, const char* value, const LexFilePosition& position)
		: id(id), value(value), position(position)
	{
	}
};

struct Scanner
{
	LexIterator& first;
	LexIterator& last;

	typedef std::vector<Token> Tokens;
	Tokens history;
	Tokens::const_iterator position;

	typedef std::set<std::string> Identifiers;
	Identifiers identifiers;

	Scanner(LexContext& context)
		: first(createBegin(context)), last(createEnd(context)), position(history.end())
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
	void backtrack(size_t count)
	{
		position -= count;
#ifdef _DEBUG
		if(count != 0)
		{
			int bleh = 0;
		}
#endif
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
			++position;
		}
		else
		{
			history.push_back(Token(get_id(), makeIdentifier(get_value()), get_position()));
			position = history.end();
			for(;;)
			{
				if(first == last)
				{
					throw LexError();
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

inline void printSequence(Scanner::Tokens::const_iterator first, Scanner::Tokens::const_iterator last)
{
	std::cout << "   ";
	bool space = false;
	bool endline = false;
	for( Scanner::Tokens::const_iterator i = first; i != last; ++i)
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
	position = std::min(scanner.history.size(), position);
	printSequence(scanner.position - position, scanner.position);
}

inline void printError(Scanner& scanner)
{
	std::cout << scanner.get_position().get_file() << "(" << scanner.get_position().get_line() << "): parse error: '" << get_value(dereference(scanner.first)) << "'" << std::endl;
	printSequence(scanner.position, scanner.history.end()); // rejected tokens
}

struct Parser
{
	Scanner& scanner;
	size_t position;

	const char* symbol;
	LexTokenId token;

	Parser(Scanner& scanner)
		: scanner(scanner), position(0)
	{
	}
	Parser(const Parser& other)
		: scanner(other.scanner), position(0)
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

	void increment()
	{
		++position;
		scanner.increment();
	}

	void backtrack()
	{
		scanner.backtrack(position);
	}
};

inline void printSequence(Parser& parser)
{
	printSequence(parser.scanner, parser.position);
}

#define PARSE_ERROR() throw LexError()
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
T* createNode(T*)
{
	return new T;
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
T* parseNodeInternal(Parser& parser, T* p, bool print)
{
	Parser tmp(parser);
	tmp.symbol = SYMBOL_NAME(T);
	p = parseNode(tmp, new T);
	if(p != NULL)
	{
#ifdef _DEBUG
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

		parser.symbol = tmp.symbol;
		parser.token = tmp.token;
		tmp.backtrack();
	}
	return p;
}

#define TOKEN_EQUAL(parser, token) isToken(parser.get_id(), token)
#define PARSE_TOKEN_REQUIRED(parser, token_) if(TOKEN_EQUAL(parser, token_)) { parser.increment(); } else { parser.token = token_; return NULL; }
#define PARSE_TOKEN_OPTIONAL(parser, result, token) result = false; if(TOKEN_EQUAL(parser, token)) { result = true; parser.increment(); }
#define PARSE_SELECT_TOKEN(parser, p, token, value_) if(TOKEN_EQUAL(parser, token)) { p->value = value_; parser.increment(); return p; }
#define PARSE_OPTIONAL(parser, p) (p) = parseNodeInternal(parser, p, true)
#define PARSE_REQUIRED(parser, p) if(((p) = parseNodeInternal(parser, p, true)) == 0) { return NULL; }
#define PARSE_SELECT(parser, Type) if(Type* p = parseNodeInternal(parser, NullPtr<Type>::VALUE, true)) { /*parser.symbol = #Type;*/ return p; }
// Type must have members 'left' and 'right', and 'typeof(left)' must by substitutable for 'Type'
#define PARSE_PREFIX(parser, Type) if(Type* p = parseNodeInternal(parser, NullPtr<Type>::VALUE, true)) { if(p->right == NULL) return p->left; return p; }

cpp::declaration_seq* parseFile(LexContext& context);
cpp::statement_seq* parseFunction(LexContext& context);

#endif


