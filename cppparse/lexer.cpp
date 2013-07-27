
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

#if 0 // WIP
struct LexFilename
{
	typedef BOOST_WAVE_STRINGTYPE::size_type size_type;
	typedef BOOST_WAVE_STRINGTYPE::value_type value_type;
	typedef BOOST_WAVE_STRINGTYPE::reference reference;
	typedef BOOST_WAVE_STRINGTYPE::const_reference const_reference;
	static const size_type npos =size_type(-1);


	BOOST_WAVE_STRINGTYPE value;

	LexFilename()
	{
	}
	LexFilename(const char* value)
		: value(value)
	{
	}
	const char* c_str() const
	{
		return value.c_str();
	}
	size_type find_first_of(const char* s, size_type pos = 0) const
	{
		return value.find_first_of(s, pos);
	}
	const_reference operator[](size_type pos) const
	{
		return value[pos];
	}
	reference operator[](size_type pos)
	{
		return value[pos];
	}

};

struct LexFilePosition : boost::wave::util::file_position<LexFilename>
{
	typedef boost::wave::util::file_position<LexFilename> Base;
	LexFilePosition()
	{
	}
	LexFilePosition(LexFilename const& file, unsigned int line = 1, unsigned int column = 1)
		: Base(file, line, column)
	{
	}
};
#elif 0
struct LexFilePosition : boost::wave::util::file_position<BOOST_WAVE_STRINGTYPE>
{
	typedef boost::wave::util::file_position<BOOST_WAVE_STRINGTYPE> Base;

	std::size_t offset;

	LexFilePosition()
	{
	}
	LexFilePosition(BOOST_WAVE_STRINGTYPE const& file, unsigned int line = 1, unsigned int column = 1, std::size_t offset = 0)
		: Base(file, line, column), offset(offset)
	{
	}
};
#else
typedef boost::wave::util::file_position<BOOST_WAVE_STRINGTYPE> LexFilePosition;
#endif


//  This token type is one of the central types used throughout the library, 
//  because it is a template parameter to some of the public classes and  
//  instances of this type are returned from the iterators.
typedef boost::wave::cpplexer::lex_token<LexFilePosition> token_type;

//  The template boost::wave::cpplexer::lex_iterator<> is the lexer type to
//  to use as the token source for the preprocessing engine. It is 
//  parametrized with the token type.
typedef boost::wave::cpplexer::lex_iterator<token_type> lex_iterator_type;

typedef std::basic_string<char, std::char_traits<char>, DebugAllocator<char> > LexString;

LexTokenId LEXER_INTRINSIC_IDS[] = {
	boost::wave::T_HAS_NOTHROW_CONSTRUCTOR,
	boost::wave::T_HAS_NOTHROW_COPY,
	boost::wave::T_HAS_TRIVIAL_ASSIGN,
	boost::wave::T_HAS_TRIVIAL_CONSTRUCTOR,
	boost::wave::T_HAS_TRIVIAL_COPY,
	boost::wave::T_HAS_TRIVIAL_DESTRUCTOR,
	boost::wave::T_HAS_VIRTUAL_DESTRUCTOR,
	boost::wave::T_IS_ABSTRACT,
	boost::wave::T_IS_BASE_OF,
	boost::wave::T_IS_CLASS,
	boost::wave::T_IS_CONVERTIBLE_TO,
	boost::wave::T_IS_EMPTY,
	boost::wave::T_IS_ENUM,
	boost::wave::T_IS_POD,
	boost::wave::T_IS_POLYMORPHIC,
	boost::wave::T_IS_UNION,
	boost::wave::T_IS_TRIVIAL,
	boost::wave::T_IS_TRIVIALLY_CONSTRUCTIBLE,
	boost::wave::T_IS_TRIVIALLY_ASSIGNABLE,
	boost::wave::T_IS_TRIVIALLY_COPYABLE,
	boost::wave::T_IS_STANDARD_LAYOUT,
	boost::wave::T_IS_LITERAL_TYPE,
	boost::wave::T_UNDERLYING_TYPE,
};


typedef const char* LexTokenValue;

