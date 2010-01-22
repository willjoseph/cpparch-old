
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

typedef TokenPrinter<std::ofstream> FileTokenPrinter;

struct WalkerContext
{
	std::ofstream out;
	FileTokenPrinter printer;

	WalkerContext(const char* path)
		: out(path),
		printer(out)
	{
	}
};

struct PrintingWalker
{
	FileTokenPrinter& printer;
	PrintingWalker(FileTokenPrinter& printer)
		: printer(printer)
	{
	}
	template<typename T>
	void printSymbol(T* symbol)
	{
		SymbolPrinter walker(*this);
		symbol->accept(walker);
	}
};


struct SymbolPrinter : PrintingWalker
{
	SymbolPrinter(PrintingWalker& base)
		: PrintingWalker(base)
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

#if 0
	void visit(cpp::declaration* symbol)
	{
		symbol->accept(*this);
		printer.out << " // ";
		TreePrinter<std::ofstream> tmp(printer.out);
		symbol->accept(tmp);
	}

	void visit(cpp::member_declaration* symbol)
	{
		symbol->accept(*this);
		printer.out << " // ";
		TreePrinter<std::ofstream> tmp(printer.out);
		symbol->accept(tmp);
	}

	void visit(cpp::statement* symbol)
	{
		symbol->accept(*this);
		printer.out << " // ";
		TreePrinter<std::ofstream> tmp(printer.out);
		symbol->accept(tmp);
	}
#endif
};

