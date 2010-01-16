
#ifndef INCLUDED_CPPPARSE_LEXER_H
#define INCLUDED_CPPPARSE_LEXER_H

#include <string>

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

#endif


