
#include "semantic.h"

#include "cpptree.h"
#include "printer.h"

#include <fstream>

#define SYMBOL_NAME(T) (typeid(T).name() + 12)

template<typename OutputStreamType>
struct TreePrinter // TODO: better name
{
	OutputStreamType& out;
	bool visited;
	TreePrinter(OutputStreamType& out)
		: out(out), visited(false)
	{
	}

	void visit(cpp::terminal_identifier symbol)
	{
	}

	void visit(cpp::terminal_string symbol)
	{
	}

	void visit(cpp::terminal_choice2 symbol)
	{
	}

	template<LexTokenId id>
	void visit(cpp::terminal<id> symbol)
	{
	}

	template<typename T>
	void visit(T* symbol)
	{
		if(typeid(T) != typeid(*symbol)) // if abstract
		{
			symbol->accept(*this);
		}
		else
		{
			if(visited)
			{
				out << ", ";
			}
			visited = true;
			out << SYMBOL_NAME(*symbol);
#if 0 // don't print children
			out << '(';
			TreePrinter<OutputStreamType> tmp(out);
			symbol->accept(tmp);
			out << ')';
#endif
		}
	}

	template<typename T>
	void visit(cpp::symbol<T> symbol)
	{
		if(symbol.p != 0)
		{
			visit(symbol.p);
		}
	}

	void visit(cpp::declaration* symbol)
	{
	}

	void visit(cpp::member_declaration* symbol)
	{
	}

	void visit(cpp::statement* symbol)
	{
	}
};

struct SymbolPrinter
{
	std::ofstream out;
	TokenPrinter<std::ostream> printer;

	SymbolPrinter(const char* path)
		: out(path),
		printer(out)
	{
	}

	void visit(cpp::terminal_identifier symbol)
	{
		printer.printToken(boost::wave::T_IDENTIFIER, symbol.value);
	}

	void visit(cpp::terminal_string symbol)
	{
		printer.printToken(boost::wave::T_STRINGLIT, symbol.value);
	}

	void visit(cpp::terminal_choice2 symbol)
	{
		printer.printToken(symbol.id, symbol.value);
	}

