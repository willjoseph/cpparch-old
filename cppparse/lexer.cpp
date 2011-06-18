
#include "lexer.h"

#include "profiler.h"

#include "util.h"

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

struct LoadFile
{
    template <typename IterContextT>
	struct inner : public boost::wave::iteration_context_policies::load_file_to_string::inner<IterContextT>
	{
            template <typename PositionT>
            static void init_iterators(IterContextT &iter_ctx, 
				PositionT const &act_pos, boost::wave::language_support language)
            {
				ProfileScope profile(gProfileIo);
				boost::wave::iteration_context_policies::load_file_to_string::inner<IterContextT>::init_iterators(iter_ctx, act_pos, language);
			}
	};
};

typedef LoadFile input_policy_type;

struct LexNames
{
	typedef std::set<std::string, std::less<std::string>, DebugAllocator<std::string> > Identifiers;
	Identifiers identifiers;
	Identifiers filenames;

	const char* makeIdentifier(const char* value)
	{
		ProfileScope profile(gProfileIdentifier);
		return (*identifiers.insert(value).first).c_str();
	}
	const char* makeFilename(const char* value)
	{
		ProfileScope profile(gProfileIdentifier);
		std::string tmp(value);
		for(std::string::iterator i = tmp.begin(); i != tmp.end(); ++i)
		{
			if(*i == '/')
			{
				*i = '\\';
			}
		}
		return (*filenames.insert(tmp).first).c_str();
	}
	FilePosition makeFilePosition(const LexFilePosition& position)
	{
		FilePosition result = {
			makeFilename(position.get_file().c_str()),
			position.get_line(),
			position.get_column()
		};
		return result;
	}
};


class Hooks : public boost::wave::context_policies::eat_whitespace<token_type>
{
public:
	LexNames& names;
	const char* includes[1024];
	size_t depth;

	size_t macroDepth;
	FilePosition macroPosition;

	IncludeDependencyGraph dependencies;
	IncludeDependencyGraph includeGraph;

	IncludeEvents events;

	StringStack ifdef;

	Hooks(LexNames& names)
		: names(names), depth(1), macroDepth(0)
	{
		includes[0] = "$outer";
	}

    // new signature
	template <typename ContextT, typename TokenT>
    bool found_directive(ContextT const& ctx, TokenT const& directive)
    {
#if 0
		if(directive == boost::wave::T_PP_ENDIF)
		{
			//std::cout << "#endif" << std::endl;
			ifdef.pop();
		}
		else if(directive == boost::wave::T_PP_ELSE)
		{
			//std::cout << "#else" << std::endl;
			ifdef.top() = 0;
		}
#endif
		return false; // process this directive
	}

    template <typename ContextT, typename TokenT, typename ContainerT>
    bool evaluated_conditional_expression(ContextT const& ctx, 
        TokenT const& directive, ContainerT const& expression, 
        bool expression_value)
    {
#if 0
		if(directive == boost::wave::T_PP_IFDEF) // this block only processed if next token is name of a defined macro
		{
#if 0
			std::cout << "#ifdef:";
			for(ContainerT::const_iterator i = expression.begin(); i != expression.end(); ++i)
			{
				std::cout << " " << (*i).get_value();
			}
			std::cout << std::endl;
#endif

			ifdef.push(names.makeIdentifier(expression.front().get_value().c_str()));
		}
		else if(directive == boost::wave::T_PP_IFNDEF)
		{
			//std::cout << "#ifndef" << std::endl;
			ifdef.push(0);
		}
		else if(directive == boost::wave::T_PP_IF)
		{
			//std::cout << "#if" << std::endl;
			const char* name = 0;
#if 0
			std::cout << "#if:";
			for(ContainerT::const_iterator i = expression.begin(); i != expression.end(); ++i)
			{
				std::cout << " " << (*i).get_value();
			}
			std::cout << std::endl;
#endif
			{
				typename ContainerT::const_iterator i = expression.begin();
				if(string_equal((*i).get_value().c_str(), "defined"))
				{
					++i;
					if(*i == boost::wave::T_LEFTPAREN)
					{
						++i;
					}
					name = names.makeIdentifier((*i).get_value().c_str());
				}
			}
			ifdef.push(name);
		}
		else if(directive == boost::wave::T_PP_ELIF)
		{
			//std::cout << "#elif" << std::endl;
			ifdef.top() = 0;
		}
#endif
		return false;         // ok to continue, do not re-evaluate expression
	}

