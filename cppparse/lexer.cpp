
#include "lexer.h"


#include <string.h> // strrchr

///////////////////////////////////////////////////////////////////////////////
//  Include Wave itself
#include <boost/wave.hpp>

///////////////////////////////////////////////////////////////////////////////
// Include the lexer stuff
#include <boost/wave/cpplexer/cpp_lex_token.hpp>    // token class
#include <boost/wave/cpplexer/cpp_lex_iterator.hpp> // lexer class

//  This token type is one of the central types used throughout the library, 
//  because it is a template parameter to some of the public classes and  
//  instances of this type are returned from the iterators.
typedef boost::wave::cpplexer::lex_token<boost::wave::util::file_position_type> token_type;

//  The template boost::wave::cpplexer::lex_iterator<> is the lexer type to
//  to use as the token source for the preprocessing engine. It is 
//  parametrized with the token type.
typedef boost::wave::cpplexer::lex_iterator<token_type> lex_iterator_type;

typedef boost::wave::iteration_context_policies::load_file_to_string input_policy_type;

class Hooks : public boost::wave::context_policies::eat_whitespace<token_type>
{
public:
	std::string includes[1024];
	size_t depth;

	Hooks() : depth(1)
	{
		includes[0] = "$outer";
	}

	template <typename ContextT>
	void opened_include_file(ContextT const &ctx, 
		std::string const &relname, std::string const& absname,
		bool is_system_include)
	{
		const char* path = strrchr(relname.c_str(), '/');
		if(path == 0)
		{
			path = relname.c_str();
		}
		else
		{
			++path;
		}
		std::cout << "  included: " << path;
		std::cout << " from: " << includes[depth - 1];
		std::cout << std::endl;
		includes[depth] = path;
		++depth;
	}
	template <typename ContextT>
	void returning_from_include_file(ContextT const &ctx)
	{
		--depth;
		std::cout << "  return: " << includes[depth - 1];
		std::cout << " from: " << includes[depth];
		std::cout << std::endl;
	}
};

//  This is the resulting context type to use. The first template parameter
//  should match the iterator type to be used during construction of the
//  corresponding context object (see below).
typedef boost::wave::context<std::string::iterator, lex_iterator_type, input_policy_type, Hooks>
context_type;

//  The preprocessor iterator shouldn't be constructed directly. It is 
//  to be generated through a wave::context<> object. This wave:context<> 
//  object is to be used additionally to initialize and define different 
//  parameters of the actual preprocessing (not done here).
//
//  The preprocessing of the input stream is done on the fly behind the 
//  scenes during iteration over the context_type::iterator_type stream.

struct LexContext : public context_type
{
	LexContext(std::string& instring, const char* input)
		: context_type(instring.begin(), instring.end(), input)
	{
		set_language(boost::wave::language_support(
			boost::wave::support_normal
			| boost::wave::support_option_variadics
			| (boost::wave::support_option_mask &
				~(boost::wave::support_option_emit_line_directives
					| boost::wave::support_option_single_line
					| boost::wave::support_option_emit_pragma_directives))));
	}
};

struct LexIterator : public context_type::iterator_type
{
	LexIterator(const context_type::iterator_type& iterator)
		: context_type::iterator_type(iterator)
	{
	}
};

struct LexToken : public token_type
{
	LexToken(const token_type& token)
		: token_type(token)
	{
	}
};

LexToken& makeToken(token_type& token)
{
	return *static_cast<LexToken*>(&token);
}

const context_type::iterator_type& makeBase(const LexIterator& i)
{
	return *static_cast<const context_type::iterator_type*>(&i);
}

context_type::iterator_type& makeBase(LexIterator& i)
{
	return *static_cast<context_type::iterator_type*>(&i);
}


LexContext& createContext(std::string& instring, const char* input)
{
	return *(new LexContext(instring, input));
}

bool add_include_path(LexContext& context, char const *path)
{
	return context.add_include_path(path);
}

bool add_sysinclude_path(LexContext& context, char const *path)
{
	return context.add_sysinclude_path(path);
}

bool add_macro_definition(LexContext& context, const char* macrostring, bool is_predefined)
{
	return context.add_macro_definition(macrostring, is_predefined);
}

void release(LexContext& context)
{
	delete &context;
}

LexIterator& createBegin(LexContext& lexer)
{
	return *(new LexIterator(lexer.begin()));
}

LexIterator& createEnd(LexContext& lexer)
{
	return *(new LexIterator(lexer.end()));
}

LexIterator& cloneIterator(LexIterator& i)
{
	return *(new LexIterator(i));
}

void assignIterator(LexIterator& i, LexIterator& other)
{
	i = other;
}

void release(LexIterator& i)
{
	delete &i;
}

bool operator==(const LexIterator& l, const LexIterator& r)
{
	return makeBase(l) == makeBase(r);
}

void increment(LexIterator& i)
{
	try {
		++makeBase(i);
	}
	catch (boost::wave::cpp_exception const& e) {
		// some preprocessing error
		std::cerr 
			<< e.file_name() << "(" << e.line_no() << "): "
			<< e.description() << std::endl;
		throw LexError();
	}
}

const LexToken& dereference(const LexIterator& i)
{
	try {
		return makeToken(*i);
	}
	catch (boost::wave::cpp_exception const& e) {
		// some preprocessing error
		std::cerr 
			<< e.file_name() << "(" << e.line_no() << "): "
			<< e.description() << std::endl;
		throw LexError();
	}
}

const char* get_value(const LexToken& token)
{
	return token.get_value().c_str();
}

LexTokenId get_id(const LexToken& token)
{
	return token;
}

const LexFilePosition& get_position(const LexToken& token)
{
	return token.get_position();
}