#define TREEWALKER_DEFAULT \
	void visit(cpp::terminal_identifier symbol) \
	{ \
		printer.printToken(boost::wave::T_IDENTIFIER, symbol.value); \
	} \
	void visit(cpp::terminal_string symbol) \
	{ \
		printer.printToken(boost::wave::T_STRINGLIT, symbol.value); \
	} \
	void visit(cpp::terminal_choice2 symbol) \
	{ \
		printer.printToken(symbol.id, symbol.value); \
	} \
	template<LexTokenId id> \
	void visit(cpp::terminal<id> symbol) \
	{ \
		if(symbol.value != 0) \
		{ \
			printer.printToken(id, symbol.value); \
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

struct Scope;

struct Declaration
{
	Scope* scope;
	Identifier name;
	Declaration* type;
	Scope* enclosed;
	bool isTypedef;

	Declaration(Scope* scope, Identifier name, Declaration* type, Scope* enclosed, bool isTypedef)
		: scope(scope), name(name), type(type), enclosed(enclosed), isTypedef(isTypedef)
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

Scope global("$global");


size_t gDepth = 0; // TMP HACK

Scope* gScope = &global;

// special-case
Declaration gUndeclared(&global, "$undeclared", 0, &global, false);
Declaration gAnonymous(&global, "$anonymous", 0, &global, false);

// symbol types
Declaration gNamespace(&global, "$namespace", 0, 0, false);
Declaration gBuiltin(&global, "$builtin", 0, 0, false);
Declaration gClass(&global, "$class", 0, 0, false);
Declaration gCtor(&global, "$ctor", 0, 0, false);

struct WalkerBase : public PrintingWalker
{
	WalkerBase(FileTokenPrinter& printer)
		: PrintingWalker(printer)
	{
	}

	Declaration* findDeclaration(Scope& scope, Identifier id)
	{
		for(Scope::Declarations::iterator i = scope.declarations.begin(); i != scope.declarations.end(); ++i)
		{
			if((*i).name == id)
			{
				return &(*i);
			}
		}
		if(scope.parent != 0)
		{
			return findDeclaration(*scope.parent, id);
		}
		printer.out << "/* undeclared: " << id << " */";
		return &gUndeclared;
	}


	void printName(Scope* scope)
	{
		if(scope != &global)
		{
			printName(scope->parent);
			printer.out << scope->name << "::";
		}
	}

	void printName(Declaration* name)
	{
		if(name == 0)
		{
			printer.out << "<unknown>";
		}
		else
		{
			printName(name->scope);
			printer.out << name->name;
		}
	}

	void printName(Declaration* type, Declaration* name)
	{
		printer.out << "/* ";
		printName(type);
		printer.out << ": ";
		printName(name);
		printer.out << " */";
	}

	Declaration* pointOfDeclaration(Scope* parent, const char* name, Declaration* type, Scope* enclosed, bool isTypedef)
	{
		parent->declarations.push_back(Declaration(parent, name, type, enclosed, isTypedef));
		if(enclosed != 0)
		{
			enclosed->name = name;
		}
		Declaration* result = &parent->declarations.back();
		printName(type, result);
		return result;
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

	void checkName(Identifier id)
	{
		printer.out << "/* lookup: ";
		Declaration* declaration = findDeclaration(*gScope, id);
		if(declaration == 0)
		{
			printer.out << "failed: ";
		}
		printer.out  << id << " */";
	}
};

#define SEMANTIC_ASSERT(condition) if(!(condition)) { throw SemanticError(); }

struct Walker
{

struct NestedNameSpecifierWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Scope* scope;
	NestedNameSpecifierWalker(const WalkerBase& base, Scope* scope = gScope)
		: WalkerBase(base), scope(scope)
	{
	}
	void visit(cpp::identifier* symbol)
	{
		Identifier id = symbol->value.value;
		Declaration* declaration = findDeclaration(*scope, id);
		SEMANTIC_ASSERT(declaration->enclosed != 0);
		scope = declaration->enclosed;
		printSymbol(symbol);
	}
	void visit(cpp::simple_template_id* symbol)
	{
		Identifier id = symbol->id->value.value;
		Declaration* declaration = findDeclaration(*scope, id);
		SEMANTIC_ASSERT(declaration->enclosed != 0);
		scope = declaration->enclosed;
		printSymbol(symbol);
	}
};

struct TypeSpecifierWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Declaration* declaration;
	Scope* scope;
	TypeSpecifierWalker(const WalkerBase& base)
		: WalkerBase(base), declaration(0), scope(gScope)
	{
	}

	void visit(cpp::identifier* symbol)
	{
		Identifier id = symbol->value.value;
		declaration = findDeclaration(*scope, id);
		printSymbol(symbol);
	}
	void visit(cpp::simple_type_specifier_name* symbol)
	{
		if(symbol->isGlobal.value != 0)
		{
			scope = &global;
		}
		symbol->accept(*this);
	}
	void visit(cpp::simple_type_specifier_template* symbol)
	{
		if(symbol->isGlobal.value != 0)
		{
			scope = &global;
		}
		symbol->accept(*this);
	}
	void visit(cpp::nested_name_specifier* symbol)
	{
		NestedNameSpecifierWalker walker(*this, scope);
		symbol->accept(walker);
		scope = walker.scope;
	}
	void visit(cpp::simple_template_id* symbol) 
	{
		Identifier id = symbol->id->value.value;
		declaration = findDeclaration(*scope, id);
		// TODO args
		printSymbol(symbol);
	}
	void visit(cpp::simple_type_specifier_builtin* symbol)
	{
		// TODO
		declaration = &gBuiltin;
		printSymbol(symbol);
	}
};

struct DeclaratorIdWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Identifier id;
	Scope* scope;
	DeclaratorIdWalker(const WalkerBase& base)
		: WalkerBase(base), id(0), scope(gScope)
	{
	}

	void visit(cpp::qualified_id_global* symbol) 
	{
		scope = &global;
		symbol->accept(*this);
	}
	void visit(cpp::qualified_id_default* symbol) 
	{
		if(symbol->isGlobal.value != 0)
		{
			scope = &global;
		}
		symbol->accept(*this);
	}
	void visit(cpp::identifier* symbol)
	{
		id = symbol->value.value;
		printSymbol(symbol);
	}
	void visit(cpp::nested_name_specifier* symbol)
	{
		NestedNameSpecifierWalker walker(*this, scope);
		symbol->accept(walker);
		scope = walker.scope;
	}
	void visit(cpp::simple_template_id* symbol) 
	{
		id = symbol->id->value.value;
		printSymbol(symbol);
	}
	void visit(cpp::operator_function_id* symbol) 
	{
		// TODO
		id = "operator <op>";
		printSymbol(symbol);
	}
	void visit(cpp::conversion_function_id* symbol) 
	{
		// TODO
		id = "operator T";
		printSymbol(symbol);
	}
	void visit(cpp::destructor_id* symbol) 
	{
		// TODO
		id = "~T";
		printSymbol(symbol);
	}
	void visit(cpp::template_id_operator_function* symbol) 
	{
		// TODO
		id = "operator () <>";
		printSymbol(symbol);
	}
};

struct DeclaratorWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Identifier id;
	Scope* scope;
	Scope* paramScope;
	DeclaratorWalker(const WalkerBase& base)
		: WalkerBase(base), id(0), scope(0), paramScope(0)
	{
	}

	void visit(cpp::declarator_id* symbol)
	{
		DeclaratorIdWalker walker(*this);
		symbol->accept(walker);
		id = walker.id;
		scope = walker.scope;
	}
	void visit(cpp::declarator_suffix_function* symbol)
	{
		pushScope(new Scope(id)); // parameter scope
		symbol->accept(*this);
		paramScope = popScope(); // parameter scope (store reference for later resumption)
	}
	void visit(cpp::parameter_declaration* symbol)
	{
		DeclarationWalker walker(*this);
		symbol->accept(walker);
	}
};

struct ClassHeadWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Declaration* declaration;
	Scope* scope;
	ClassHeadWalker(const WalkerBase& base)
		: WalkerBase(base), declaration(&gAnonymous), scope(gScope)
	{
	}

	void visit(cpp::identifier* symbol)
	{
		Identifier id = symbol->value.value;
		printSymbol(symbol);
		declaration = pointOfDeclaration(scope, id, &gClass, 0, false); // 3.3.1.3 The point of declaration for a class first declared by a class-specifier is immediately after the identifier or simple-template-id (if any) in its class-head
	}
	void visit(cpp::nested_name_specifier* symbol)
	{
		NestedNameSpecifierWalker walker(*this);
		symbol->accept(walker);
		scope = walker.scope;
	}
	void visit(cpp::simple_template_id* symbol) 
	{
		Identifier id = symbol->id->value.value;
		printSymbol(symbol);
		declaration = pointOfDeclaration(scope, id, &gClass, 0, false); // 3.3.1.3 The point of declaration for a class first declared by a class-specifier is immediately after the identifier or simple-template-id (if any) in its class-head
		// TODO args
	}
	void visit(cpp::base_clause* symbol) 
	{
		// TODO
		printSymbol(symbol);
	}
};

