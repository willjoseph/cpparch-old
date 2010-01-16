
#include "parser.h"

#include <iostream>
#include <fstream>
#include <string>

typedef int (*VerifyFunc)(void* output);

typedef void* (*ParseFunc)(LexContext& context);

struct Test
{
	const char* input;
	VerifyFunc verify;
	ParseFunc parse;
};

template<typename Result>
Test makeTest(const char* input, int (*verify)(Result*), Result* (*parse)(LexContext&))
{
	Test result = { input, VerifyFunc(verify), ParseFunc(parse) };
	return result;
}

int runTest(const Test& test)
{
	try {
		//[quick_start_main
		//  The following preprocesses the given input file.
		//  Open and read in the specified input file.
		std::ifstream instream(test.input);
		std::string instring;

		if (!instream.is_open()) {
			std::cerr << "Could not open input file: " << test.input << std::endl;
			return -2;
		}
		instream.unsetf(std::ios::skipws);
		instring = std::string(std::istreambuf_iterator<char>(instream.rdbuf()),
			std::istreambuf_iterator<char>());

		LexContext& context = createContext(instring, test.input);
		add_sysinclude_path(context, "C:\\Program Files\\Microsoft Visual Studio 8\\VC\\include");
		add_macro_definition(context, "__thiscall=", true);
		add_macro_definition(context, "__clrcall=", true);
		add_macro_definition(context, "__cdecl=", true);
		add_macro_definition(context, "__pragma(arg)=", true);
		// optional: _DEBUG, _DLL, /Ze=_MSC_EXTENSIONS, /MT=_MT, /Zc:wchar_t=_NATIVE_WCHAR_T_DEFINED/_WCHAR_T_DEFINED
		add_macro_definition(context, "_WIN32", true);
		add_macro_definition(context, "__FUNCTION__=\"<function-sig>\"", true);
		add_macro_definition(context, "_INTEGRAL_MAX_BITS=32", true);
		add_macro_definition(context, "_M_IX86=600", true); // /GB: Blend
		add_macro_definition(context, "_MSC_VER=1400", true); // Visual C++ 2005
#if 1
		return test.verify(test.parse(context));
#else
		LexIterator& first = createBegin(context);
		LexIterator& last = createEnd(context);

		//  The input stream is preprocessed for you while iterating over the range
		//  [first, last)
		while (first != last) {
			const LexToken& token = dereference(first);
			std::cout << get_value(token);
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
#define VERIFY_CAST(Type, p) verifyNotNull(dynamic_cast<Type*>(verifyNotNull(p)))

template<typename T>
inline void verifyIdentifier(T* p, const char* value)
{
	PARSE_ASSERT(VERIFY_CAST(cpp::identifier, p)->value == value);
}

int verifyFunctionDefinition(cpp::declaration_seq* result)
{
	cpp::function_definition* func = VERIFY_CAST(cpp::function_definition, verifyNotNull(result)->item);
	PARSE_ASSERT(VERIFY_CAST(cpp::simple_type_specifier_builtin, verifyNotNull(func->spec)->type)->value == cpp::simple_type_specifier_builtin::VOID);
	verifyIdentifier(VERIFY_CAST(cpp::direct_declarator, func->decl)->prefix, "function");
	cpp::compound_statement* body = VERIFY_CAST(cpp::compound_statement, func->body);
	return 0;
}

int verifyNamespace(cpp::declaration_seq* result)
{
	cpp::namespace_definition* def = VERIFY_CAST(cpp::namespace_definition, verifyNotNull(result)->item);
	PARSE_ASSERT(def->id->value != "");
	PARSE_ASSERT(def->body == 0);
	return 0;
}

int verifyPtr(cpp::declaration_seq* result)
{
	cpp::simple_declaration* decl = VERIFY_CAST(cpp::simple_declaration, verifyNotNull(result)->item);
	PARSE_ASSERT(decl->spec != 0);
	PARSE_ASSERT(decl->spec->type != 0);
	cpp::simple_type_specifier_builtin* spec = VERIFY_CAST(cpp::simple_type_specifier_builtin, decl->spec->type);
	PARSE_ASSERT(spec->value == cpp::simple_type_specifier_builtin::VOID);
	PARSE_ASSERT(decl->decl != 0);
	PARSE_ASSERT(decl->decl->item != 0);
	PARSE_ASSERT(decl->decl->item->decl != 0);
	cpp::declarator_ptr* declr = VERIFY_CAST(cpp::declarator_ptr, decl->decl->item->decl);
	PARSE_ASSERT(declr->op != 0);
	PARSE_ASSERT(declr->decl != 0);
	cpp::direct_declarator* dir = VERIFY_CAST(cpp::direct_declarator, declr->decl);
	PARSE_ASSERT(dir->prefix != 0);
	cpp::identifier* id = VERIFY_CAST(cpp::identifier, dir->prefix);
	PARSE_ASSERT(dir->suffix == 0);
	PARSE_ASSERT(decl->decl->item->init == 0);
	return 0;
}

int verifyNull(cpp::declaration_seq* result)
{
	return 0;
}

int verifyAmbFuncCast(cpp::statement_seq* result)
{
	// TODO: ambiguity: int(x); // function-style-cast or simple-declaration?
	//cpp::postfix_expression_construct* result = VERIFY_CAST(cpp::postfix_expression_construct, verifyNotNull(result)->item);
	return 0;
}

int verifyAmbOnesComp(cpp::statement_seq* result)
{
	cpp::expression_statement* stmt = VERIFY_CAST(cpp::expression_statement, verifyNotNull(result)->item);
	cpp::unary_expression_op* expr = VERIFY_CAST(cpp::unary_expression_op, stmt->expr);
	PARSE_ASSERT(expr->op->value == cpp::unary_operator::COMPL);
	cpp::postfix_expression_default* post = VERIFY_CAST(cpp::postfix_expression_default, expr->expr);
	verifyIdentifier(post->expr, "x");
	cpp::postfix_expression_call* call = VERIFY_CAST(cpp::postfix_expression_call, verifyNotNull(post->suffix)->item);
	return 0;
}

int verifyAmbDeclSpec(cpp::declaration_seq* result)
{
	cpp::simple_declaration* decl = VERIFY_CAST(cpp::simple_declaration, verifyNotNull(result)->item);
	cpp::simple_type_specifier_name* spec = VERIFY_CAST(cpp::simple_type_specifier_name, verifyNotNull(decl->spec)->type);
	PARSE_ASSERT(spec->context);
	verifyIdentifier(verifyNotNull(spec->context->prefix)->name, "Class");
	verifyIdentifier(verifyNotNull(spec->context->suffix)->name, "Scope");
	verifyIdentifier(spec->id, "p");
	return 0;
}

int verifyAmbCastExpr(cpp::statement_seq* result)
{
	// TODO: ambiguity: (Type)(x); // c-style-cast or function-declaration?
	//cpp::simple_declaration* decln = VERIFY_CAST(cpp::simple_declaration, verifyNotNull(result)->item);
	return 0;
}

int verifyAmbConstructor(cpp::declaration_seq* result)
{
	cpp::simple_declaration* decln = VERIFY_CAST(cpp::simple_declaration, verifyNotNull(result)->item);
	cpp::class_specifier* spec = VERIFY_CAST(cpp::class_specifier, verifyNotNull(decln->spec)->type);
	cpp::member_specification_list* members = VERIFY_CAST(cpp::member_specification_list, verifyNotNull(spec->members));
	cpp::member_declaration_ctor* member = VERIFY_CAST(cpp::member_declaration_ctor, verifyNotNull(members->item));
	cpp::member_declarator_default* declr = VERIFY_CAST(cpp::member_declarator_default, verifyNotNull(member->decl)->item);
	verifyIdentifier(VERIFY_CAST(cpp::direct_declarator, declr->decl)->prefix, "A");
	return 0;
}

int verifyIf(cpp::statement_seq* result)
{
	cpp::selection_statement_if* stmt = VERIFY_CAST(cpp::selection_statement_if, verifyNotNull(result)->item);
	{
		cpp::condition_init* cond = VERIFY_CAST(cpp::condition_init, stmt->cond);
		verifyIdentifier(VERIFY_CAST(cpp::simple_type_specifier_name, cond->type->type)->id, "Type");
		verifyIdentifier(VERIFY_CAST(cpp::direct_declarator, cond->decl)->prefix, "x");
		verifyIdentifier(VERIFY_CAST(cpp::postfix_expression_default, cond->init)->expr, "y");
	}
	cpp::compound_statement* body = VERIFY_CAST(cpp::compound_statement, stmt->body);
	return 0;
}

int verifyFor(cpp::statement_seq* result)
{
	cpp::iteration_statement_for* stmt = VERIFY_CAST(cpp::iteration_statement_for, verifyNotNull(result)->item);
	{
		cpp::simple_declaration* decl = VERIFY_CAST(cpp::simple_declaration, stmt->init);
		verifyIdentifier(VERIFY_CAST(cpp::simple_type_specifier_name, decl->spec->type)->id, "Type");
		verifyIdentifier(VERIFY_CAST(cpp::direct_declarator, decl->decl->item->decl)->prefix, "x");
		cpp::initializer_default* init = VERIFY_CAST(cpp::initializer_default, decl->decl->item->init);
		verifyIdentifier(VERIFY_CAST(cpp::postfix_expression_default, init->clause)->expr, "y");
	}
	cpp::compound_statement* body = VERIFY_CAST(cpp::compound_statement, stmt->body);
	return 0;
}

int main(int argc, char *argv[])
{
	if(argc == 1)
	{
		const Test tests[] = {
			makeTest("test/test_error.cpp", verifyNull, parseFile),
			makeTest("test/test_iostream.cpp", verifyNull, parseFile),
			makeTest("test/test_amb_constructor.cpp", verifyAmbConstructor, parseFile),
			makeTest("test/test_amb_func_cast.cpp", verifyAmbFuncCast, parseFunction),
			makeTest("test/test_amb_ones_comp.cpp", verifyAmbOnesComp, parseFunction),
			makeTest("test/test_for.cpp", verifyFor, parseFunction),
			makeTest("test/test_if.cpp", verifyIf, parseFunction),
			makeTest("test/test_function_definition.cpp", verifyFunctionDefinition, parseFile),
			makeTest("test/test_amb_cast_expr.cpp", verifyAmbCastExpr, parseFunction),
			makeTest("test/test_ptr.cpp", verifyPtr, parseFile),
			makeTest("test/test_amb_decl_spec.cpp", verifyAmbDeclSpec, parseFile),
			makeTest("test/test_namespace.cpp", verifyNamespace, parseFile),
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

	return runTest(makeTest(input, verifyNull, parseFile));
}
