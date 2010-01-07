
#ifndef INCLUDED_CPPPARSE_LEXER_H
#define INCLUDED_CPPPARSE_LEXER_H

#include <string>

#include <boost/wave/token_ids.hpp>

struct LexContext;
struct LexIterator;
struct LexToken;

typedef boost::wave::token_id LexTokenId;

struct LexError
{
	LexError()
	{
	}
};

LexContext& createContext(std::string& instring, const char* input);
void release(LexContext& context);
LexIterator& createBegin(LexContext& lexer);
LexIterator& createEnd(LexContext& lexer);
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

#endif


