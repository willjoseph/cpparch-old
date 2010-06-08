
#ifndef INCLUDED_CPPPARSE_LEXER_H
#define INCLUDED_CPPPARSE_LEXER_H

#include <boost/wave/token_ids.hpp>
#include <boost/wave/util/file_position.hpp>

#include <string>
#include <fstream>

#include "common.h"
#include "printer.h"
#include "profiler.h"
#include "allocator.h"


typedef boost::wave::token_id LexTokenId;
typedef boost::wave::util::file_position_type LexFilePosition;

struct LexContext;
struct LexIterator;
struct LexToken;

struct LexError
{
	LexError()
	{
	}
};

#define LEXER_ASSERT(condition) if(!(condition)) { throw LexError(); }

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
#if 0
void increment(LexIterator& i);
const LexToken& dereference(const LexIterator& i);
const char* get_value(const LexToken& token);
LexTokenId get_id(const LexToken& token);
const LexFilePosition& get_position(const LexToken& token);
#endif

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
	FilePosition position;

	Token()
		: id(boost::wave::T_UNKNOWN)
	{
	}
	Token(LexTokenId id, const char* value, const FilePosition& position)
		: id(id), value(value), position(position)
	{
	}
};


struct BacktrackStats
{
	size_t count;
	const char* symbol;
	FilePosition position;
	BacktrackStats()
		: count(0)
	{
	}
};



class TokenBuffer
{
	TokenBuffer(const TokenBuffer&);
	TokenBuffer& operator=(const TokenBuffer&);
public:
	Token* tokens;
	size_t m_size;
	TokenBuffer()
		: tokens(0), m_size(0)
	{
	}
	TokenBuffer(size_t size)
		: tokens(new Token[size]), m_size(size)
	{
	}
	~TokenBuffer()
	{
		delete[] tokens;
	}

	Token* begin()
	{
		return tokens;
	}
	const Token* begin() const
	{
		return tokens;
	}
	Token* end()
	{
		return tokens + m_size;
	}
	const Token* end() const
	{
		return tokens + m_size;
	}

	void resize(size_t count)
	{
		this->~TokenBuffer();
		new(this) TokenBuffer(count);
	}

	void swap(TokenBuffer& other)
	{
		std::swap(tokens, other.tokens);
		std::swap(m_size, other.m_size);
	}
};


struct BacktrackBuffer : public TokenBuffer
{
	Token* position;

	typedef Token* iterator;
	typedef const Token* const_iterator;

	BacktrackBuffer() : position(0)
	{
	}
	BacktrackBuffer(size_t count)
		: TokenBuffer(count), position(tokens)
	{
	}

	iterator next(iterator i)
	{
		++i;
		return i == TokenBuffer::end() ? TokenBuffer::begin() : i;
	}
	const_iterator next(const_iterator i) const
	{
		++i;
		return i == TokenBuffer::end() ? TokenBuffer::begin() : i;
	}

	size_t distance(const_iterator i, const_iterator other) const
	{
		return (i > other) ? m_size - (i - other) : other - i;
	}

	const_iterator backtrack(const_iterator i, size_t count)
	{
		return (count > size_t(i - tokens)) ? i + (m_size - count) : i - count;
	}
	const_iterator advance(const_iterator i, size_t count)
	{
		return (i + count < tokens + m_size) ? i + count : i - (m_size - count);
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
	size_t size() const
	{
		return m_size;
	}

	void resize(size_t count)
	{
		TokenBuffer::resize(count);
		position = tokens;
	}
	void swap(BacktrackBuffer& other)
	{
		TokenBuffer::swap(other);
		std::swap(position, other.position);
	}
};

inline size_t distance(const BacktrackBuffer& buffer, BacktrackBuffer::const_iterator first, BacktrackBuffer::const_iterator last)
{
	return buffer.distance(first, last);
}

inline BacktrackBuffer::const_iterator next(const BacktrackBuffer& buffer, BacktrackBuffer::const_iterator position)
{
	return buffer.next(position);
}

inline BacktrackBuffer::const_iterator backtrack(BacktrackBuffer& buffer, BacktrackBuffer::const_iterator position, size_t count)
{
	return buffer.backtrack(position, count);
}

inline BacktrackBuffer::const_iterator advance(BacktrackBuffer& buffer, BacktrackBuffer::const_iterator position, size_t count)
{
	return buffer.advance(position, count);
}

typedef TokenPrinter<std::ofstream> FileTokenPrinter;


typedef LinearAllocator<true> LexerAllocator;
struct Lexer
{
	std::ofstream out;
	FileTokenPrinter printer;

	LexerAllocator allocator;

	LexIterator& first;
	LexIterator& last;

	enum { BACKTRACK_MAX = 1024 * 128 };
	typedef BacktrackBuffer Tokens;
	Tokens history;
	Tokens::const_iterator position;
	Tokens::const_iterator error;

	typedef std::set<std::string, std::less<std::string>, DebugAllocator<std::string> > Identifiers;
	Identifiers identifiers;
	Identifiers filenames;

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
		history(BACKTRACK_MAX),
		position(history.end()),
		error(history.end()),
		stackpos(stacktrace.end()),
		maxBacktrack(false)
	{
		refill();
	}
	~Lexer()
	{
		release(first);
		release(last);
	}
	bool canBacktrack(size_t count)
	{
		return ::distance(history, position, history.end()) + count < history.size();
	}
	void backtrack(size_t count, const char* symbol = 0)
	{
		if(count == 0)
		{
			return;
		}
		if(!canBacktrack(count))
		{
			maxBacktrack = true;
		}
		else
		{
			if(::distance(history, position, history.end()) < ::distance(history, error, history.end()))
			{
				error = position;
			}
			position = ::backtrack(history, position, count);
#ifdef _DEBUG
			if(count > stats.count
				&& symbol != 0)
			{
				stats.count = count;
				stats.symbol = symbol;
				stats.position = (*position).position;
			};
#endif
		}
	}
	void advance(size_t count)
	{
		if(count == 0)
		{
			return;
		}
		position = ::advance(history, position, count);
	}

	const char* makeIdentifier(const char* value)
	{
		ProfileScope profile(gProfileIdentifier);
		return (*identifiers.insert(value).first).c_str();
	}
	FilePosition makeFilePosition(const LexFilePosition& position)
	{
		ProfileScope profile(gProfileIdentifier);
		FilePosition result = {
			(*filenames.insert(position.get_file().c_str()).first).c_str(),
			position.get_line(),
			position.get_column()
		};
		return result;
	}

	bool finished() const
	{
		return position == history.end() && first == last;
	}

	LexTokenId get_id()
	{
		return (*position).id;
	}
	const char* get_value()
	{
		return (*position).value;
	}
	FilePosition get_position()
	{
		return (*position).position;
	}
	const char* getErrorValue()
	{
		return (*error).value;
	}
	FilePosition getErrorPosition()
	{
		return (*error).position;
	}

	Token* read(Token* first, Token* last);
	void refill()
	{
		if(position == history.end())
		{
			const size_t COUNT = 4;
			Token* end = history.TokenBuffer::end();
			history.position = read(history.position, std::min(end, history.position + COUNT));
			if(history.position == end)
			{
				history.position = history.TokenBuffer::begin();
			}
		}
	}

	void increment()
	{
		position = ::next(history, position);
		refill();
	}
};

inline void printPosition(const LexFilePosition& position)
{
	std::cout << position.get_file() << "(" << position.get_line() << "): ";
}

inline void printPosition(const FilePosition& position)
{
	std::cout << position.file << "(" << position.line << "): ";
}

#endif


