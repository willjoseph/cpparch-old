
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

typedef const char* Identifier;

const Identifier TYPE_BUILTIN = "builtin";
const Identifier TYPE_CLASS = "class";

struct Scope;

struct Declaration
{
	Identifier name;
	Identifier type;
	Scope* enclosed;
	bool isTypedef;

	Declaration(Identifier name, Identifier type, Scope* enclosed, bool isTypedef)
		: name(name), type(type), enclosed(enclosed), isTypedef(isTypedef)
	{
	}
};

struct Scope
{
	Scope* parent;
	Identifier name;
	typedef std::list<Declaration> Declarations;
	Declarations declarations;

	Scope(Identifier name)
		: parent(0), name(name)
	{
	}
};

inline Declaration* findDeclaration(Scope& scope, Identifier id)
{
	for(Scope::Declarations::iterator i = scope.declarations.begin(); i != scope.declarations.end(); ++i)
	{
		if((*i).name == id)
		{
			std::cout << scope.name << "::";
			return &(*i);
		}
	}
	if(scope.parent != 0)
	{
		return findDeclaration(*scope.parent, id);
	}
	return 0;
}


struct GlobalScope : public Scope
{
	GlobalScope() : Scope("")
	{
		declarations.push_back(Declaration(TYPE_BUILTIN, 0, 0, true));
	}
} global;


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

Declaration* pointOfDeclaration(const char* name, const char* type, Scope* enclosed, bool isTypedef)
{
	printName(type == 0 ? "<none>" : type, name);
	gScope->declarations.push_back(Declaration(name, type, enclosed, isTypedef));
	if(enclosed != 0)
	{
		enclosed->name = name;
	}
	return &gScope->declarations.back();
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

inline void checkName(Identifier id)
{
	std::cout << "lookup: ";
	Declaration* declaration = findDeclaration(*gScope, id);
	if(declaration == 0)
	{
		std::cout << "failed: ";
	}
	std::cout  << id << std::endl;
}


inline Declaration* declareClass(Identifier id)
{
	// TODO: distinguish between class, struct, union
	return pointOfDeclaration(id, TYPE_CLASS, 0, false);
};


struct Walker
{


struct TypeNameWalker
{
	TREEWALKER_DEFAULT;

	Identifier id;
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
		// TODO context
	}
	void visit(cpp::simple_template_id* symbol) 
	{
		id = symbol->id->value.value;
		// TODO args
	}
	void visit(cpp::simple_type_specifier_builtin* symbol)
	{
		// TODO
		id = TYPE_BUILTIN;
	}
};

struct DeclaratorIdWalker
{
	TREEWALKER_DEFAULT;

	Identifier id;
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
		// TODO context
	}
	void visit(cpp::simple_template_id* symbol) 
	{
		id = symbol->id->value.value;
	}
	void visit(cpp::operator_function_id* symbol) 
	{
		// TODO
		id = "operator <op>";
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

	Identifier id;
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
	void visit(cpp::declarator_suffix_function* symbol)
	{
		pushScope(new Scope("parameter")); // parameter scope
		symbol->accept(*this);
		paramScope = popScope(); // parameter scope (store reference for later resumption)
	}
	void visit(cpp::parameter_declaration* symbol)
	{
		DeclarationWalker walker;
		symbol->accept(walker);
	}
};

struct ClassHeadWalker
{
	TREEWALKER_DEFAULT;

	Declaration* declaration;
	ClassHeadWalker()
		: declaration(0)
	{
	}

	void visit(cpp::identifier* symbol)
	{
		Identifier id = symbol->value.value;
		declaration = declareClass(id); // 3.3.1.3 The point of declaration for a class first declared by a class-specifier is immediately after the identifier or simple-template-id (if any) in its class-head
	}
	void visit(cpp::nested_name_specifier* symbol)
	{
		TypeNameWalker walker;
		symbol->accept(walker);
		// TODO context
	}
	void visit(cpp::simple_template_id* symbol) 
	{
		Identifier id = symbol->id->value.value;
		declaration = declareClass(id); // 3.3.1.3 The point of declaration for a class first declared by a class-specifier is immediately after the identifier or simple-template-id (if any) in its class-head
		// TODO args
	}

	void visit(cpp::class_head* symbol)
	{
		TypeNameWalker walker;
		symbol->accept(walker);
	}
};

struct ClassSpecifierWalker
{
	TREEWALKER_DEFAULT;

	Identifier id;
	ClassSpecifierWalker()
		: id(0)
	{
	}

	void visit(cpp::class_head* symbol)
	{
		ClassHeadWalker walker;
		symbol->accept(walker);
		id = walker.declaration->name;
		Scope* scope = new Scope(id);
		walker.declaration->enclosed = scope;
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

	Identifier id;
	TypeSpecifierWalker()
		: id(0)
	{
	}

	void visit(cpp::simple_type_specifier* symbol)
	{
		TypeNameWalker walker;
		symbol->accept(walker);
		id = walker.id;
		checkName(id);
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

	Identifier type;
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

	Identifier type;
	bool isTypedef;
	Scope* paramScope;
	DeclarationWalker()
		: type(0), isTypedef(false), paramScope(0)
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
		paramScope = walker.paramScope;
	}
	void visit(cpp::abstract_declarator* symbol)
	{
		DeclaratorWalker walker;
		symbol->accept(walker);
	}
	void visit(cpp::member_declarator_bitfield* symbol)
	{
		Identifier id = symbol->id->value.value;
		pointOfDeclaration(id, type, 0, isTypedef); // 3.3.1.1
	}

	void visit(cpp::initializer* symbol)
	{
		// TODO
	}
	void visit(cpp::compound_statement* symbol)
	{
		if(paramScope != 0)
		{
			pushScope(paramScope); // 3.3.2.1 parameter scope
		}
		pushScope(new Scope("local")); // local scope
		CompoundStatementWalker walker;
		symbol->accept(walker);
		popScope(); // local scope
		if(paramScope != 0)
		{
			popScope(); // parameter scope
		}
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
