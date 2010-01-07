
#include "lexer.h"

#include <iostream>
#include <fstream>
#include <string>

int main(int argc, char *argv[])
{
	if (2 != argc) {
		std::cerr << "Usage: cppparse infile" << std::endl;
		return -1;
	}

	const char* input = argv[1];

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