	template <typename ContextT>
    bool found_include_directive(ContextT const& ctx, std::string const& filename, bool include_next) 
    {
        return false;    // ok to include this file
    }

	template <typename ContextT>
	void resolved_include_file(ContextT const &ctx, 
		std::string const &relname, std::string const& absname,
		bool is_system_include)
	{
		IncludeDependencyNode& d = includeGraph.get(names.makeFilename(absname.c_str()));
		//d.macro = names.makeIdentifier(macrodef.get_value().c_str());
		includeGraph.get(getSourcePath()).insert(&d);
	}
	template <typename ContextT>
	void opened_include_file(ContextT const &ctx, 
		std::string const &relname, std::string const& absname,
		bool is_system_include)
	{
		includes[depth] = names.makeFilename(absname.c_str());
		//LEXER_ASSERT(std::find(includes, includes + depth, includes[depth]) == includes + depth); // cyclic includes! 
		std::cout << "lexer: " << findFilename(includes[depth - 1]) << "  included: " << findFilename(includes[depth]) << std::endl;
		++depth;
		++events.push;
	}
	template <typename ContextT>
	void returning_from_include_file(ContextT const &ctx)
	{
#if 0
		{
			IncludeDependencyNode& d = includeGraph.get(getSourcePath());
			for(IncludeDependencyNode::iterator i = d.begin(); i != d.end(); ++i)
			{
				std::cout << "    " << (*i)->name << std::endl;
			}
		}
#endif
#if 0
		{
			IncludeDependencyNode& d = dependencies.get(getSourcePath());
			for(IncludeDependencyNode::iterator i = d.begin(); i != d.end(); ++i)
			{
				std::cout << "    " << (*i)->name << std::endl;
			}
		}
#endif

		--depth;
		std::cout << "lexer: " << findFilename(includes[depth - 1]) << "  returned: " << findFilename(includes[depth]) << std::endl;
		if(events.push != 0)
		{
			--events.push;
		}
		else
		{
			++events.pop;
		}
	}

	bool isConditional(const char* name)
	{
		for(StringStack::const_iterator i = ifdef.begin(); i != ifdef.end(); ++i)
		{
			if(*i == name)
			{
				return true;
			}
		}
		return false;
	}

	template<typename ContextT, typename TokenT>
	void expanding_macro(ContextT& ctx, const TokenT& macrodef, const TokenT& macrocall)
	{
		if(++macroDepth == 1)
		{
			macroPosition = names.makeFilePosition(macrocall.get_position());
		}
		const char* defPath = names.makeFilename(macrodef.get_position().get_file().c_str());
		IncludeDependencyNode& d = dependencies.get(defPath);
		//d.macro = names.makeIdentifier(macrodef.get_value().c_str());
		dependencies.get(getSourcePath()).insert(&d);

		if(defPath != getSourcePath()) // if the macro being called is within a different file
		{
			const char* name = names.makeIdentifier(macrodef.get_value().c_str());
#if 0
			if(isConditional(name)) // if the call to macro X is within an '#ifdef X' block
			{
				// the file containing the declaration of the called macro is implicitly included by the current file
				// TODO: perform this when 'ifdef X' is encountered, if X was defined in another file
				IncludeDependencyNode& d = includeGraph.get(defPath);
				includeGraph.get(getSourcePath()).insert(&d);
			}
#endif
			// the current file depends on the file containing the declaration of the called macro
			includeGraph.macros[getSourcePath()].insert(MacroDeclaration(defPath, name));
		}
	}
    template <typename ContextT, typename TokenT, typename ContainerT, typename IteratorT>
    bool 
    expanding_function_like_macro(ContextT& ctx,
        TokenT const& macrodef, std::vector<TokenT> const& formal_args, 
        ContainerT const& definition,
        TokenT const& macrocall, std::vector<ContainerT> const& arguments,
        IteratorT const& seqstart, IteratorT const& seqend) 
    {
		expanding_macro(ctx, macrodef, macrocall);
		return false;
	}
    template <typename ContextT, typename TokenT, typename ContainerT>
    bool 
    expanding_object_like_macro(ContextT const& ctx, TokenT const& macrodef, 
        ContainerT const& definition, TokenT const& macrocall)
    {
		expanding_macro(ctx, macrodef, macrocall);
		return false;
	}
    template <typename ContextT, typename ContainerT>
    void rescanned_macro(ContextT const &ctx, ContainerT const &result)
	{
		LEXER_ASSERT(macroDepth != 0);
		--macroDepth;
	}