LexTokenValue LEXER_INTRINSIC_VALUES[] = {
	"__has_nothrow_constructor",
	"__has_nothrow_copy",
	"__has_trivial_assign",
	"__has_trivial_constructor",
	"__has_trivial_copy",
	"__has_trivial_destructor",
	"__has_virtual_destructor",
	"__is_abstract",
	"__is_base_of",
	"__is_class",
	"__is_convertible_to",
	"__is_empty",
	"__is_enum",
	"__is_pod",
	"__is_polymorphic",
	"__is_union",
	"__is_trivial",
	"__is_trivially_constructible",
	"__is_trivially_assignable",
	"__is_trivially_copyable",
	"__is_standard_layout",
	"__is_literal_type",
	"__underlying_type",
};

struct StringEqualPredicate
{
	const char* s;
	StringEqualPredicate(const char* s)
		: s(s)
	{
	}
	bool operator()(const char* other) const
	{
		return string_equal(s, other);
	}
};

LexTokenId getLexerIntrinsicId(const char* value)
{
	const LexTokenValue* result = std::find_if(LEXER_INTRINSIC_VALUES, ARRAY_END(LEXER_INTRINSIC_VALUES), StringEqualPredicate(value));
	if(result != ARRAY_END(LEXER_INTRINSIC_VALUES))
	{
		return LEXER_INTRINSIC_IDS[result - LEXER_INTRINSIC_VALUES];
	}
	return boost::wave::T_IDENTIFIER;
}

struct LexNames
{
	typedef std::set<LexString, std::less<LexString>, DebugAllocator<int> > Identifiers;
	Identifiers identifiers;
	Identifiers filenames;

	const char* makeIdentifier(const char* value)
	{
		ProfileScope profile(gProfileIdentifier);
		return (*identifiers.insert(value).first).c_str();
	}
	Name makeFilename(const char* value)
	{
		ProfileScope profile(gProfileIdentifier);
		LexString tmp(value);
		for(LexString::iterator i = tmp.begin(); i != tmp.end(); ++i)
		{
			if(*i == '\\')
			{
				*i = '/';
			}
		}
		return Name((*filenames.insert(tmp).first).c_str());
	}
	FilePosition makeFilePosition(const LexFilePosition& position)
	{
		FilePosition result = {
			position.get_file(),
			position.get_line(),
			position.get_column()
		};
		return result;
	}
};

std::ptrdiff_t popDirectories(const char* path, std::size_t n)
{
	const char* last = findFilename(path);
	for(std::size_t i = 0; i != n; ++i)
	{
		LEXER_ASSERT(last != path);
		const char* p = last - 1;
		for(;;)
		{
			if(path == p)
			{
				return i - n;
			}
			--p;
			if(*p == '/')
			{
				last = p + 1;
				break;
			}
		}
	}
	return last - path;
}


class Hooks : public boost::wave::context_policies::eat_whitespace<token_type>
{
public:
	LexNames& names;
	Path includes[1024];
	size_t depth;
	std::string directive;

	size_t macroDepth;
	size_t macroTokenCount;
	FilePosition macroPosition;

	IncludeDependencyGraph dependencies;
	IncludeDependencyGraph includeGraph;

	IncludeEvents events;

	StringStack ifdef;

	Hooks(LexNames& names)
		: names(names), depth(1), macroDepth(0), macroTokenCount(0)
	{
		includes[0] = Path(Name(""), Name("$outer"));
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
		directive = std::string(filename.c_str() + 1, findFilename(filename.c_str() + 1));
        return false;    // ok to include this file
    }

