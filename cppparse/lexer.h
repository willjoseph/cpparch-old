
#ifndef INCLUDED_CPPPARSE_LEXER_H
#define INCLUDED_CPPPARSE_LEXER_H

#include <string>
#include <fstream>

#include "printer.h"

#include <boost/wave/token_ids.hpp>
#include <boost/wave/util/file_position.hpp>

struct LexContext;
struct LexIterator;
struct LexToken;

typedef boost::wave::token_id LexTokenId;
typedef boost::wave::util::file_position_type LexFilePosition;

struct LexError
{
	LexError()
	{
	}
};

LexContext& createContext(std::string& instring, const char* input);
bool add_include_path(LexContext& context, const char* path);
bool add_sysinclude_path(LexContext& context, const char* path);
bool add_macro_definition(LexContext& context, const char* macroname, bool is_predefined);
void release(LexContext& context);

LexIterator& createBegin(LexContext& lexer);
LexIterator& createEnd(LexContext& lexer);
LexIterator& cloneIterator(LexIterator& i);
void assignIterator(LexIterator& i, LexIterator& other);
void release(LexIterator& i);
bool operator==(const LexIterator& l, const LexIterator& r);
inline bool operator!=(const LexIterator& l, const LexIterator& r)
{
	return !(l == r);
}
void increment(LexIterator& i);
const LexToken& dereference(const LexIterator& i);
const char* get_value(const LexToken& token);
LexTokenId get_id(const LexToken& token);
LexFilePosition get_position(const LexToken& token);


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

typedef TokenPrinter<std::ofstream> FileTokenPrinter;

struct Lexer
{
	std::ofstream out;
	FileTokenPrinter printer;

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

	Lexer(LexContext& context, const char* path)
		: out(path),
		printer(out),
		first(createBegin(context)),
		last(createEnd(context)),
		position(history.end()),
		stackpos(stacktrace.end()),
		maxBacktrack(false)
	{
		if(isWhiteSpace(get_id()))
		{
			increment();
		}
	}
	~Lexer()
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
		return position != history.end() ? (*position).value : makeIdentifier(::get_value(dereference(first)));
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
			printer.printToken(get_id(), get_value());
			history.push_back(Token(get_id(), get_value(), get_position()));
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

#endif