	template<LexTokenId id>
	void visit(cpp::terminal<id> symbol)
	{
		if(symbol.value != 0)
		{
			printer.printToken(id, symbol.value);
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

#if 1
	void visit(cpp::declaration* symbol)
	{
		symbol->accept(*this);
		out << " // ";
		TreePrinter<std::ofstream> tmp(out);
		symbol->accept(tmp);
	}

	void visit(cpp::member_declaration* symbol)
	{
		symbol->accept(*this);
		out << " // ";
		TreePrinter<std::ofstream> tmp(out);
		symbol->accept(tmp);
	}

	void visit(cpp::statement* symbol)
	{
		symbol->accept(*this);
		out << " // ";
		TreePrinter<std::ofstream> tmp(out);
		symbol->accept(tmp);
	}
#endif
};

#define TREEWALKER_DEFAULT \
	void visit(cpp::terminal_identifier symbol) \
	{ \
	} \
	void visit(cpp::terminal_string symbol) \
	{ \
	} \
	void visit(cpp::terminal_choice2 symbol) \
	{ \
	} \
	template<LexTokenId id> \
	void visit(cpp::terminal<id> symbol) \
	{ \
		if(symbol.value != 0) \
		{ \
		} \
	} \
	template<typename T> \
	void visit(T* symbol) \
	{ \
		symbol->accept(*this); \
	} \
	template<typename T> \
	void visit(cpp::symbol<T> symbol) \
	{ \
		if(symbol.p != 0) \
		{ \
			visit(symbol.p); \
		} \
	}

#include <iostream>
#include <list>

struct SemanticError
{
	SemanticError()
	{
	}
};


struct Scope;

struct Declaration
{
	const char* name;
	const char* type;
	Scope* enclosed;
	bool isTypedef;

	Declaration(const char* name, const char* type, Scope* enclosed, bool isTypedef)
		: name(0), type(0), enclosed(enclosed), isTypedef(isTypedef)
	{
	}
};

struct Scope
{
	Scope* parent;
	typedef std::list<Declaration> Declarations;
	Declarations declarations;

	Scope()
		: parent(0)
	{
	}
};

Scope global;

size_t gDepth = 0; // TMP HACK

void printName(const char* caption, const char* id)
{
	for(size_t i = 0; i != gDepth; ++i)
	{
		std::cout << '\t';
	}
	std::cout << caption << ": " << id << std::endl;
}

Scope* gScope = &global;

void pointOfDeclaration(const char* name, const char* type, Scope* enclosed, bool isTypedef)
{
	printName(type == 0 ? "<unknown>" : type, name);
	gScope->declarations.push_back(Declaration(name, type, enclosed, isTypedef));
}

void pushScope(Scope* scope)
{
	++gDepth;
	scope->parent = gScope;
	gScope = scope;
}
Scope* popScope()
{
	--gDepth;
	if(gScope->parent == 0)
	{
		throw SemanticError();
	}
	Scope* result = gScope;
	gScope = gScope->parent;
	return result;
}


struct Walker
{


struct TypeNameWalker
{
	TREEWALKER_DEFAULT;

	const char* id;
	TypeNameWalker()
		: id(0)
	{
	}

	void visit(cpp::identifier* symbol)
	{
		id = symbol->value.value;
	}
	void visit(cpp::nested_name_specifier* symbol)
	{
		TypeNameWalker walker;
		symbol->accept(walker);
	}
	void visit(cpp::simple_template_id* symbol) 
	{
		id = symbol->id->value.value;
		// TODO args
	}
	void visit(cpp::simple_type_specifier_builtin* symbol)
	{
		// TODO
		id = symbol->value.value;
	}
};

struct DeclaratorIdWalker
{
	TREEWALKER_DEFAULT;

	const char* id;
	DeclaratorIdWalker()
		: id(0)
	{
	}

	void visit(cpp::identifier* symbol)
	{
		id = symbol->value.value;
	}
	void visit(cpp::nested_name_specifier* symbol)
	{
		TypeNameWalker walker;
		symbol->accept(walker);
	}
	void visit(cpp::simple_template_id* symbol) 
	{
		id = symbol->id->value.value;
	}
	void visit(cpp::operator_function_id* symbol) 
	{
		// TODO
		id = "operator ()";
	}
	void visit(cpp::conversion_function_id* symbol) 
	{
		// TODO
		id = "operator T";
	}
	void visit(cpp::destructor_id* symbol) 
	{
		// TODO
		id = "~T";
	}
	void visit(cpp::template_id_operator_function* symbol) 
	{
		// TODO
		id = "operator () <>";
	}
};

struct DeclaratorWalker
{
	TREEWALKER_DEFAULT;

	const char* id;
	Scope* paramScope;
	DeclaratorWalker()
		: id(0), paramScope(0)
	{
	}

	void visit(cpp::declarator_id* symbol)
	{
		DeclaratorIdWalker walker;
		symbol->accept(walker);
		id = walker.id;
	}
	void visit(cpp::parameter_declaration_clause* symbol)
	{
		pushScope(new Scope); // parameter scope
		symbol->accept(*this);
		paramScope = popScope(); // parameter scope
	}
	void visit(cpp::parameter_declaration* symbol)
	{
		DeclarationWalker walker;
		symbol->accept(walker);
	}
};

struct ClassSpecifierWalker
{
	TREEWALKER_DEFAULT;

	const char* id;
	ClassSpecifierWalker()
		: id(0)
	{
	}

	void visit(cpp::class_head* symbol)
	{
		TypeNameWalker walker;
		symbol->accept(walker);
		id = walker.id;
		// TODO: distinguish between class, struct, union
		Scope* scope = new Scope;
		pointOfDeclaration(id, "class", scope, false); // 3.3.1.3
		pushScope(scope); // 3.3.6.1.1 // class scope
	}
	void visit(cpp::member_declaration* symbol)
	{
		DeclarationWalker walker;
		symbol->accept(walker);
	}
};

struct TypeSpecifierWalker
{
	TREEWALKER_DEFAULT;

	const char* id;
	TypeSpecifierWalker()
		: id(0)
	{
	}

	void visit(cpp::simple_type_specifier* symbol)
	{
		TypeNameWalker walker;
		symbol->accept(walker);
		id = walker.id;
	}
	void visit(cpp::elaborated_type_specifier* symbol)
	{
		TypeNameWalker walker;
		symbol->accept(walker);
		id = walker.id;
	}
	void visit(cpp::typename_specifier* symbol)
	{
		TypeNameWalker walker;
		symbol->accept(walker);
		id = walker.id;
	}
	void visit(cpp::class_specifier* symbol)
	{
		ClassSpecifierWalker walker;
		symbol->accept(walker);
		id = walker.id;
		popScope(); // class scope
	}
	void visit(cpp::enum_specifier* symbol)
	{
		// TODO
		id = symbol->id->value.value;
	}
};

struct DeclSpecifierWalker
{
	TREEWALKER_DEFAULT;

	const char* type;
	bool isTypedef;
	DeclSpecifierWalker()
		: type(0), isTypedef(false)
	{
	}

	void visit(cpp::type_specifier_noncv* symbol)
	{
		TypeSpecifierWalker walker;
		symbol->accept(walker);
		type = walker.id;
	}
	void visit(cpp::decl_specifier_default* symbol)
	{
		if(symbol->id == cpp::decl_specifier_default::TYPEDEF)
		{
			isTypedef = true;
		}
	}
};

struct CompoundStatementWalker
{
	TREEWALKER_DEFAULT;

	void visit(cpp::simple_declaration* symbol)
	{
		DeclarationWalker walker;
		symbol->accept(walker);
	}
};

struct DeclarationWalker
{
	TREEWALKER_DEFAULT;

	const char* type;
	bool isTypedef;
	DeclarationWalker()
		: type(0)
	{
	}

	void visit(cpp::decl_specifier_seq* symbol)
	{
		DeclSpecifierWalker walker;
		symbol->accept(walker);
		type = walker.type;
		isTypedef = walker.isTypedef;
	}

	void visit(cpp::declarator* symbol)
	{
		DeclaratorWalker walker;
		symbol->accept(walker);
		pointOfDeclaration(walker.id, type, walker.paramScope, isTypedef); // 3.3.1.1
		if(walker.paramScope != 0)
		{
			pushScope(walker.paramScope); // 3.3.2.1 parameter scope
		}
	}
	void visit(cpp::abstract_declarator* symbol)
	{
		DeclaratorWalker walker;
		symbol->accept(walker);
	}
	void visit(cpp::member_declarator_bitfield* symbol)
	{
		const char* id = symbol->id->value.value;
	}

	void visit(cpp::initializer* symbol)
	{
		// TODO
	}
	void visit(cpp::compound_statement* symbol)
	{
		pushScope(new Scope); // local scope
		CompoundStatementWalker walker;
		symbol->accept(walker);
		popScope(); // local scope
	}

	void visit(cpp::constructor_definition* symbol)
	{
		symbol->accept(*this);
		popScope(); // parameter scope
	}
	void visit(cpp::function_definition_suffix* symbol)
	{
		symbol->accept(*this);
		popScope(); // parameter scope
	}
};


struct RootWalker
{
	TREEWALKER_DEFAULT;

	void visit(cpp::general_declaration* symbol)
	{
		DeclarationWalker walker;
		symbol->accept(walker);
	}
	void visit(cpp::template_declaration* symbol)
	{
		// TODO
		symbol->accept(*this);
	}
};

};

template<typename T>
cpp::symbol<T> makeSymbol(T* p)
{
	return cpp::symbol<T>(p);
};

void printSymbol(cpp::declaration_seq* p, const char* path)
{
	Walker::RootWalker walker;
	walker.visit(makeSymbol(p));
	SymbolPrinter visitor(path);
	visitor.visit(makeSymbol(p));
}

void printSymbol(cpp::statement_seq* p, const char* path)
{
	Walker::RootWalker walker;
	walker.visit(makeSymbol(p));
	SymbolPrinter visitor(path);
	visitor.visit(makeSymbol(p));
}
