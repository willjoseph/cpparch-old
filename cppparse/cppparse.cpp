
#include "parser.h"

#include <iostream>
#include <fstream>
#include <string>

typedef int (*VerifyFunc)(cpp::declaration_seq* declarations);

int runTest(const char* input, VerifyFunc verify)
{
	try {
		//[quick_start_main
		//  The following preprocesses the given input file.
		//  Open and read in the specified input file.
		std::ifstream instream(input);
		std::string instring;

		if (!instream.is_open()) {
			std::cerr << "Could not open input file: " << input << std::endl;
			return -2;
		}
		instream.unsetf(std::ios::skipws);
		instring = std::string(std::istreambuf_iterator<char>(instream.rdbuf()),
			std::istreambuf_iterator<char>());

#if 1
		return verify(parseFile(instring, input));
#else
		LexContext& context = createContext(instring, input);
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
	}
	catch(LexError&)
	{
		std::cerr 
			<< "exception caught"
			<< std::endl;
		return 1;
	}
	catch(std::exception const&)
	{
		std::cerr 
			<< "exception caught"
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

struct Test
{
	const char* input;
	VerifyFunc verify;
};

template<typename T>
inline T* verifyNotNull(T* p)
{
	PARSE_ASSERT(p != 0);
	return p;
}
#define VERIFY_CAST(Type, p) verifyNotNull(dynamic_cast<Type*>(p))

int verifyFunctionDefinition(cpp::declaration_seq* declarations)
{
	return 0;
}

int verifyNamespace(cpp::declaration_seq* declarations)
{
	cpp::namespace_definition* result = VERIFY_CAST(cpp::namespace_definition, declarations->item);
	PARSE_ASSERT(result->id.value == "Name");
	PARSE_ASSERT(result->body == 0);
	return 0;
}

int verifyNull(cpp::declaration_seq* declarations)
{
	return 0;
}

int main(int argc, char *argv[])
{
	if(argc == 1)
	{
		const Test tests[] = {
			{ "test/test_namespace.cpp", verifyNamespace },
			{ "test/test_function_definition.cpp", verifyFunctionDefinition },
		};
		for(const Test* p = tests; p != tests + (sizeof(tests) / sizeof(*tests)); ++p)
		{
			int result = runTest(p->input, p->verify);
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

	return runTest(input, verifyNull);
}
