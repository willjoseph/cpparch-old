
#ifndef INCLUDED_CPPPARSE_PARSER_H
#define INCLUDED_CPPPARSE_PARSER_H

#include "lexer.h"
#include "cpptree.h"

#include <vector>
#include <set>
#include <iostream>

typedef boost::wave::token_id TokenId;

inline bool isEOF(TokenId token)
{
	return IS_CATEGORY(token, boost::wave::EOFTokenType);
}

inline bool isWhiteSpace(TokenId token)
{
	return IS_CATEGORY(token, boost::wave::WhiteSpaceTokenType)
		|| IS_CATEGORY(token, boost::wave::EOLTokenType);
}

struct Token
{
	TokenId id;
	const char* value;
	Token(TokenId id, const char* value)
		: id(id), value(value)
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

	TokenId get_id()
	{
		return position != history.end() ? (*position).id : ::get_id(dereference(first));
	}
	const char* get_value()
	{
		return position != history.end() ? (*position).value : ::get_value(dereference(first));
	}

	void increment()
	{
		if(position != history.end())
		{
			++position;
		}
		else
		{
			history.push_back(Token(get_id(), makeIdentifier(get_value())));
			position = history.end();
			for(;;)
			{
				if(first == last
					|| isEOF(get_id()))
				{
					throw LexError();
				}
				::increment(first);
				if(!isWhiteSpace(get_id()))
				{
					break;
				}
			}
		}
	}
};

struct Parser
{
	Scanner& scanner;
	size_t position;

	Parser(Scanner& scanner)
		: scanner(scanner), position(0)
	{
	}
	Parser(const Parser& other)
		: scanner(other.scanner), position(0)
	{
	}

	TokenId get_id()
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


#define PARSE_ERROR() throw LexError()
#define PARSE_ASSERT(condition) if(!(condition)) { PARSE_ERROR(); }

inline bool isToken(TokenId token, boost::wave::token_id id)
{
	return token == id;
}

inline bool isIdentifier(TokenId token)
{
	return isToken(token, boost::wave::T_IDENTIFIER);
}

inline void parseToken(Parser& parser, boost::wave::token_id id)
{
	PARSE_ASSERT(isToken(parser.get_id(), id));
	parser.increment();
};

template<typename T>
T* createNode(T*)
{
	return new T;
}

template<typename T>
T* parseNodeInternal(Parser& parser, T* p)
{
	Parser tmp(parser);
	p = parseNode(tmp, p);
	if(p != NULL)
	{
		parser.position += tmp.position;
	}
	else
	{
		tmp.backtrack();
	}
	return p;
}

#define TOKEN_EQUAL(parser, token) isToken(parser.get_id(), token)
#define PARSE_TOKEN_REQUIRED(parser, token) if(TOKEN_EQUAL(parser, token)) { parser.increment(); } else { return NULL; }
#define PARSE_TOKEN_OPTIONAL(parser, result, token) result = false; if(TOKEN_EQUAL(parser, token)) { result = true; parser.increment(); }
#define PARSE_SELECT_TOKEN(parser, p, token, value_) if(TOKEN_EQUAL(parser, token)) { p->value = value_; parser.increment(); return p; }
#define PARSE_OPTIONAL(parser, p) (p) = parseNodeInternal(parser, createNode(p))
#define PARSE_REQUIRED(parser, p) if(((p) = parseNodeInternal(parser, createNode(p))) == 0) { return NULL; }
#define PARSE_SELECT(parser, Type) if(Type* p = parseNodeInternal(parser, new Type)) { std::cout << "matched: " << #Type << std::endl; return p; }
// Type must have members 'left' and 'right', and 'typeof(left)' must by substitutable for 'Type'
#define PARSE_PREFIX(parser, Type) if(Type* p = parseNodeInternal(parser, new Type)) { if(p->right == NULL) return p->left; return p; }

cpp::declaration_seq* parseFile(std::string& instring, const char* input);
cpp::statement_seq* parseFunction(std::string& instring, const char* input);

#endif


