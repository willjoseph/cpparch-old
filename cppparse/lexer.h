
#include <string>

#include <boost/wave/token_ids.hpp>

struct LexContext;
struct LexIterator;
struct LexToken;

typedef boost::wave::token_id LexTokenId;

struct LexError
{
};

LexContext& createContext(std::string& instring, const char* input);
LexIterator& createBegin(LexContext& lexer);
LexIterator& createEnd(LexContext& lexer);
bool operator==(const LexIterator& l, const LexIterator& r);
inline bool operator!=(const LexIterator& l, const LexIterator& r)
{
	return !(l == r);
}
void increment(LexIterator& i);
const LexToken& dereference(const LexIterator& i);
const char* get_value(const LexToken& token);
LexTokenId get_id(const LexToken& token);



