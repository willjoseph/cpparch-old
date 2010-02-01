
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

typedef cpp::terminal_identifier Identifier;

inline Identifier makeIdentifier(const char* value)
{
	Identifier result = { value };
	return result;
}

struct Scope;

struct Declaration
{
	Scope* scope;
	Identifier name;
	Declaration* type;
	Scope* enclosed;
	bool isTypedef;
	bool isFriend;

	Declaration(Scope* scope, Identifier name, Declaration* type, Scope* enclosed, bool isTypedef, bool isFriend = false)
		: scope(scope), name(name), type(type), enclosed(enclosed), isTypedef(isTypedef), isFriend(isFriend)
	{
	}
};

struct Scope
{
	Scope* parent;
	Identifier name;
	typedef std::list<Declaration> Declarations;
	Declarations declarations;
	bool isElt;

	Scope(Identifier name, bool isElt = false)
		: parent(0), name(name), isElt(isElt)
	{
	}
};

Scope global(makeIdentifier("$global"), true);

struct WalkerContext
{
	std::ofstream out;
	FileTokenPrinter printer;
	Scope* scope;

	WalkerContext(const char* path)
		: out(path),
		printer(out),
		scope(&global)
	{
	}
};


// special-case
Declaration gUndeclared(&global, makeIdentifier("$undeclared"), 0, &global, false);
Declaration gAnonymous(&global, makeIdentifier("$anonymous"), 0, &global, false);
Declaration gFriend(&global, makeIdentifier("$friend"), 0, &global, false);

// symbol types
Declaration gNamespace(&global, makeIdentifier("$namespace"), 0, 0, false);
Declaration gBuiltin(&global, makeIdentifier("$builtin"), 0, 0, false);
Declaration gClassFwd(&global, makeIdentifier("$classfwd"), 0, 0, false);
Declaration gClass(&global, makeIdentifier("$class"), 0, 0, false);
Declaration gEnum(&global, makeIdentifier("$enum"), 0, 0, false);
Declaration gCtor(&global, makeIdentifier("$ctor"), 0, 0, false);

struct WalkerBase : public PrintingWalker
{
	WalkerContext& context;
	WalkerBase(WalkerContext& context)
		: PrintingWalker(context.printer), context(context)
	{
	}

	Declaration* findDeclaration(Scope::Declarations& declarations, const Identifier& id)
	{
		for(Scope::Declarations::iterator i = declarations.begin(); i != declarations.end(); ++i)
		{
			if((*i).name.value == id.value)
			{
				return &(*i);
			}
		}
		return 0;
	}

	Declaration* findDeclaration(Scope& scope, const Identifier& id)
	{
		Declaration* result = findDeclaration(scope.declarations, id);
		if(result != 0)
		{
			return result;
		}
		if(scope.parent != 0)
		{
			return findDeclaration(*scope.parent, id);
		}
		printPosition(id.position);
		printer.out << "/* undeclared: " << id.value << " */";
		return &gUndeclared;
	}