    template <
        typename ContextT, typename TokenT, typename ParametersT, 
        typename DefinitionT
    >
    void
    defined_macro(ContextT const& ctx, TokenT const& macro_name, 
        bool is_functionlike, ParametersT const& parameters, 
        DefinitionT const& definition, bool is_predefined)
    {
		//std::cout << "defined macro: " << macro_name.get_value().c_str() << std::endl;
	}

	template <typename ContextT, typename ExceptionT>
	void
	throw_exception(ContextT const& ctx, ExceptionT const& e)
	{
		boost::throw_exception(e);
	}
	template <typename ContextT>
	void
	throw_exception(ContextT const& ctx, boost::wave::preprocess_exception const& e)
	{
		std::cerr 
			<< e.file_name() << "(" << e.line_no() << "): "
			<< e.description() << std::endl;
		if(!boost::wave::is_recoverable(e))
		{
			throw LexError();
		}
	}

	const char* getSourcePath() const
	{
		return includes[depth - 1];
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

struct LexContext : public context_type, public LexNames
{
	LexContext(std::string& instring, const char* input)
		: context_type(instring.begin(), instring.end(), input, *this)
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

const IncludeDependencyGraph& Lexer::getIncludeGraph() const
{
	return context.get_hooks().includeGraph;
}

#ifdef _DEBUG
void Lexer::debugEvents(IncludeEvents events, const char* source)
{
	for(unsigned short i = 0; i != events.pop; ++i)
	{
		--depth;
		includes[depth] = 0;
	}
	for(unsigned short i = 0; i != events.push; ++i)
	{
		includes[depth++] = 0;
	}
	if(includes[depth - 1] == 0)
	{
		includes[depth - 1] = source;
		std::cout << "parser: " << findFilenameSafe(includes[depth - 1]) << std::endl;
	}
}
#endif

Token* Lexer::read(Token* first, Token* last)
{
	try
	{
		ProfileScope profile(gProfileWave);
		for(; this->first != this->last; ++makeBase(this->first))
		{
			const token_type& token = *makeBase(this->first);
			if(!isWhiteSpace(token))
			{
				if(isEOF(token))
				{
					continue; // note: not passing EOF through, as Wave appears to generate EOF tokens mid-stream
				}
				if(first == last)
				{
					break;
				}
#ifdef _DEBUG
				{
					ProfileScope profile(gProfileDiagnose);
					printer.printToken(token, token.get_value().c_str());
				}
#endif
				FilePosition position = context.get_hooks().macroDepth == 0
					? context.makeFilePosition(token.get_position())
					: context.get_hooks().macroPosition;
				*first++ = Token(token, context.makeIdentifier(token.get_value().c_str()), position, context.get_hooks().getSourcePath(), context.get_hooks().events);

				//debugEvents(context.get_hooks().events, context.get_hooks().getSourcePath());

				context.get_hooks().events = IncludeEvents();
			}
		}
		// if reached end of token stream, append EOF
		if(this->first == this->last
			&& first != last)
		{
			*first++ = Token(boost::wave::T_EOF, "", FilePosition(), context.get_hooks().getSourcePath(), context.get_hooks().events);

			//debugEvents(context.get_hooks().events, context.get_hooks().getSourcePath());
		}
	}
	catch (boost::wave::cpp_exception const& e) {
		// some preprocessing error
		std::cerr 
			<< e.file_name() << "(" << e.line_no() << "): "
			<< e.description() << std::endl;
		return read(first, last);
	}
	return first;
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
