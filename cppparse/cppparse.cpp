
#include "parser.h"
#include "semantic.h"
#include "report.h"
#include "util.h"

#include <iostream>
#include <fstream>
#include <string>


typedef int (*VerifyFunc)(void* output, const PrintSymbolArgs& args);

typedef void* (*ParseFunc)(ParserContext& context);

typedef const char* CharConstPointer;
typedef ArrayRange<CharConstPointer> CharConstPointerRange;

struct Test
{
	const char* input;
	CharConstPointerRange definitions;
	CharConstPointerRange includes;
	VerifyFunc verify;
	ParseFunc parse;
};

template<typename Result>
Test makeTest(const char* input, const CharConstPointerRange& definitions, const CharConstPointerRange& includes, int (*verify)(Result*, const PrintSymbolArgs&), Result* (*parse)(ParserContext&))
{
	Test result = { input, definitions, includes, VerifyFunc(verify), ParseFunc(parse) };
	return result;
}

int runTest(const Test& test)
{
	try {

		std::string instring;

		const char* predefined = "$predefined_msvc.h";
		std::ifstream instream(predefined);

		if (!instream.is_open()) {
			std::cerr << "Could not open input file: " << predefined << std::endl;
			return -2;
		}
		instream.unsetf(std::ios::skipws);

		std::cout << "parsing input file: " << test.input << std::endl;

		LexContext& lexer = createContext(instream, "$outer.cpp");
		add_macro_definition(lexer, Concatenate(makeRange("_CPPP_SRC=\""), makeRange(test.input), makeRange("\"")).c_str(), true);
		add_macro_definition(lexer, "__fastcall=", true);
		add_macro_definition(lexer, "__thiscall=", true);
		add_macro_definition(lexer, "__clrcall=", true);
		add_macro_definition(lexer, "__stdcall=", true);
		add_macro_definition(lexer, "__cdecl=", true);
		add_macro_definition(lexer, "__pragma(arg)=", true);
		add_macro_definition(lexer, "__inline=", true);
		add_macro_definition(lexer, "__forceinline=", true);
		add_macro_definition(lexer, "__w64=", true);
		add_macro_definition(lexer, "__ptr64=", true);
		add_macro_definition(lexer, "__ptr32=", true);
		add_macro_definition(lexer, "_WCHAR_T_DEFINED=1", true);
		add_macro_definition(lexer, "_NATIVE_WCHAR_T_DEFINED=1", true);
		add_macro_definition(lexer, "__wchar_t=wchar_t", true);
		add_macro_definition(lexer, "__declspec(modifiers)=", true);
		add_macro_definition(lexer, "__uuidof(type)=GUID()", true);
		add_macro_definition(lexer, "__alignof(type)=4", true);

		// Visual Studio 2010
		add_macro_definition(lexer, "__single_inheritance", true);
		add_macro_definition(lexer, "__multiple_inheritance", true);
		add_macro_definition(lexer, "__virtual_inheritance", true);

		add_macro_definition(lexer, "__has_nothrow_constructor(type)=0", true);
		add_macro_definition(lexer, "__has_nothrow_copy(type)=0", true);
		add_macro_definition(lexer, "__has_trivial_assign(type)=0", true);
		add_macro_definition(lexer, "__has_trivial_constructor(type)=0", true);
		add_macro_definition(lexer, "__has_trivial_copy(type)=0", true);
		add_macro_definition(lexer, "__has_trivial_destructor(type)=0", true);
		add_macro_definition(lexer, "__has_virtual_destructor(type)=0", true);
		add_macro_definition(lexer, "__is_abstract(type)=0", true);
		add_macro_definition(lexer, "__is_base_of(base, derived)=0", true);
		add_macro_definition(lexer, "__is_class(type)=0", true);
		add_macro_definition(lexer, "__is_convertible_to(type)=0", true);
		add_macro_definition(lexer, "__is_empty(type)=0", true);
		add_macro_definition(lexer, "__is_enum(type)=0", true);
		add_macro_definition(lexer, "__is_pod(type)=0", true);
		add_macro_definition(lexer, "__is_polymorphic(type)=0", true);
		add_macro_definition(lexer, "__is_union(type)=0", true);

		// optional: _DEBUG, _DLL, /Ze=_MSC_EXTENSIONS, /MT=_MT
		add_macro_definition(lexer, "_WIN32", true);
		add_macro_definition(lexer, "__FUNCTION__=\"<function-sig>\"", true);
		add_macro_definition(lexer, "_INTEGRAL_MAX_BITS=64", true); // long long
		add_macro_definition(lexer, "_M_IX86=600", true); // /GB: Blend
		add_macro_definition(lexer, "_MSC_VER=1400", true); // Visual C++ 8
		add_macro_definition(lexer, "_MSC_FULL_VER=140050727", true); // Visual C++ 8

		add_macro_definition(lexer, "_CPPP_TEST", true);


		for(const CharConstPointer* p = test.definitions.first; p != test.definitions.last; ++p)
		{
			add_macro_definition(lexer, *p, true);
		}
		for(const CharConstPointer* p = test.includes.first; p != test.includes.last; ++p)
		{
			add_include_path(lexer, *p);
			add_sysinclude_path(lexer, *p);
		}
#if 1 // full parse
		StringRange root(test.input, strrchr(test.input, '.'));
		ParserContext context(lexer, Concatenate(root, makeRange(".prepro.cpp")).c_str());
		int result = test.verify(test.parse(context), PrintSymbolArgs(
			"",
			Concatenate(makeRange("out/"), makeRange(findFilename(test.input)), makeRange("/")).c_str(), 
			context.getIncludeGraph())
		);
		if(context.stats.count != 0)
		{
			printPosition(context.stats.position);
			std::cout << "backtrack: " << context.stats.symbol << ": " << context.stats.count << std::endl;
		}
#else // just print all the preprocessed tokens
		LexIterator& first = createBegin(lexer);
		LexIterator& last = createEnd(lexer);

		//  The input stream is preprocessed for you while iterating over the range
		//  [first, last)
		while (first != last) {
			const LexToken& token = dereference(first);
#ifdef _DEBUG
			std::cout << get_value(token);
#endif
			increment(first);
		}
#endif
		release(lexer);
	}
	catch(LexError&)
	{
		std::cerr 
			<< "exception caught"
			<< std::endl;
		return 1;
	}
	catch(std::exception const& e)
	{
		std::cerr 
			<< "exception caught: "
			<< e.what()
			<< std::endl;
		return 1;
	}
#if 0
	catch(...)
	{
		std::cerr 
			<< "exception caught"
			<< std::endl;
		return 1;
	}
#endif

	return 0;
}