	void printName(Scope* scope)
	{
		if(scope != &global)
		{
			printName(scope->parent);
			printer.out << scope->name.value << "::";
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
			printer.out << name->name.value;
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

	inline void printPosition(const LexFilePosition& position)
	{
		std::cout << position.get_file() << "(" << position.get_line() << "): ";
	}

	Declaration* getBaseType(Declaration* type)
	{
		while(type->isTypedef)
		{
			type = type->type;
		}
		return type;
	}

	Declaration* pointOfDeclaration(Scope* parent, const Identifier& name, Declaration* type, Scope* enclosed, bool isTypedef, bool isFriend = false)
	{
		if(isFriend)
		{
			// TODO
			return &gFriend;
		}
		{
			Declaration* declaration = findDeclaration(parent->declarations, name);
			if(declaration != 0)
			{
				if(type == &gNamespace)
				{
					if(declaration->type != &gNamespace)
					{
						// name already declared as non-namespace
						printPosition(name.position);
						throw SemanticError();
					}
					// namespace-continuation
					return declaration;
				}
				if(type == &gClassFwd // is a forward-declaration
					&& declaration->type == &gClass) // already class-declaration
				{
					// forward-declaration after class-declaration
					return declaration;
				}
				if(type != &gClass // is not a class-declaration
					&& declaration->type == &gClassFwd) // already forward-declared
				{
					// name already declared as class
					throw SemanticError();
				}
				if(isTypedef || declaration->isTypedef)
				{
					// 7.1.3-4: In a given scope, a typedef specifier shall not be used to redefine the name of any type declared in that scope to refer to a different type.
					if(getBaseType(declaration) != getBaseType(type))
					{
						// name already declared with different type
						printPosition(name.position);
						throw SemanticError();
					}
				}
				else if(type == &gEnum)
				{
					// name already declared
					printPosition(name.position);
					throw SemanticError();
				}
				else if(type == &gBuiltin // is a built-in-type
					|| type->type != 0 // is a user-type
					|| type == &gCtor) // is a constructor/destructor
				{
					if(enclosed != 0) // is a function-declaration (or function-definition)
					{
						if(declaration->enclosed == 0)
						{
							// name already declared
							printPosition(name.position);
							throw SemanticError();
						}
						if(declaration->enclosed == 0)
						{
							// name already declared
							printPosition(name.position);
							throw SemanticError();
						}
					}
					else
					{
						// name already declared
						printPosition(name.position);
						throw SemanticError();
					}
				}
			}
		}
		parent->declarations.push_front(Declaration(parent, name, type, enclosed, isTypedef, isFriend));
		if(enclosed != 0)
		{
			enclosed->name = name;
		}
		Declaration* result = &parent->declarations.front();
		printName(type, result);
		return result;
	}

	void pushScope(Scope* scope)
	{
		scope->parent = context.scope;
		context.scope = scope;
	}

	Scope* popScope()
	{
		if(context.scope->parent == 0)
		{
			throw SemanticError();
		}
		Scope* result = context.scope;
		context.scope = context.scope->parent;
		return result;
	}

	void checkName(const Identifier& id)
	{
		printer.out << "/* lookup: ";
		Declaration* declaration = findDeclaration(*context.scope, id);
		if(declaration == 0)
		{
			printer.out << "failed: ";
		}
		printer.out  << id.value << " */";
	}

	Scope* getEltScope()
	{
		Scope* scope = context.scope;
		for(; !scope->isElt; scope = scope->parent)
		{
		}
		return scope;
	}

	Scope* getNamespaceScope()
	{
		Scope* scope = context.scope;
		for(; !scope->isElt; scope = scope->parent)
		{
		}
		return scope;
	}

};

#define SEMANTIC_ASSERT(condition) if(!(condition)) { throw SemanticError(); }

bool isForwardDeclaration(cpp::elaborated_type_specifier_default* symbol)
{
	return symbol != 0 && symbol->isGlobal.value == 0 && symbol->context.p == 0;
}

bool isForwardDeclaration(cpp::decl_specifier_seq* symbol)
{
	return symbol != 0 && isForwardDeclaration(dynamic_cast<cpp::elaborated_type_specifier_default*>(symbol->type.p));
}


struct Walker
{

struct NestedNameSpecifierWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Scope* scope;
	NestedNameSpecifierWalker(const WalkerBase& base)
		: WalkerBase(base), scope(context.scope)
	{
	}
	NestedNameSpecifierWalker(const WalkerBase& base, Scope* scope)
		: WalkerBase(base), scope(scope)
	{
	}
	void visit(cpp::identifier* symbol)
	{
		Declaration* declaration = findDeclaration(*scope, symbol->value);
		if(declaration->enclosed == 0)
		{
			printPosition(symbol->value.position);
			std::cout << "'" << declaration->name.value << "' is incomplete, declared here:" << std::endl;
			printPosition(declaration->name.position);
			throw SemanticError();
		}
		scope = declaration->enclosed;
		printSymbol(symbol);
	}
	void visit(cpp::simple_template_id* symbol)
	{
		Declaration* declaration = findDeclaration(*scope, symbol->id->value);
		if(declaration->enclosed == 0)
		{
			printPosition(symbol->id->value.position);
			std::cout << "'" << declaration->name.value << "' is incomplete, declared here:" << std::endl;
			printPosition(declaration->name.position);
			throw SemanticError();
		}
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
		: WalkerBase(base), declaration(0), scope(context.scope)
	{
	}

	void visit(cpp::identifier* symbol)
	{
		declaration = findDeclaration(*scope, symbol->value);
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
		declaration = findDeclaration(*scope, symbol->id->value);
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
		: WalkerBase(base), id(makeIdentifier("$anonymous")), scope(context.scope)
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
		id = symbol->value;
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
		id = symbol->id->value;
		printSymbol(symbol);
	}
	void visit(cpp::operator_function_id* symbol) 
	{
		// TODO
		id = makeIdentifier("operator <op>");
		printSymbol(symbol);
	}
	void visit(cpp::conversion_function_id* symbol) 
	{
		// TODO
		id = makeIdentifier("operator T");
		printSymbol(symbol);
	}
	void visit(cpp::destructor_id* symbol) 
	{
		// TODO
		id = makeIdentifier("~T");
		printSymbol(symbol);
	}
	void visit(cpp::template_id_operator_function* symbol) 
	{
		// TODO
		id = makeIdentifier("operator () <>");
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
		: WalkerBase(base), id(makeIdentifier("$undefined")), scope(0), paramScope(0)
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
		: WalkerBase(base), declaration(&gAnonymous), scope(context.scope)
	{
	}

	void visit(cpp::identifier* symbol)
	{
		printSymbol(symbol);
		declaration = pointOfDeclaration(scope, symbol->value, &gClass, 0, false); // 3.3.1.3 The point of declaration for a class first declared by a class-specifier is immediately after the identifier or simple-template-id (if any) in its class-head
	}
	void visit(cpp::nested_name_specifier* symbol)
	{
		NestedNameSpecifierWalker walker(*this);
		symbol->accept(walker);
		scope = walker.scope;
	}
	void visit(cpp::simple_template_id* symbol) 
	{
		printSymbol(symbol);
		declaration = pointOfDeclaration(scope, symbol->id->value, &gClass, 0, false); // 3.3.1.3 The point of declaration for a class first declared by a class-specifier is immediately after the identifier or simple-template-id (if any) in its class-head
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
			printer.printToken(boost::wave::T_IDENTIFIER, declaration->name.value);
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
	bool isFriend;
	DeclSpecifierSeqWalker(const WalkerBase& base)
		: WalkerBase(base), declaration(0), isTypedef(false), isFriend(false)
	{
	}

	void visit(cpp::simple_type_specifier* symbol)
	{
		TypeSpecifierWalker walker(*this);
		symbol->accept(walker);
		declaration = walker.declaration;
	}
	void visit(cpp::elaborated_type_specifier_template* symbol)
	{
		TypeSpecifierWalker walker(*this);
		symbol->accept(walker);
		declaration = walker.declaration;
	}
	void visit(cpp::elaborated_type_specifier_default* symbol)
	{
		if(isForwardDeclaration(symbol))
		{
			printSymbol(symbol);
			// 3.3.1.6: elaborated-type-specifier that is not a block-declaration is declared in smallest enclosing non-class non-function-prototype scope
			declaration = pointOfDeclaration(getEltScope(), symbol->id->value, &gClass, 0, false);
		}
		else
		{
			TypeSpecifierWalker walker(*this);
			symbol->accept(walker);
			declaration = walker.declaration;
		}
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
		Identifier id = symbol->id.p == 0 ? makeIdentifier("$anonymous") : symbol->id->value;
		declaration = pointOfDeclaration(context.scope, id, &gEnum, 0, false);
		printSymbol(symbol);
	}
	void visit(cpp::decl_specifier_default* symbol)
	{
		if(symbol->id == cpp::decl_specifier_default::TYPEDEF)
		{
			isTypedef = true;
		}
		else if(symbol->id == cpp::decl_specifier_default::FRIEND)
		{
			isFriend = true;
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
		pushScope(new Scope(makeIdentifier("local"), true)); // local scope
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
	bool isFriend;
	Scope* paramScope;
	FunctionDefinitions* deferred;

	DeclarationWalker(const WalkerBase& base, FunctionDefinitions* deferred = 0)
		: WalkerBase(base), type(&gCtor), isTypedef(false), isFriend(false), paramScope(0), deferred(deferred)
	{
	}

	void visit(cpp::decl_specifier_seq* symbol)
	{
		DeclSpecifierSeqWalker walker(*this);
		symbol->accept(walker);
		type = walker.declaration;
		isTypedef = walker.isTypedef;
		isFriend = walker.isFriend;
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
		pointOfDeclaration(walker.scope, walker.id, type, isTypedef ? type->enclosed : walker.paramScope, isTypedef, isFriend); // 3.3.1.1
		paramScope = walker.paramScope;
	}
	void visit(cpp::abstract_declarator* symbol)
	{
		DeclaratorWalker walker(*this);
		symbol->accept(walker);
	}
	void visit(cpp::member_declarator_bitfield* symbol)
	{
		printSymbol(symbol);
		pointOfDeclaration(context.scope, symbol->id->value, type, 0, isTypedef, isFriend); // 3.3.1.1
	}

	void visit(cpp::initializer* symbol)
	{
		// TODO
		printSymbol(symbol);
	}
	void visit(cpp::function_definition_suffix* symbol)
	{
		// TODO: also defer name-lookup for default-arguments and initializers
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

struct ForwardDeclarationWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Declaration* declaration;
	ForwardDeclarationWalker(WalkerBase& base)
		: WalkerBase(base), declaration(0)
	{
	}

	void visit(cpp::elaborated_type_specifier_default* symbol)
	{
		printSymbol(symbol);
		declaration = pointOfDeclaration(context.scope, symbol->id->value, &gClass, 0, false);
	}
};

struct TemplateDeclarationWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	TemplateDeclarationWalker(WalkerBase& base)
		: WalkerBase(base)
	{
	}
	void visit(cpp::type_parameter_default* symbol)
	{
		pointOfDeclaration(context.scope, symbol->id->value, &gClass, 0, true);
	}
	void visit(cpp::type_parameter_template* symbol)
	{
		// TODO
	}
	void visit(cpp::parameter_declaration* symbol)
	{
		DeclarationWalker walker(*this);
		symbol->accept(walker);
	}
	void visit(cpp::general_declaration* symbol)
	{
		if(typeid(*symbol->affix.p) == typeid(cpp::general_declaration_type)
			&& isForwardDeclaration(symbol->spec))
		{
			ForwardDeclarationWalker walker(*this);
			symbol->accept(walker);
			SEMANTIC_ASSERT(walker.declaration != 0);
		}
		else
		{
			DeclarationWalker walker(*this);
			symbol->accept(walker);
			SEMANTIC_ASSERT(walker.type != 0);
		}
	}
	// occurs in for-init-statement
	void visit(cpp::simple_declaration* symbol)
	{
		if(typeid(*symbol->affix.p) == typeid(cpp::general_declaration_type)
			&& isForwardDeclaration(symbol->spec))
		{
			ForwardDeclarationWalker walker(*this);
			symbol->accept(walker);
			SEMANTIC_ASSERT(walker.declaration != 0);
		}
		else
		{
			DeclarationWalker walker(*this);
			symbol->accept(walker);
			SEMANTIC_ASSERT(walker.type != 0);
		}
	}
};

struct NamespaceWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;


	NamespaceWalker(WalkerContext& context)
		: WalkerBase(context)
	{
	}

	void visit(cpp::namespace_definition* symbol)
	{
		Identifier id = symbol->id.p == 0 ? makeIdentifier("$anonymous") : symbol->id->value;
		Scope* scope = new Scope(id, true);
		pointOfDeclaration(context.scope, id, &gNamespace, scope, false);
		pushScope(scope);
		symbol->accept(*this);
		popScope();
	}
	void visit(cpp::general_declaration* symbol)
	{
		TemplateDeclarationWalker walker(*this);
		walker.visit(symbol);
	}
	// occurs in for-init-statement
	void visit(cpp::simple_declaration* symbol)
	{
		TemplateDeclarationWalker walker(*this);
		walker.visit(symbol);
	}
	void visit(cpp::template_declaration* symbol)
	{
		// TODO
		pushScope(new Scope(makeIdentifier("$template")));
		TemplateDeclarationWalker walker(*this);
		symbol->accept(walker);
		popScope();
	}
	void visit(cpp::selection_statement* symbol)
	{
		pushScope(new Scope(makeIdentifier("selection"), true));
		symbol->accept(*this);
		popScope();
	}
	void visit(cpp::iteration_statement* symbol)
	{
		pushScope(new Scope(makeIdentifier("iteration"), true));
		symbol->accept(*this);
		popScope();
	}
	void visit(cpp::condition_init* symbol)
	{
		DeclarationWalker walker(*this);
		symbol->accept(walker);
		SEMANTIC_ASSERT(walker.type != 0);
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
