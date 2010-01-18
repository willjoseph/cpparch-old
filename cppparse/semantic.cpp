
#include "semantic.h"

#include "cpptree.h"

#include <iostream>


struct SymbolPrinter
{
	size_t braceDepth;
	SymbolPrinter()
		: braceDepth(0)
	{
	}

	void nextLine()
	{
		std::cout << std::endl;
		for(size_t i = 0; i != braceDepth; ++i)
		{
			std::cout << '\t';
		}
	}
	void printTerminal(const char* symbol)
	{
		std::cout << symbol << " ";
	}

	void visit(const char* value)
	{
		printTerminal(value);
	}

	template<LexTokenId id>
	void visit(cpp::terminal<id> symbol)
	{
		if(symbol.value != 0)
		{
			if(id == boost::wave::T_LEFTBRACE)
			{
				nextLine();
				++braceDepth;
			}
			else if(id == boost::wave::T_RIGHTBRACE)
			{
				--braceDepth;
				nextLine();
			}

			printTerminal(symbol.value);

			if(id == boost::wave::T_SEMICOLON
				|| id == boost::wave::T_LEFTBRACE
				|| id == boost::wave::T_RIGHTBRACE)
			{
				nextLine();
			}
		}
	}

	template<typename T>
	void visit(T* symbol)
	{
		symbol->accept(*this);
	}

	template<typename T>
	void visit(cpp::symbol<T> symbol)
	{
		if(symbol.p != 0)
		{
			visit(symbol.p);
		}
	}
};

template<typename T>
cpp::symbol<T> makeSymbol(T* p)
{
	return cpp::symbol<T>(p);
};

void printSymbol(cpp::declaration_seq* p)
{
	SymbolPrinter visitor;
	visitor.visit(makeSymbol(p));
}