	template <typename ContextT>
	void resolved_include_file(ContextT const &ctx, 
		std::string const &relname, std::string const& absname,
		bool is_system_include)
	{
		IncludeDependencyNode& d = includeGraph.get(names.makeFilename(absname.c_str()));
		//d.macro = names.makeIdentifier(macrodef.get_value().c_str());
		includeGraph.get(getSourcePath().absolute).insert(&d);
	}
	template <typename ContextT>
	void opened_include_file(ContextT const &ctx, 
		std::string const &relname, std::string const& absname,
		bool is_system_include)
	{
		std::size_t up = 0;
		std::vector<char> normalised;
		{
			const char* p = directive.c_str();
			for(const char* i = p; *i != '\0'; ++i)
			{
				if(*i == '\\'
					|| *i == '/')
				{
					if(i - p == 0)
					{
						// ignore "//"
					}
					if(i - p == 1
						&& *p == '.')
					{
						// ignore "/./"
					}
					else if(i - p == 2
						&& *p == '.'
						&& *(p + 1) == '.')
					{
						if(normalised.empty())
						{
							++up;
						}
						else for(;;)
						{
							normalised.pop_back();
							if(normalised.back() == '/')
							{
								break;
							}
						}
					}
					else
					{
						normalised.insert(normalised.end(), p, i);
						normalised.push_back('/');
					}
					p = i + 1;
				}
			}
		}

		Path parent = includes[depth - 1];
		Name absolute = names.makeFilename(absname.c_str());

		bool isLocal = false; // whether the included file is local to the including file

		if(!is_system_include)
		{
			if(*parent.absolute.c_str() == '$')
			{
				// special case for root
				isLocal = true;
			}
			else
			{
				std::ptrdiff_t n = popDirectories(parent.absolute.c_str(), up);
				if(n >= 0)
				{
					isLocal = std::equal(parent.absolute.c_str(), parent.absolute.c_str() + n, absolute.c_str())
						&& std::equal(normalised.begin(), normalised.end(), absolute.c_str() + n);
				}
			}
		}

		std::ptrdiff_t n = up;
		StringRange suffix = makeRange("");
		std::vector<char> root;
		if(!isLocal)
		{
			StringRange tmp(makeRange("$include/"));
			root.insert(root.end(), tmp.first, tmp.last);
		}
		else
		{
			n = popDirectories(parent.relative.c_str(), up);
			if(n > 0)
			{
				suffix = StringRange(parent.relative.c_str(), parent.relative.c_str() + n);
				n = 0;
			}
			else
			{
				n = -n;
			}
		}
		for(; n != 0; --n)
		{
			StringRange tmp(makeRange("$../"));
			root.insert(root.end(), tmp.first, tmp.last);
		}
		root.insert(root.end(), suffix.first, suffix.last);

		Name relative = names.makeFilename(Concatenate(makeRange(root), makeRange(normalised)).c_str());

		includes[depth] = Path(relative, absolute);
		//LEXER_ASSERT(std::find(includes, includes + depth, includes[depth]) == includes + depth); // cyclic includes! 
#ifdef _DEBUG
		std::cout << "lexer: " << findFilename(includes[depth - 1].c_str()) << "  included: " << findFilename(includes[depth].c_str()) << "\n";
#endif
		++depth;
		++events.push;
	}
	template <typename ContextT>
	void returning_from_include_file(ContextT const &ctx)
	{
#if 0
		{
			IncludeDependencyNode& d = includeGraph.get(getSourcePath().absolute);
			for(IncludeDependencyNode::iterator i = d.begin(); i != d.end(); ++i)
			{
				std::cout << "    " << (*i)->name << std::endl;
			}
		}
#endif
#if 0
		{
			IncludeDependencyNode& d = dependencies.get(getSourcePath().absolute);
			for(IncludeDependencyNode::iterator i = d.begin(); i != d.end(); ++i)
			{
				std::cout << "    " << (*i)->name << std::endl;
			}
		}
#endif

		--depth;
#ifdef _DEBUG
		std::cout << "lexer: " << findFilename(includes[depth - 1].c_str()) << "  returned: " << findFilename(includes[depth].c_str()) << "\n";
#endif
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
		Name defPath = names.makeFilename(macrodef.get_position().get_file().c_str());
		IncludeDependencyNode& d = dependencies.get(defPath);
		//d.macro = names.makeIdentifier(macrodef.get_value().c_str());
		dependencies.get(getSourcePath().absolute).insert(&d);

		if(defPath != getSourcePath().absolute) // if the macro being called is within a different file
		{
			const char* name = names.makeIdentifier(macrodef.get_value().c_str());
#if 0
			if(isConditional(name)) // if the call to macro X is within an '#ifdef X' block
			{
				// the file containing the declaration of the called macro is implicitly included by the current file
				// TODO: perform this when 'ifdef X' is encountered, if X was defined in another file
				IncludeDependencyNode& d = includeGraph.get(defPath);
				includeGraph.get(getSourcePath().absolute).insert(&d);
			}
#endif
			// the current file depends on the file containing the declaration of the called macro
			includeGraph.macros[getSourcePath().absolute].insert(MacroDeclarationSet::value_type(defPath, name));
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
		if(--macroDepth == 0)
		{
			LEXER_ASSERT(result.size() > 0); // TODO: replacement sequence always contains one more token than required?
			macroTokenCount = result.size() - 1;
		}
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

	Path getSourcePath() const
	{
		return includes[depth - 1];
	}
};

class FileBuffer
{
	FileBuffer(const FileBuffer&);
	FileBuffer& operator=(const FileBuffer&);
public:
	char* buffer;
	size_t length;


	FileBuffer()
		: buffer(0)
	{
	}
	FileBuffer(std::ifstream& instream)
	{
		instream.seekg(0, std::ios::end);
		length = size_t(instream.tellg());
		instream.seekg(0, std::ios::beg);
		buffer = new char[length];
		length = size_t(instream.readsome(buffer, length));
	}
	~FileBuffer()
	{
		delete[] buffer;
	}
};

struct LoadFile
{
	typedef const char* iterator_type;

	template <typename IterContextT>
	struct inner
	{
		FileBuffer file;

		template <typename PositionT>
		static void init_iterators(IterContextT &iter_ctx, 
			PositionT const &act_pos, boost::wave::language_support language)
		{
			ProfileScope profile(gProfileIo);

			typedef typename IterContextT::iterator_type iterator_type;

			// read in the file
			std::ifstream instream(iter_ctx.filename.c_str());
			if (!instream.is_open()) {
				BOOST_WAVE_THROW_CTX(iter_ctx.ctx, preprocess_exception, 
					bad_include_file, iter_ctx.filename.c_str(), act_pos);
				return;
			}
			instream.unsetf(std::ios::skipws);

			iter_ctx.file.~FileBuffer();
			new (&iter_ctx.file) FileBuffer(instream);

			iter_ctx.first = iterator_type(
				iter_ctx.file.buffer, iter_ctx.file.buffer + iter_ctx.file.length, 
				PositionT(iter_ctx.filename), language);
			iter_ctx.last = iterator_type();
		}
	};
};


struct StreamFile
{
	typedef std::istreambuf_iterator<char> base_iterator_type;
	typedef boost::spirit::multi_pass<base_iterator_type> IteratorBase;
	struct Iterator : IteratorBase
	{
		Iterator()
		{
		}
		Iterator(const base_iterator_type& other)
			: IteratorBase(other)
		{
		}
		reference operator*() const
		{
			return IteratorBase::operator*();
		}
		pointer operator->() const
		{
			return &(operator*());
		}
	};
	typedef Iterator iterator_type;

	template <typename IterContextT>
	struct inner
	{
		template <typename PositionT>
		static void init_iterators(IterContextT &iter_ctx, 
			PositionT const &act_pos, boost::wave::language_support language)
		{
			typedef typename IterContextT::iterator_type iterator_type;

			// read in the file
			std::ifstream instream(iter_ctx.filename.c_str());
			if (!instream.is_open()) {
				BOOST_WAVE_THROW_CTX(iter_ctx.ctx, preprocess_exception, 
					bad_include_file, iter_ctx.filename.c_str(), act_pos);
				return;
			}
			instream.unsetf(std::ios::skipws);

			iter_ctx.first = iterator_type(
				Iterator(base_iterator_type(instream.rdbuf())), Iterator(base_iterator_type()), 
				PositionT(iter_ctx.filename), language);
			iter_ctx.last = iterator_type();
		}
	};
};

#if 1
typedef LoadFile input_policy_type;
#else
typedef StreamFile input_policy_type; // doesn't work because lexer assumes iterator to be a char*
#endif


//  This is the resulting context type to use. The first template parameter
//  should match the iterator type to be used during construction of the
//  corresponding context object (see below).
typedef boost::wave::context<input_policy_type::iterator_type, lex_iterator_type, input_policy_type, Hooks>
context_type;

//  The preprocessor iterator shouldn't be constructed directly. It is 
//  to be generated through a wave::context<> object. This wave:context<> 
//  object is to be used additionally to initialize and define different 
//  parameters of the actual preprocessing (not done here).
//
//  The preprocessing of the input stream is done on the fly behind the 
//  scenes during iteration over the context_type::iterator_type stream.

struct LexContext : public FileBuffer, public context_type, public LexNames
{
	LexContext(std::ifstream& instream, const char* input)
		: FileBuffer(instream), context_type(FileBuffer::buffer, FileBuffer::buffer + FileBuffer::length, input, *this)
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


LexContext& createContext(std::ifstream& instring, const char* input)
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

const char* Lexer::makeIdentifier(const char* value)
{
	return context.makeIdentifier(value);
}



#ifdef _DEBUG
void Lexer::debugEvents(IncludeEvents events, Name source)
{
	for(unsigned short i = 0; i != events.pop; ++i)
	{
		--depth;
		includes[depth] = NAME_NULL;
	}
	for(unsigned short i = 0; i != events.push; ++i)
	{
		includes[depth++] = NAME_NULL;
	}
	if(includes[depth - 1] == NAME_NULL)
	{
		includes[depth - 1] = source;
		std::cout << "parser: " << findFilenameSafe(includes[depth - 1].c_str()) << std::endl;
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
#if 1//def _DEBUG
				{
					ProfileScope profile(gProfileDiagnose);
					printer.printToken(token, token.get_value().c_str());
				}
#endif
				FilePosition position = context.get_hooks().macroTokenCount == 0
					? context.makeFilePosition(token.get_position())
					: context.get_hooks().macroPosition;
				LexTokenId id = token;
				const char* name = context.makeIdentifier(token.get_value().c_str());
				if(id == boost::wave::T_IDENTIFIER
					&& name[0] == '_'
					&& name[1] == '_')
				{
					// compiler intrinsic?
					id = getLexerIntrinsicId(name);
				}

				*first++ = Token(id, TokenValue(name), position, Source(context.get_hooks().getSourcePath(), position.line, position.column), context.get_hooks().events);

				//debugEvents(context.get_hooks().events, context.get_hooks().getSourcePath().absolute);

				context.get_hooks().events = IncludeEvents();
				if(context.get_hooks().macroTokenCount != 0)
				{
					--context.get_hooks().macroTokenCount;
				}
			}
		}
		// if reached end of token stream, append EOF
		if(this->first == this->last
			&& first != last)
		{
			*first++ = Token(boost::wave::T_EOF, TokenValue(), FilePosition(), Source(context.get_hooks().getSourcePath(), 0, 0), context.get_hooks().events);

			//debugEvents(context.get_hooks().events, context.get_hooks().getSourcePath().absolute);
		}
	}
	catch (boost::wave::cpp_exception const& e)
	{
		// some preprocessing error
		std::cerr 
			<< e.file_name() << "(" << e.line_no() << "): "
			<< e.description() << std::endl;
		throw LexError();
	}
	catch (boost::wave::cpplexer::lexing_exception const& e)
	{
		std::cerr 
			<< e.file_name() << "(" << e.line_no() << "): "
			<< e.description() << std::endl;
		throw LexError();
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
	catch (boost::wave::cpplexer::lexing_exception const& e)
	{
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
	catch (boost::wave::cpplexer::lexing_exception const& e)
	{
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

#if 1 // required only if iterator is not a std::string::iterator
#include <boost/wave/cpplexer/re2clex/cpp_re2c_lexer.hpp>
template struct boost::wave::cpplexer::new_lexer_gen<input_policy_type::iterator_type, LexFilePosition>;
#endif
