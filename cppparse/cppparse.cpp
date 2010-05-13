
#include "parser.h"
#include "semantic.h"

#include <iostream>
#include <fstream>
#include <string>

template<typename T>
struct ArrayRange
{
	const T* first;
	const T* last;
	ArrayRange(const T* first, const T* last)
		: first(first), last(last)
	{
	}
};

template<typename T>
ArrayRange<T> makeRange(const T* first, const T* last)
{
	return ArrayRange<T>(first, last);
}

#define ARRAY_RANGE(array) makeRange(array, ARRAY_END(array))



typedef int (*VerifyFunc)(void* output, const char* path);

typedef void* (*ParseFunc)(ParserContext& lexer);

typedef const char* CharConstPointer;
typedef ArrayRange<CharConstPointer> CharConstPointerRange;
const CharConstPointer DEFINITIONS_DEBUG[] = {
	"WIN32",
	"_DEBUG"
};

const CharConstPointerRange CHARCONSTPOINTERRANGE_EMPTY = CharConstPointerRange(0, 0);

struct Test
{
	const char* input;
	CharConstPointerRange definitions;
	CharConstPointerRange includes;
	VerifyFunc verify;
	ParseFunc parse;
};

template<typename Result>
Test makeTest(const char* input, const CharConstPointerRange& definitions, const CharConstPointerRange& includes, int (*verify)(Result*, const char*), Result* (*parse)(ParserContext&))
{
	Test result = { input, definitions, includes, VerifyFunc(verify), ParseFunc(parse) };
	return result;
}

struct StringRange
{
	const char* first;
	const char* last;
	StringRange(const char* first, const char* last)
		: first(first), last(last)
	{
	}
};

StringRange makeRange(const char* s)
{
	return StringRange(s, s + strlen(s));
}

size_t getLength(const StringRange& range)
{
	return range.last - range.first;
}

struct Concatenate
{
	typedef std::vector<char> Buffer;
	Buffer buffer;
	Concatenate(const StringRange& left, const StringRange& right)
	{
		buffer.reserve(getLength(left) + getLength(right) + 1);
		append(left);
		append(right);
		buffer.push_back('\0');
	}
	Concatenate(const StringRange& left, const StringRange& mid, const StringRange& right)
	{
		buffer.reserve(getLength(left) + getLength(mid) + getLength(right) + 1);
		append(left);
		append(mid);
		append(right);
		buffer.push_back('\0');
	}
	void append(const StringRange& range)
	{
		buffer.insert(buffer.end(), range.first, range.last);
	}
	const char* c_str() const
	{
		return &(*buffer.begin());
	}
};

