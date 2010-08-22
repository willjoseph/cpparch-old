
#include "parser.h"
#include "semantic.h"
#include "util.h"

#include <iostream>
#include <fstream>
#include <string>


typedef int (*VerifyFunc)(void* output, const PrintSymbolArgs& args);

typedef void* (*ParseFunc)(ParserContext& lexer);

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

#if 1 // read predefined stuff into a buffer, append #include "<source>"
		const char* predefined = "$predefined_msvc.h";
		std::ifstream instream(predefined);

		if (!instream.is_open()) {
			std::cerr << "Could not open input file: " << predefined << std::endl;
			return -2;
		}
		std::cout << "reading input file: " << predefined << std::endl;
		instream.unsetf(std::ios::skipws);
		instring = std::string(std::istreambuf_iterator<char>(instream.rdbuf()),
			std::istreambuf_iterator<char>());

		instring += Concatenate(makeRange("\n #include \""), makeRange(test.input), makeRange("\"\n")).c_str();
#else // #include predefined stuff before #include "<source>"
		instring = Concatenate(makeRange("#include \"$predefined_msvc.h\"\n" " #include \""), makeRange(test.input), makeRange("\"\n")).c_str();
#endif

		LexContext& context = createContext(instring, "$outer.cpp");
		add_macro_definition(context, "__fastcall=", true);
		add_macro_definition(context, "__thiscall=", true);
		add_macro_definition(context, "__clrcall=", true);
		add_macro_definition(context, "__stdcall=", true);
		add_macro_definition(context, "__cdecl=", true);
		add_macro_definition(context, "__pragma(arg)=", true);
		add_macro_definition(context, "__inline=", true);
		add_macro_definition(context, "__forceinline=", true);
		add_macro_definition(context, "__w64=", true);
		add_macro_definition(context, "__ptr64=", true);
		add_macro_definition(context, "__ptr32=", true);
		add_macro_definition(context, "_WCHAR_T_DEFINED=1", true);
		add_macro_definition(context, "_NATIVE_WCHAR_T_DEFINED=1", true);
		add_macro_definition(context, "__wchar_t=wchar_t", true);
		add_macro_definition(context, "__declspec(modifiers)=", true);
		add_macro_definition(context, "__uuidof(type)=GUID()", true);
		add_macro_definition(context, "__alignof(type)=4", true);

		add_macro_definition(context, "__has_nothrow_constructor(type)=0", true);
		add_macro_definition(context, "__has_nothrow_copy(type)=0", true);
		add_macro_definition(context, "__has_trivial_assign(type)=0", true);
		add_macro_definition(context, "__has_trivial_constructor(type)=0", true);
		add_macro_definition(context, "__has_trivial_copy(type)=0", true);
		add_macro_definition(context, "__has_trivial_destructor(type)=0", true);
		add_macro_definition(context, "__has_virtual_destructor(type)=0", true);
		add_macro_definition(context, "__is_abstract(type)=0", true);
		add_macro_definition(context, "__is_base_of(base, derived)=0", true);
		add_macro_definition(context, "__is_class(type)=0", true);
		add_macro_definition(context, "__is_convertible_to(type)=0", true);
		add_macro_definition(context, "__is_empty(type)=0", true);
		add_macro_definition(context, "__is_enum(type)=0", true);
		add_macro_definition(context, "__is_pod(type)=0", true);
		add_macro_definition(context, "__is_polymorphic(type)=0", true);
		add_macro_definition(context, "__is_union(type)=0", true);

		// optional: _DEBUG, _DLL, /Ze=_MSC_EXTENSIONS, /MT=_MT
		add_macro_definition(context, "_WIN32", true);
		add_macro_definition(context, "__FUNCTION__=\"<function-sig>\"", true);
		add_macro_definition(context, "_INTEGRAL_MAX_BITS=64", true); // long long
		add_macro_definition(context, "_M_IX86=600", true); // /GB: Blend
		add_macro_definition(context, "_MSC_VER=1400", true); // Visual C++ 8
		add_macro_definition(context, "_MSC_FULL_VER=140050727", true); // Visual C++ 8

		add_macro_definition(context, "_CPPP_TEST", true);


		for(const CharConstPointer* p = test.definitions.first; p != test.definitions.last; ++p)
		{
			add_macro_definition(context, *p, true);
		}
		for(const CharConstPointer* p = test.includes.first; p != test.includes.last; ++p)
		{
			add_include_path(context, *p);
			add_sysinclude_path(context, *p);
		}
#if 1 // full parse
		StringRange root(test.input, strrchr(test.input, '.'));
		ParserContext lexer(context, Concatenate(root, makeRange(".prepro.cpp")).c_str());
		PrintSymbolArgs args = { "out\\", lexer.getIncludeGraph() };
		int result = test.verify(test.parse(lexer), args);
		if(lexer.stats.count != 0)
		{
			printPosition(lexer.stats.position);
			std::cout << "backtrack: " << lexer.stats.symbol << ": " << lexer.stats.count << std::endl;
		}
#else // just print all the preprocessed tokens
		LexIterator& first = createBegin(context);
		LexIterator& last = createEnd(context);

		//  The input stream is preprocessed for you while iterating over the range
		//  [first, last)
		while (first != last) {
			const LexToken& token = dereference(first);
			std::cout << get_value(token);
			increment(first);
		}
#endif
		release(context);
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
	catch(...)
	{
		std::cerr 
			<< "exception caught"
			<< std::endl;
		return 1;
	}

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
