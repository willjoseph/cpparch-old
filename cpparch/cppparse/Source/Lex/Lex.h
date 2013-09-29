
#ifndef INCLUDED_CPPPARSE_LEX_LEX_H
#define INCLUDED_CPPPARSE_LEX_LEX_H

#include <fstream>

#include "Common/Common.h"
#include "Token.h"
#include "TokenPrinter.h"
#include "Common/Profile.h"
#include "Common/Allocator.h"


typedef boost::wave::token_id LexTokenId;


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

LexContext& createContext(std::ifstream& instring, const char* input);
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
#if 0
const char* get_value(const LexToken& token);
LexTokenId get_id(const LexToken& token);
const LexFilePosition& get_position(const LexToken& token);
#endif

#include <vector>
#include <set>
#include <map>
#include <iostream>
#include <string.h> // strlen
#include <algorithm> // std::min


typedef std::set<const struct IncludeDependencyNode*> IncludeDependencyNodes;

struct IncludeDependencyNode : public IncludeDependencyNodes
{
	Name name;
	IncludeDependencyNode(Name name)
		: name(name)
	{
	}
};

inline bool operator<(const IncludeDependencyNode& left, const IncludeDependencyNode& right)
{
	return left.name < right.name;
}

typedef std::pair<Name, const char*> MacroDeclaration; // first=source, second=name
typedef std::set<MacroDeclaration> MacroDeclarationSet;
const MacroDeclarationSet MACRODECLARATIONSET_NULL = MacroDeclarationSet();
typedef std::map<Name, MacroDeclarationSet> MacroDependencyMap; // key=source

struct IncludeDependencyGraph
{
	typedef std::set<IncludeDependencyNode> Includes;
	Includes includes;

	MacroDependencyMap macros;

	IncludeDependencyNode& get(Name name)
	{
		Includes::iterator i = includes.insert(name).first;
		return const_cast<IncludeDependencyNode&>(*i);
	}
};



inline bool isEOF(LexTokenId token)
{
	return IS_CATEGORY(token, boost::wave::EOFTokenType);
}

inline bool isWhiteSpace(LexTokenId token)
{
	return IS_CATEGORY(token, boost::wave::WhiteSpaceTokenType)
		|| IS_CATEGORY(token, boost::wave::EOLTokenType);
}

struct Token
{
	LexTokenId id;
	TokenValue value;
	FilePosition position;
	Source source;
	IncludeEvents events;

	Token()
		: id(boost::wave::T_UNKNOWN)
	{
	}
	Token(LexTokenId id, const TokenValue& value, const FilePosition& position, Source source = Source(Path(Name("\"$unknown\""), Name("$unknown")), 0, 0), IncludeEvents events = IncludeEvents())
		: id(id), value(value), position(position), events(events), source(source)
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

typedef TokenPrinter<std::ostream> FileTokenPrinter;


struct Lexer
{
	LexContext& context;
	std::ofstream out;
	FileTokenPrinter printer;

	LexIterator& first;
	LexIterator& last;

	enum { BACKTRACK_MAX = 1024 * 128 };
	typedef BacktrackBuffer Tokens;
	Tokens history;
	Tokens::const_iterator position;
	Tokens::const_iterator error;

	typedef std::vector<size_t> Positions;
	Positions stacktrace;
	Positions::iterator stackpos;

	BacktrackStats stats;
	bool maxBacktrack;

	Lexer(LexContext& context, const char* path)
		: context(context),
		out(path),
		printer(out),
		first(createBegin(context)),
		last(createEnd(context)),
		history(BACKTRACK_MAX),
		position(history.end()),
		error(history.end()),
		stackpos(stacktrace.end()),
		maxBacktrack(false)
	{
#ifdef _DEBUG
		depth = 0;
		includes[depth++] = Name("<root>");
#endif
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

	bool finished() const
	{
		return isEOF((*position).id) && first == last;
	}

	LexTokenId get_id()
	{
		return (*position).id;
	}
	const TokenValue& get_value()
	{
		return (*position).value;
	}
	const FilePosition& get_position()
	{
		return (*position).position;
	}
	IncludeEvents get_events()
	{
		return (*position).events;
	}
	const Source& get_source()
	{
		return (*position).source;
	}
	const TokenValue& getErrorValue()
	{
		return (*error).value;
	}
	FilePosition getErrorPosition()
	{
		return (*error).position;
	}

#ifdef _DEBUG
	Name includes[1024];
	size_t depth;

	void debugEvents(IncludeEvents events, Name source);
#endif

	Token* read(Token* first, Token* last);
	void refill()
	{
		if(position == history.end())
		{
			PROFILESCOPE_DISABLECOLLECTION(profile);
			const size_t COUNT = 1024*16;//BACKTRACK_MAX >> 1;
			Token* end = history.TokenBuffer::end();
			history.position = read(history.position, std::min(end, history.position + COUNT));
			if(history.position == end)
			{
				history.position = history.TokenBuffer::begin();
			}
		}
#ifdef _DEBUG
		debugEvents((*position).events, (*position).source.absolute);
#endif
	}

	void increment()
	{
		position = ::next(history, position);
		refill();
	}

	const IncludeDependencyGraph& getIncludeGraph() const;
	const char* makeIdentifier(const char* value);
};

#if 0
inline void printPosition(const LexFilePosition& position)
{
	std::cout << position.get_file().c_str() << "(" << position.get_line() << "): ";
}
#endif

inline void printPosition(const FilePosition& position)
{
	std::cout << position.file.c_str() << "(" << position.line << "): ";
}

inline void printPosition(const Source& source)
{
	std::cout << source.absolute.c_str() << "(" << source.line << "): ";
}

#endif