int runTest(const Test& test)
{
	try {
		//[quick_start_main
		//  The following preprocesses the given input file.
		//  Open and read in the specified input file.
		std::string instring;
#if 1
		instring = Concatenate(makeRange(/*"#include \"test/predefined_msvc.h\"\n"*/ " #include \""), makeRange(test.input), makeRange("\"\n")).c_str();
#else
		std::ifstream instream(test.input);

		if (!instream.is_open()) {
			std::cerr << "Could not open input file: " << test.input << std::endl;
			return -2;
		}
		std::cout << "reading input file: " << test.input << std::endl;
		instream.unsetf(std::ios::skipws);
		instring = std::string(std::istreambuf_iterator<char>(instream.rdbuf()),
			std::istreambuf_iterator<char>());
#endif

		LexContext& context = createContext(instring, "$outer.cpp");
		add_sysinclude_path(context, "C:\\Program Files (x86)\\Microsoft Visual Studio 8\\VC\\include");
		add_sysinclude_path(context, "C:\\Program Files (x86)\\Microsoft Visual Studio 8\\VC\\PlatformSDK\\include");
		add_sysinclude_path(context, "C:\\Program Files\\Microsoft Visual Studio 8\\VC\\include");
		add_sysinclude_path(context, "C:\\Program Files\\Microsoft Visual Studio 8\\VC\\PlatformSDK\\include");
		add_sysinclude_path(context, "C:\\Program Files\\Microsoft Platform SDK for Windows Server 2003 R2\\include");
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
#if 1
		StringRange root(test.input, strrchr(test.input, '.'));
		ParserContext lexer(context, Concatenate(root, makeRange(".prepro.cpp")).c_str());
		int result = test.verify(test.parse(lexer), Concatenate(root, makeRange(".html")).c_str());
		if(lexer.stats.count != 0)
		{
			printPosition(lexer.stats.position);
			std::cout << "backtrack: " << lexer.stats.symbol << ": " << lexer.stats.count << std::endl;
		}
#else
		LexIterator& first = createBegin(context);
		LexIterator& last = createEnd(context);

		//  The input stream is preprocessed for you while iterating over the range
		//  [first, last)
		while (first != last) {
			const LexToken& token = dereference(first);
			//std::cout << get_value(token);
			increment(first);
		}
		//]
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

template<typename T>
inline T* verifyNotNull(T* p)
{
	PARSE_ASSERT(p != 0);
	return p;
}
template<typename T>
inline T* verifyNotNull(cpp::symbol<T> symbol)
{
	PARSE_ASSERT(symbol.p != 0);
	return symbol.p;
}
#define VERIFY_CAST(Type, p) verifyNotNull(dynamic_cast<Type*>(verifyNotNull(p)))

template<typename T>
inline void verifyIdentifier(cpp::symbol<T> p, const char* value)
{
	PARSE_ASSERT(strcmp(VERIFY_CAST(cpp::identifier, p)->value.value, value) == 0);
}

int verifyFunctionDefinition(cpp::declaration_seq* result, const char* path)
{
	printSymbol(result, path);
#if 0
	cpp::function_definition* func = VERIFY_CAST(cpp::function_definition, verifyNotNull(result)->item);
	PARSE_ASSERT(VERIFY_CAST(cpp::simple_type_specifier_builtin, verifyNotNull(func->spec)->type)->value == cpp::simple_type_specifier_builtin::VOID);
	verifyIdentifier(VERIFY_CAST(cpp::direct_declarator, func->decl)->prefix, "function");
	cpp::compound_statement* body = VERIFY_CAST(cpp::compound_statement, func->suffix->body);
#endif
	return 0;
}

int verifyNamespace(cpp::declaration_seq* result, const char* path)
{
	printSymbol(result, path);
	cpp::namespace_definition* def = VERIFY_CAST(cpp::namespace_definition, verifyNotNull(result)->item);
	//PARSE_ASSERT(def->id->value != "");
	PARSE_ASSERT(def->body == 0);
	return 0;
}

int verifyPtr(cpp::declaration_seq* result, const char* path)
{
	printSymbol(result, path);
	cpp::simple_declaration* decl = VERIFY_CAST(cpp::simple_declaration, verifyNotNull(result)->item);
	PARSE_ASSERT(decl->spec != 0);
	PARSE_ASSERT(decl->spec->type != 0);
	cpp::simple_type_specifier_builtin* spec = VERIFY_CAST(cpp::simple_type_specifier_builtin, decl->spec->type);
	PARSE_ASSERT(spec->id == cpp::simple_type_specifier_builtin::VOID);
#if 0
	PARSE_ASSERT(decl->decl != 0);
	cpp::declarator_ptr* declr = VERIFY_CAST(cpp::declarator_ptr, decl->decl);
	PARSE_ASSERT(declr->op != 0);
	PARSE_ASSERT(declr->decl != 0);
	cpp::direct_declarator* dir = VERIFY_CAST(cpp::direct_declarator, declr->decl);
	PARSE_ASSERT(dir->prefix != 0);
	cpp::identifier* id = VERIFY_CAST(cpp::identifier, dir->prefix);
	PARSE_ASSERT(dir->suffix == 0);
	PARSE_ASSERT(decl->suffix->init == 0);
#endif
	return 0;
}

int verifyNull(cpp::declaration_seq* result, const char* path)
{
	printSymbol(result, path);
	return 0;
}

int verifyAmbFuncCast(cpp::statement_seq* result, const char* path)
{
	printSymbol(result, path);
	// TODO: ambiguity: int(x); // function-style-cast or simple-declaration?
	//cpp::postfix_expression_construct* result = VERIFY_CAST(cpp::postfix_expression_construct, verifyNotNull(result)->item);
	return 0;
}

int verifyAmbOnesComp(cpp::statement_seq* result, const char* path)
{
	printSymbol(result, path);
	cpp::expression_statement* stmt = VERIFY_CAST(cpp::expression_statement, verifyNotNull(result)->item);
	cpp::unary_expression_op* expr = VERIFY_CAST(cpp::unary_expression_op, stmt->expr);
	PARSE_ASSERT(expr->op->id == cpp::unary_operator::COMPL);
	cpp::postfix_expression_default* post = VERIFY_CAST(cpp::postfix_expression_default, expr->expr);
	verifyIdentifier(post->left, "x");
	cpp::postfix_expression_call* call = VERIFY_CAST(cpp::postfix_expression_call, verifyNotNull(post->right)->item);
	return 0;
}

int verifyAmbDeclSpec(cpp::declaration_seq* result, const char* path)
{
	printSymbol(result, path);
	cpp::simple_declaration* decl = VERIFY_CAST(cpp::simple_declaration, verifyNotNull(result)->item);
	cpp::simple_type_specifier_name* spec = VERIFY_CAST(cpp::simple_type_specifier_name, verifyNotNull(decl->spec)->type);
	PARSE_ASSERT(spec->context);
#if 0
	verifyIdentifier(verifyNotNull(spec->context->prefix)->id, "Class");
	verifyIdentifier(verifyNotNull(spec->context->suffix)->id, "Scope");
#endif
	verifyIdentifier(spec->id, "p");
	return 0;
}

int verifyAmbCastExpr(cpp::statement_seq* result, const char* path)
{
	printSymbol(result, path);
	// TODO: ambiguity: (Type)(x); // c-style-cast or function-declaration?
	//cpp::simple_declaration* decln = VERIFY_CAST(cpp::simple_declaration, verifyNotNull(result)->item);
	return 0;
}

int verifyAmbConstructor(cpp::declaration_seq* result, const char* path)
{
	printSymbol(result, path);
	cpp::simple_declaration* decln = VERIFY_CAST(cpp::simple_declaration, verifyNotNull(result)->item);
	cpp::class_specifier* spec = VERIFY_CAST(cpp::class_specifier, verifyNotNull(decln->spec)->type);
	cpp::member_specification_list* members = VERIFY_CAST(cpp::member_specification_list, verifyNotNull(spec->members));
#if 0
	cpp::member_declaration_implicit* member = VERIFY_CAST(cpp::member_declaration_implicit, verifyNotNull(members->item));
	verifyIdentifier(VERIFY_CAST(cpp::direct_declarator, member->decl)->prefix, "A");
#endif
	return 0;
}

int verifyIf(cpp::statement_seq* result, const char* path)
{
	printSymbol(result, path);
	cpp::selection_statement_if* stmt = VERIFY_CAST(cpp::selection_statement_if, verifyNotNull(result)->item);
	{
		cpp::condition_init* cond = VERIFY_CAST(cpp::condition_init, stmt->cond);
		verifyIdentifier(VERIFY_CAST(cpp::simple_type_specifier_name, cond->type->type)->id, "Type");
		verifyIdentifier(VERIFY_CAST(cpp::direct_declarator, cond->decl)->prefix, "x");
		verifyIdentifier(VERIFY_CAST(cpp::postfix_expression_default, cond->init)->left, "y");
	}
	cpp::compound_statement* body = VERIFY_CAST(cpp::compound_statement, stmt->body);
	return 0;
}

int verifyFor(cpp::statement_seq* result, const char* path)
{
	printSymbol(result, path);
	cpp::iteration_statement_for* stmt = VERIFY_CAST(cpp::iteration_statement_for, verifyNotNull(result)->item);
	{
		cpp::simple_declaration* decl = VERIFY_CAST(cpp::simple_declaration, stmt->init);
		verifyIdentifier(VERIFY_CAST(cpp::simple_type_specifier_name, decl->spec->type)->id, "Type");
#if 0
		verifyIdentifier(VERIFY_CAST(cpp::direct_declarator, decl->decl)->prefix, "x");
		cpp::initializer_default* init = VERIFY_CAST(cpp::initializer_default, decl->suffix->init);
		verifyIdentifier(VERIFY_CAST(cpp::postfix_expression_default, init->clause)->left, "y");
#endif
	}
	cpp::compound_statement* body = VERIFY_CAST(cpp::compound_statement, stmt->body);
	return 0;
}

int main(int argc, char *argv[])
{
	if(argc == 1)
	{
		const CharConstPointer DEFINITIONS_CPPPARSE[] = {
			"WIN32",
			"_DEBUG",
			"_CONSOLE",
			"_CRT_SECURE_NO_DEPRECATE",
			"CRT_SECURE_NO_WARNINGS",
			"_SCL_SECURE_NO_WARNINGS",
		};
		const CharConstPointer INCLUDES_CPPPARSE[] = {
			"../../boost_1_41_0",
		};
		const Test tests[] = {
			makeTest("test/test_prepro.cpp", ARRAY_RANGE(DEFINITIONS_DEBUG), ARRAY_RANGE(INCLUDES_CPPPARSE), verifyNull, parseFile),
			makeTest("test/test_quick.cpp", ARRAY_RANGE(DEFINITIONS_DEBUG), CHARCONSTPOINTERRANGE_EMPTY, verifyNull, parseFile),
			makeTest("test/test_vector.cpp", ARRAY_RANGE(DEFINITIONS_DEBUG), CHARCONSTPOINTERRANGE_EMPTY, verifyNull, parseFile),
			makeTest("test/test_iostream.cpp", ARRAY_RANGE(DEFINITIONS_DEBUG), CHARCONSTPOINTERRANGE_EMPTY, verifyNull, parseFile),
			makeTest("test/test_windows.cpp", ARRAY_RANGE(DEFINITIONS_DEBUG), CHARCONSTPOINTERRANGE_EMPTY, verifyNull, parseFile),
			makeTest("test/test_map.cpp", ARRAY_RANGE(DEFINITIONS_DEBUG), CHARCONSTPOINTERRANGE_EMPTY, verifyNull, parseFile),
			makeTest("lexer.cpp", ARRAY_RANGE(DEFINITIONS_CPPPARSE), ARRAY_RANGE(INCLUDES_CPPPARSE), verifyNull, parseFile),
			makeTest("cppparse.cpp", ARRAY_RANGE(DEFINITIONS_CPPPARSE), ARRAY_RANGE(INCLUDES_CPPPARSE), verifyNull, parseFile),
			makeTest("parser.cpp", ARRAY_RANGE(DEFINITIONS_CPPPARSE), ARRAY_RANGE(INCLUDES_CPPPARSE), verifyNull, parseFile),
			makeTest("semantic.cpp", ARRAY_RANGE(DEFINITIONS_CPPPARSE), ARRAY_RANGE(INCLUDES_CPPPARSE), verifyNull, parseFile),
			makeTest("test/test_error.cpp", ARRAY_RANGE(DEFINITIONS_DEBUG), CHARCONSTPOINTERRANGE_EMPTY, verifyNull, parseFile),
			makeTest("test/test_amb_constructor.cpp", ARRAY_RANGE(DEFINITIONS_DEBUG), CHARCONSTPOINTERRANGE_EMPTY, verifyAmbConstructor, parseFile),
			makeTest("test/test_amb_func_cast.cpp", ARRAY_RANGE(DEFINITIONS_DEBUG), CHARCONSTPOINTERRANGE_EMPTY, verifyAmbFuncCast, parseFunction),
			makeTest("test/test_amb_ones_comp.cpp", ARRAY_RANGE(DEFINITIONS_DEBUG), CHARCONSTPOINTERRANGE_EMPTY, verifyAmbOnesComp, parseFunction),
			makeTest("test/test_for.cpp", ARRAY_RANGE(DEFINITIONS_DEBUG), CHARCONSTPOINTERRANGE_EMPTY, verifyFor, parseFunction),
			makeTest("test/test_if.cpp", ARRAY_RANGE(DEFINITIONS_DEBUG), CHARCONSTPOINTERRANGE_EMPTY, verifyIf, parseFunction),
			makeTest("test/test_function_definition.cpp", ARRAY_RANGE(DEFINITIONS_DEBUG), CHARCONSTPOINTERRANGE_EMPTY, verifyFunctionDefinition, parseFile),
			makeTest("test/test_amb_cast_expr.cpp", ARRAY_RANGE(DEFINITIONS_DEBUG), CHARCONSTPOINTERRANGE_EMPTY, verifyAmbCastExpr, parseFunction),
			makeTest("test/test_ptr.cpp", ARRAY_RANGE(DEFINITIONS_DEBUG), CHARCONSTPOINTERRANGE_EMPTY, verifyPtr, parseFile),
			makeTest("test/test_amb_decl_spec.cpp", ARRAY_RANGE(DEFINITIONS_DEBUG), CHARCONSTPOINTERRANGE_EMPTY, verifyAmbDeclSpec, parseFile),
			makeTest("test/test_namespace.cpp", ARRAY_RANGE(DEFINITIONS_DEBUG), CHARCONSTPOINTERRANGE_EMPTY, verifyNamespace, parseFile),
		};
		for(const Test* p = tests; p != tests + (sizeof(tests) / sizeof(*tests)); ++p)
		{
			int result = runTest(*p);
			if(result != 0)
			{
				return result;
			}
		}
		return 0;
	}

	if (2 != argc) {
		std::cerr << "Usage: cppparse infile" << std::endl;
		return -1;
	}

	const char* input = argv[1];

	return runTest(makeTest(input, ARRAY_RANGE(DEFINITIONS_DEBUG), CHARCONSTPOINTERRANGE_EMPTY, verifyNull, parseFile));
}