int printFile(cpp::declaration_seq* result, const PrintSymbolArgs& args)
{
	printSymbol(result, args);
	return 0;
}


typedef std::list<std::string> StringList;

struct CompilerOptions
{
	StringList definitions;
	StringList includes;
	StringList sources;
};

enum CompilerOptionType
{
	COMPILEROPTION_DEFINITION,
	COMPILEROPTION_INCLUDE,
	COMPILEROPTION_SOURCE,
	COMPILEROPTIONCOUNT
};

inline StringList& GetValues(CompilerOptions& options, CompilerOptionType type)
{
	switch(type)
	{
	case COMPILEROPTION_DEFINITION: return options.definitions;
	case COMPILEROPTION_INCLUDE: return options.includes;
	case COMPILEROPTION_SOURCE: return options.sources;
	default:break;
	}
	StringList* p = 0;
	return *p;
}


int main(int argc, char *argv[])
{
	if (2 != argc) {
		std::cerr << "Usage: cppparse <command-file>" << std::endl;
		return -1;
	}

	const char* input = argv[1];

	std::string compiler;
	CompilerOptions options;
	{
		std::ifstream in(input);
		if(!in.is_open())
		{
			std::cerr << "failed to open command-file: " << input << std::endl;
			return -1;
		}
		else
		{

			char line[1024];
			while(in.getline(line, ARRAY_COUNT(line)), in.good())
			{
				if(compiler.empty())
				{
					compiler = line;
				}
				else
				{
					CompilerOptionType type = COMPILEROPTION_SOURCE; // line contains name of source file if it is not an option
					const char* value = line;
					if(*line == '/')
					{
						// this line is an option
						const char* option = line + 1;
						if(string_equal_prefix(option, "I"))
						{
							type = COMPILEROPTION_INCLUDE;
							value = option + 2;
						}
						else if(string_equal_prefix(option, "D"))
						{
							type = COMPILEROPTION_DEFINITION;
							value = option + 2;
						}
						else if(string_equal_prefix(option, "Tp")
							|| string_equal_prefix(option, "Tc"))
						{
							type = COMPILEROPTION_SOURCE;
							value = option + 3;
						}
						else
						{
							type = COMPILEROPTIONCOUNT;
						}
					}

					if(type != COMPILEROPTIONCOUNT) 
					{
						GetValues(options, type).push_back(value);
					}
				}
			}
			if(in.fail()
				&& !in.bad()
				&& !in.eof()) // line contains eof
			{
				// buffer too small
				std::cerr << "line too long in command file: " << input << std::endl;
				return -1;
			}
		}
	}

	CharConstPointer definitions[1024];
	CharConstPointer* definition = definitions;
	for(StringList::const_iterator i = options.definitions.begin(); i != options.definitions.end(); ++i)
	{
		*definition++ = (*i).c_str();
	}
	
	CharConstPointer includes[1024];
	CharConstPointer* include = includes;
	for(StringList::const_iterator i = options.includes.begin(); i != options.includes.end(); ++i)
	{
		*include++ = (*i).c_str();
	}

	for(StringList::const_iterator i = options.sources.begin(); i != options.sources.end(); ++i)
	{
		int result = runTest(makeTest((*i).c_str(), makeRange(definitions, definition), makeRange(includes, include), printFile, parseFile));
		if(result != 0)
		{
			return result;
		}
	}
 	return 0;
}