typedef std::pair<Scope*, cpp::function_definition_suffix*> FunctionDefinition;
typedef std::vector<FunctionDefinition> FunctionDefinitions;

struct ClassSpecifierWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Declaration* declaration;
	FunctionDefinitions deferred;
	ClassSpecifierWalker(const WalkerBase& base)
		: WalkerBase(base), declaration(0)
	{
	}
	void walkDeferred()
	{
		for(FunctionDefinitions::const_iterator i = deferred.begin(); i != deferred.end(); ++i)
		{
			printer.printToken(boost::wave::T_IDENTIFIER, declaration->name);
			printer.printToken(boost::wave::T_COLON_COLON, "::");
			printer.printToken(boost::wave::T_IDENTIFIER, "<deferred>");
			printer.printToken(boost::wave::T_LEFTPAREN, "(");
			printer.printToken(boost::wave::T_IDENTIFIER, "<params>");
			printer.printToken(boost::wave::T_RIGHTPAREN, ")");

			FunctionDefinitionSuffixWalker walker(*this, (*i).first);
			(*i).second->accept(walker);
		}
	};

	void visit(cpp::class_head* symbol)
	{
		ClassHeadWalker walker(*this);
		symbol->accept(walker);
		declaration = walker.declaration;
		Scope* scope = new Scope(declaration->name);
		declaration->enclosed = scope;
		pushScope(scope); // 3.3.6.1.1 // class scope
	}
	void visit(cpp::member_declaration* symbol)
	{
		DeclarationWalker walker(*this, &deferred);
		symbol->accept(walker);
	}
};

struct DeclSpecifierSeqWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Declaration* declaration;
	bool isTypedef;
	DeclSpecifierSeqWalker(const WalkerBase& base)
		: WalkerBase(base), declaration(0), isTypedef(false)
	{
	}

	void visit(cpp::simple_type_specifier* symbol)
	{
		TypeSpecifierWalker walker(*this);
		symbol->accept(walker);
		declaration = walker.declaration;
	}
	void visit(cpp::elaborated_type_specifier* symbol)
	{
		TypeSpecifierWalker walker(*this);
		symbol->accept(walker);
		// TODO 3.3.1.6: complicated rules on point-of-declaration for elaborated-type-specifier
		declaration = walker.declaration;
	}
	void visit(cpp::typename_specifier* symbol)
	{
		TypeSpecifierWalker walker(*this);
		symbol->accept(walker);
		// TODO point-of-declaration for typename-specifier
		declaration = walker.declaration;
	}
	void visit(cpp::class_specifier* symbol)
	{
		ClassSpecifierWalker walker(*this);
		symbol->accept(walker);
		declaration = walker.declaration;
		walker.walkDeferred();
		popScope(); // class scope
	}
	void visit(cpp::enum_specifier* symbol)
	{
		// TODO 
		// + anonymous enums
		//Identifier id = symbol->id->value.value;
		printSymbol(symbol);
	}
	void visit(cpp::decl_specifier_default* symbol)
	{
		if(symbol->id == cpp::decl_specifier_default::TYPEDEF)
		{
			isTypedef = true;
		}
		printSymbol(symbol);
	}
};

struct CompoundStatementWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	CompoundStatementWalker(const WalkerBase& base)
		: WalkerBase(base)
	{
	}

	void visit(cpp::simple_declaration* symbol)
	{
		DeclarationWalker walker(*this);
		symbol->accept(walker);
	}
};


struct FunctionDefinitionSuffixWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Scope* paramScope;
	FunctionDefinitionSuffixWalker(const WalkerBase& base, Scope* paramScope)
		: WalkerBase(base), paramScope(paramScope)
	{

	}
	void visit(cpp::compound_statement* symbol)
	{
		if(paramScope != 0)
		{
			pushScope(paramScope); // 3.3.2.1 parameter scope
		}
		pushScope(new Scope("local")); // local scope
		CompoundStatementWalker walker(*this);
		symbol->accept(walker);
		popScope(); // local scope
		if(paramScope != 0)
		{
			popScope(); // parameter scope
		}
	}
};

struct DeclarationWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Declaration* type;
	bool isTypedef;
	Scope* paramScope;
	FunctionDefinitions* deferred;

	DeclarationWalker(const WalkerBase& base, FunctionDefinitions* deferred = 0)
		: WalkerBase(base), type(&gCtor), isTypedef(false), paramScope(0), deferred(deferred)
	{
	}

	void visit(cpp::decl_specifier_seq* symbol)
	{
		DeclSpecifierSeqWalker walker(*this);
		symbol->accept(walker);
		type = walker.declaration;
		isTypedef = walker.isTypedef;
	}
	void visit(cpp::type_specifier_seq* symbol)
	{
		DeclSpecifierSeqWalker walker(*this);
		symbol->accept(walker);
		type = walker.declaration;
	}

	void visit(cpp::declarator* symbol)
	{
		DeclaratorWalker walker(*this);
		symbol->accept(walker);
		pointOfDeclaration(walker.scope, walker.id, type, isTypedef ? type->enclosed : walker.paramScope, isTypedef); // 3.3.1.1
		paramScope = walker.paramScope;
	}
	void visit(cpp::abstract_declarator* symbol)
	{
		DeclaratorWalker walker(*this);
		symbol->accept(walker);
	}
	void visit(cpp::member_declarator_bitfield* symbol)
	{
		Identifier id = symbol->id->value.value;
		printSymbol(symbol);
		pointOfDeclaration(gScope, id, type, 0, isTypedef); // 3.3.1.1
	}

	void visit(cpp::initializer* symbol)
	{
		// TODO
		printSymbol(symbol);
	}
	void visit(cpp::function_definition_suffix* symbol)
	{
		if(deferred != 0)
		{
			printer.printToken(boost::wave::T_SEMICOLON, ";");
			deferred->push_back(FunctionDefinition(paramScope, symbol));
		}
		else
		{
			FunctionDefinitionSuffixWalker walker(*this, paramScope);
			symbol->accept(walker);
		}
	}
};

struct NamespaceWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;


	NamespaceWalker(WalkerContext& context)
		: WalkerBase(context.printer)
	{
	}

	void visit(cpp::namespace_definition* symbol)
	{
		Identifier id = symbol->id.p == 0 ? "$anonymous" : symbol->id->value.value;
		Scope* scope = new Scope(id);
		pointOfDeclaration(gScope, id, &gNamespace, scope, false);
		pushScope(scope);
		symbol->accept(*this);
		popScope();
	}
	void visit(cpp::general_declaration* symbol)
	{
		DeclarationWalker walker(*this);
		symbol->accept(walker);
	}
	// occurs in for-init-statement
	void visit(cpp::simple_declaration* symbol)
	{
		DeclarationWalker walker(*this);
		symbol->accept(walker);
	}
	void visit(cpp::template_declaration* symbol)
	{
		// TODO
		symbol->accept(*this);
	}
	void visit(cpp::selection_statement* symbol)
	{
		pushScope(new Scope("selection"));
		symbol->accept(*this);
		popScope();
	}
	void visit(cpp::iteration_statement* symbol)
	{
		pushScope(new Scope("iteration"));
		symbol->accept(*this);
		popScope();
	}
	void visit(cpp::condition_init* symbol)
	{
		DeclarationWalker walker(*this);
		symbol->accept(walker);
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
	try
	{
		WalkerContext context(path);
		Walker::NamespaceWalker walker(context);
		walker.visit(makeSymbol(p));
	}
	catch(SemanticError&)
	{
	}
#if 0
	SymbolPrinter visitor(context);
	visitor.visit(makeSymbol(p));
#endif
}

void printSymbol(cpp::statement_seq* p, const char* path)
{
	try
	{
		WalkerContext context(path);
		Walker::NamespaceWalker walker(context);
		walker.visit(makeSymbol(p));
	}
	catch(SemanticError&)
	{
	}
#if 0
	SymbolPrinter visitor(context);
	visitor.visit(makeSymbol(p));
#endif
}
