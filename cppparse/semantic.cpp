
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

struct DeclSpecifiers
{
	bool isTypedef;
	bool isFriend;
	bool isStatic;
	bool isExtern;
	DeclSpecifiers()
		: isTypedef(false), isFriend(false), isStatic(false), isExtern(false)
	{
	}
	DeclSpecifiers(bool isTypedef, bool isFriend, bool isStatic, bool isExtern)
		: isTypedef(isTypedef), isFriend(isFriend), isStatic(isStatic), isExtern(isExtern)
	{
	}
};

const DeclSpecifiers DECLSPEC_TYPEDEF = DeclSpecifiers(true, false, false, false);

struct Declaration
{
	Scope* scope;
	Identifier name;
	Declaration* type;
	Scope* enclosed;
	DeclSpecifiers specifiers;

	Declaration(Scope* scope, Identifier name, Declaration* type, Scope* enclosed, DeclSpecifiers specifiers = DeclSpecifiers())
		: scope(scope), name(name), type(type), enclosed(enclosed), specifiers(specifiers)
	{
	}
};

enum ScopeType
{
	SCOPETYPE_NAMESPACE,
	SCOPETYPE_PROTOTYPE,
	SCOPETYPE_LOCAL,
	SCOPETYPE_CLASS,
	SCOPETYPE_TEMPLATE,
};

struct Scope
{
	Scope* parent;
	Identifier name;
	typedef std::list<Declaration> Declarations;
	Declarations declarations;
	ScopeType type;

	Scope(Identifier name, ScopeType type)
		: parent(0), name(name), type(type)
	{
	}
};

bool enclosesElt(ScopeType type)
{
	return type == SCOPETYPE_NAMESPACE
		|| type == SCOPETYPE_LOCAL;
}

Scope global(makeIdentifier("$global"), SCOPETYPE_NAMESPACE);

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


// special-case
Declaration gUndeclared(&global, makeIdentifier("$undeclared"), 0, &global);
Declaration gAnonymous(&global, makeIdentifier("$anonymous"), 0, &global);
Declaration gFriend(&global, makeIdentifier("$friend"), 0, &global);

// symbol types
Declaration gNamespace(&global, makeIdentifier("$namespace"), 0, 0);
Declaration gBuiltin(&global, makeIdentifier("$builtin"), 0, 0);
Declaration gTypename(&global, makeIdentifier("$typename"), 0, 0);
Declaration gClassFwd(&global, makeIdentifier("$classfwd"), 0, 0);
Declaration gClass(&global, makeIdentifier("$class"), 0, 0);
Declaration gEnum(&global, makeIdentifier("$enum"), 0, 0);
Declaration gCtor(&global, makeIdentifier("$ctor"), 0, 0);


struct WalkerBase : public PrintingWalker
{
	WalkerContext& context;
	Scope* enclosing;
	Scope* templateParams;

	WalkerBase(WalkerContext& context)
		: PrintingWalker(context.printer), context(context), enclosing(0), templateParams(0)
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
		return 0;
	}

	Declaration* findDeclaration(const Identifier& id)
	{
		{
			Declaration* result = findDeclaration(*enclosing, id);
			if(result != 0)
			{
				return result;
			}
		}
		if(templateParams != 0)
		{
			Declaration* result = findDeclaration(*templateParams, id);
			if(result != 0)
			{
				return result;
			}
		}
		printPosition(id.position);
		std::cout << "'" << id.value << "' was not declared" << std::endl;
		printer.out << "/* undeclared: " << id.value << " */";
		return &gUndeclared;
	}

	void printName(Scope* scope)
	{
		if(scope->parent != 0)
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

	const Declaration* getBaseType(const Declaration* type)
	{
		while(type->specifiers.isTypedef)
		{
			type = type->type;
		}
		return type;
	}

	bool isTyped(const Declaration& declaration)
	{
		return declaration.type == &gBuiltin
			|| declaration.type->type != 0;
	}

	bool isFunctionDefinition(const Declaration& declaration)
	{
		return declaration.enclosed != 0;
	}

	bool isStaticMember(const Declaration& declaration)
	{
		return declaration.scope->type == SCOPETYPE_CLASS
			&& declaration.specifiers.isStatic
			&& !isFunctionDefinition(declaration);
	}

	bool isTypedefDeclaration(const Declaration& declaration)
	{
		return declaration.specifiers.isTypedef;
	}

	bool isExternDeclaration(const Declaration& declaration)
	{
		return declaration.specifiers.isExtern;
	}

	bool isDefinition(const Declaration& declaration)
	{
		return declaration.type == &gClass // class A {};
			|| declaration.type == &gEnum // enum E {};
			|| (isTyped(declaration) // int i; void f();
				&& !isTypedefDeclaration(declaration) // typedef int I;
				&& !isStaticMember(declaration) // struct S { static int i };
				&& !isExternDeclaration(declaration) // extern int i;
				&& !isFunctionDefinition(declaration)); // TODO: function overloading
	}

	bool isRedeclaration(const Declaration& declaration, const Declaration& other)
	{
		return getBaseType(&declaration)->type == getBaseType(&other)->type;
	}

	Declaration* pointOfDeclaration(Scope* parent, const Identifier& name, Declaration* type, Scope* enclosed, DeclSpecifiers specifiers = DeclSpecifiers())
	{
		if(specifiers.isFriend)
		{
			// TODO
			return &gFriend;
		}
			
		Declaration other(parent, name, type, enclosed, specifiers);
		{
			Declaration* declaration = findDeclaration(parent->declarations, name);
			if(declaration != 0)
			{
#if 0
				if(isDefinition(*declaration)
					&& isDefinition(other))
				{
					printPosition(name.position);
					std::cout << "'" << name.value << "' already defined here:" << std::endl;
					printPosition(declaration->name.position);
					throw SemanticError();
				}
				if(!isRedeclaration(*declaration, other))
				{
					printPosition(name.position);
					std::cout << "'" << name.value << "' already declared here:" << std::endl;
					printPosition(declaration->name.position);
					throw SemanticError();
				}
#else
				if(type == &gNamespace)
				{
					if(declaration->type != &gNamespace)
					{
						// name already declared as non-namespace
						printPosition(name.position);
						std::cout << "'" << name.value << "' already declared here:" << std::endl;
						printPosition(declaration->name.position);
						throw SemanticError();
					}
					// namespace-continuation
					return declaration;
				}
				if(type == &gClassFwd // is a forward-declaration
					&& getBaseType(declaration)->type == &gClass) // already class-declaration
				{
					// forward-declaration after class-declaration
					return declaration;
				}
				if(type != &gClass // is not a class-declaration
					&& type != &gCtor // is not a constructor-declaration
					&& declaration->type == &gClassFwd) // already forward-declared
				{
					// name already declared as class
					printPosition(name.position);
					std::cout << "'" << name.value << "' already declared here:" << std::endl;
					printPosition(declaration->name.position);
					throw SemanticError();
				}
				if(specifiers.isTypedef || declaration->specifiers.isTypedef)
				{
					// 7.1.3-4: In a given scope, a typedef specifier shall not be used to redefine the name of any type declared in that scope to refer to a different type.
					if(getBaseType(declaration) != getBaseType(type))
					{
						// name already declared with different type
						printPosition(name.position);
						std::cout << "'" << name.value << "' already declared here:" << std::endl;
						printPosition(declaration->name.position);
						throw SemanticError();
					}
				}
				else if(type == &gEnum)
				{
					// name already declared
					printPosition(name.position);
					std::cout << "'" << name.value << "' already declared here:" << std::endl;
					printPosition(declaration->name.position);
					throw SemanticError();
				}
				else if(type == &gCtor)
				{
					// TODO
				}
				else if(type == &gBuiltin // is a built-in-type
					|| type->type != 0) // is a user-type
				{
					if(enclosed != 0) // is a function-declaration (or function-definition)
					{
						if(declaration->enclosed == 0)
						{
							// name already declared
							printPosition(name.position);
							std::cout << "'" << name.value << "' already declared here:" << std::endl;
							printPosition(declaration->name.position);
							throw SemanticError();
						}
					}
					else if(!(parent->type == SCOPETYPE_CLASS
						&& declaration->specifiers.isStatic))
					{
						// name already declared
						printPosition(name.position);
						std::cout << "'" << name.value << "' already declared here:" << std::endl;
						printPosition(declaration->name.position);
						throw SemanticError();
					}
				}
#endif
			}
		}
		parent->declarations.push_front(other);
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
		scope->parent = enclosing;
		enclosing = scope;
	}

	void pushTemplateParams(Scope* other)
	{
		other->parent = templateParams;
		templateParams = other;
	}

	Scope* getEltScope()
	{
		Scope* scope = enclosing;
		for(; !enclosesElt(scope->type); scope = scope->parent)
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

	NestedNameSpecifierWalker(const WalkerBase& base)
		: WalkerBase(base)
	{
	}
	void visit(cpp::identifier* symbol)
	{
		Declaration* declaration = findDeclaration(symbol->value);
		if(declaration->enclosed == 0)
		{
			printPosition(symbol->value.position);
			std::cout << "'" << declaration->name.value << "' is incomplete, declared here:" << std::endl;
			printPosition(declaration->name.position);
			throw SemanticError();
		}
		pushScope(declaration->enclosed);
		printSymbol(symbol);
	}
	void visit(cpp::simple_template_id* symbol)
	{
		Declaration* declaration = findDeclaration(symbol->id->value);
		if(declaration->enclosed == 0)
		{
			printPosition(symbol->id->value.position);
			std::cout << "'" << declaration->name.value << "' is incomplete, declared here:" << std::endl;
			printPosition(declaration->name.position);
			throw SemanticError();
		}
		pushScope(declaration->enclosed);
		printSymbol(symbol);
	}
};

struct TypeSpecifierWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Declaration* declaration;
	TypeSpecifierWalker(const WalkerBase& base)
		: WalkerBase(base), declaration(0)
	{
	}

	void visit(cpp::identifier* symbol)
	{
		declaration = findDeclaration(symbol->value);
		printSymbol(symbol);
	}
	void visit(cpp::simple_type_specifier_name* symbol)
	{
		if(symbol->isGlobal.value != 0)
		{
			enclosing = &global;
		}
		symbol->accept(*this);
	}
	void visit(cpp::simple_type_specifier_template* symbol)
	{
		if(symbol->isGlobal.value != 0)
		{
			enclosing = &global;
		}
		symbol->accept(*this);
	}
	void visit(cpp::nested_name_specifier* symbol)
	{
		NestedNameSpecifierWalker walker(*this);
		symbol->accept(walker);
		enclosing = walker.enclosing;
	}
	void visit(cpp::simple_template_id* symbol) 
	{
		declaration = findDeclaration(symbol->id->value);
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
	DeclaratorIdWalker(const WalkerBase& base)
		: WalkerBase(base), id(makeIdentifier("$anonymous"))
	{
	}

	void visit(cpp::qualified_id_global* symbol) 
	{
		enclosing = &global;
		symbol->accept(*this);
	}
	void visit(cpp::qualified_id_default* symbol) 
	{
		if(symbol->isGlobal.value != 0)
		{
			enclosing = &global;
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
		NestedNameSpecifierWalker walker(*this);
		symbol->accept(walker);
		enclosing = walker.enclosing;
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

struct ParameterDeclarationClauseWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	ParameterDeclarationClauseWalker(const WalkerBase& base)
		: WalkerBase(base)
	{
		pushScope(new Scope(makeIdentifier("$prototype"), SCOPETYPE_PROTOTYPE)); // parameter scope
	}

	void visit(cpp::parameter_declaration* symbol)
	{
		SimpleDeclarationWalker walker(*this);
		symbol->accept(walker);
	}
};

struct DeclaratorWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Identifier id;
	Scope* paramScope;
	DeclaratorWalker(const WalkerBase& base)
		: WalkerBase(base), id(makeIdentifier("$undefined")), paramScope(0)
	{
	}

	void visit(cpp::declarator_id* symbol)
	{
		DeclaratorIdWalker walker(*this);
		symbol->accept(walker);
		id = walker.id;
		enclosing = walker.enclosing;
	}
	void visit(cpp::parameter_declaration_clause* symbol)
	{
		ParameterDeclarationClauseWalker walker(*this);
		symbol->accept(walker);
		paramScope = walker.enclosing; // store reference for later resumption
	}
	void visit(cpp::parameter_declaration* symbol)
	{
		SimpleDeclarationWalker walker(*this);
		symbol->accept(walker);
	}
	void visit(cpp::exception_specification* symbol)
	{
		printSymbol(symbol);
	}
};

struct ClassHeadWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Declaration* declaration;
	ClassHeadWalker(const WalkerBase& base)
		: WalkerBase(base), declaration(&gAnonymous)
	{
	}

	void visit(cpp::identifier* symbol)
	{
		printSymbol(symbol);
		declaration = pointOfDeclaration(enclosing, symbol->value, &gClass, 0); // 3.3.1.3 The point of declaration for a class first declared by a class-specifier is immediately after the identifier or simple-template-id (if any) in its class-head
	}
	void visit(cpp::nested_name_specifier* symbol)
	{
		NestedNameSpecifierWalker walker(*this);
		symbol->accept(walker);
		enclosing = walker.enclosing;
	}
	void visit(cpp::simple_template_id* symbol) 
	{
		printSymbol(symbol);
		declaration = pointOfDeclaration(enclosing, symbol->id->value, &gClass, 0); // 3.3.1.3 The point of declaration for a class first declared by a class-specifier is immediately after the identifier or simple-template-id (if any) in its class-head
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

struct MemberDeclarationWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	FunctionDefinitions* deferred;
	MemberDeclarationWalker(const WalkerBase& base, FunctionDefinitions* deferred)
		: WalkerBase(base), deferred(deferred)
	{
	}
	void visit(cpp::member_template_declaration* symbol)
	{
		TemplateDeclarationWalker walker(*this, deferred);
		symbol->accept(walker);
	}
	void visit(cpp::member_declaration_implicit* symbol)
	{
		SimpleDeclarationWalker walker(*this, deferred);
		symbol->accept(walker);
	}
	void visit(cpp::member_declaration_default* symbol)
	{
		if(typeid(*symbol->decl.p) == typeid(cpp::general_declaration_type)
			&& isForwardDeclaration(symbol->spec))
		{
			ForwardDeclarationWalker walker(*this);
			symbol->accept(walker);
			SEMANTIC_ASSERT(walker.declaration != 0);
		}
		else
		{
			SimpleDeclarationWalker walker(*this, deferred);
			symbol->accept(walker);
			SEMANTIC_ASSERT(walker.type != 0);
		}
	}
	void visit(cpp::member_declaration_nested* symbol)
	{
		SimpleDeclarationWalker walker(*this, deferred);
		symbol->accept(walker);
	}
};


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
		Scope* scope = new Scope(declaration->name, SCOPETYPE_CLASS);
		declaration->enclosed = scope;
		pushScope(scope); // 3.3.6.1.1 // class scope
	}
	void visit(cpp::member_declaration* symbol)
	{
		MemberDeclarationWalker walker(*this, &deferred);
		symbol->accept(walker);
	}
};

struct DeclSpecifierSeqWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Declaration* declaration;
	DeclSpecifiers specifiers;
	DeclSpecifierSeqWalker(const WalkerBase& base)
		: WalkerBase(base), declaration(0)
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
			declaration = pointOfDeclaration(getEltScope(), symbol->id->value, &gClassFwd, 0);
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
		printSymbol(symbol);
		declaration = &gTypename;
	}
	void visit(cpp::class_specifier* symbol)
	{
		ClassSpecifierWalker walker(*this);
		symbol->accept(walker);
		declaration = walker.declaration;
		walker.walkDeferred();
	}
	void visit(cpp::enum_specifier* symbol)
	{
		// TODO 
		// + anonymous enums
		if(symbol->id.p != 0)
		{
			Identifier id = symbol->id->value;
			declaration = pointOfDeclaration(enclosing, id, &gEnum, 0);
		}
		else
		{
			declaration = &gAnonymous;
		}
		printSymbol(symbol);
	}
	void visit(cpp::decl_specifier_default* symbol)
	{
		if(symbol->id == cpp::decl_specifier_default::TYPEDEF)
		{
			specifiers.isTypedef = true;
		}
		else if(symbol->id == cpp::decl_specifier_default::FRIEND)
		{
			specifiers.isFriend = true;
		}
		printSymbol(symbol);
	}
	void visit(cpp::storage_class_specifier* symbol)
	{
		if(symbol->id == cpp::storage_class_specifier::STATIC)
		{
			specifiers.isStatic = true;
		}
		else if(symbol->id == cpp::storage_class_specifier::EXTERN)
		{
			specifiers.isExtern = true;
		}
		printSymbol(symbol);
	}
};

struct StatementWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	StatementWalker(const WalkerBase& base)
		: WalkerBase(base)
	{
	}
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
			SimpleDeclarationWalker walker(*this);
			symbol->accept(walker);
			SEMANTIC_ASSERT(walker.type != 0);
		}
	}
	void visit(cpp::selection_statement* symbol)
	{
		ControlStatementWalker walker(*this);
		symbol->accept(walker);
	}
	void visit(cpp::iteration_statement* symbol)
	{
		ControlStatementWalker walker(*this);
		symbol->accept(walker);
	}
};

struct ControlStatementWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	ControlStatementWalker(const WalkerBase& base)
		: WalkerBase(base)
	{
		pushScope(new Scope(makeIdentifier("$control"), SCOPETYPE_LOCAL));
	}
	void visit(cpp::condition_init* symbol)
	{
		SimpleDeclarationWalker walker(*this);
		symbol->accept(walker);
		SEMANTIC_ASSERT(walker.type != 0);
	}
	void visit(cpp::statement* symbol)
	{
		StatementWalker walker(*this);
		symbol->accept(walker);
	}
};

struct CompoundStatementWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	CompoundStatementWalker(const WalkerBase& base)
		: WalkerBase(base)
	{
		pushScope(new Scope(makeIdentifier("local"), SCOPETYPE_LOCAL)); // local scope
	}

	void visit(cpp::statement* symbol)
	{
		StatementWalker walker(*this);
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
		pushScope(paramScope); // 3.3.2.1 parameter scope
	}
	void visit(cpp::compound_statement* symbol)
	{
		CompoundStatementWalker walker(*this);
		symbol->accept(walker);
	}
};

struct SimpleDeclarationWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Declaration* type;
	DeclSpecifiers specifiers;
	Scope* paramScope;
	FunctionDefinitions* deferred;

	SimpleDeclarationWalker(const WalkerBase& base, FunctionDefinitions* deferred = 0)
		: WalkerBase(base), type(&gCtor), paramScope(0), deferred(deferred)
	{
	}


	void visit(cpp::decl_specifier_seq* symbol)
	{
		DeclSpecifierSeqWalker walker(*this);
		symbol->accept(walker);
		type = walker.declaration;
		specifiers = walker.specifiers;
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
		pointOfDeclaration(walker.enclosing, walker.id, type, specifiers.isTypedef ? type->enclosed : walker.paramScope, specifiers); // 3.3.1.1
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
		if(symbol->id.p != 0)
		{
			pointOfDeclaration(enclosing, symbol->id->value, type, 0, specifiers); // 3.3.1.1
		}
	}

	void visit(cpp::initializer* symbol)
	{
		// TODO
		printSymbol(symbol);
	}
	void visit(cpp::function_definition_suffix* symbol)
	{
		SEMANTIC_ASSERT(paramScope != 0);
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
		declaration = pointOfDeclaration(enclosing, symbol->id->value, &gClassFwd, 0);
	}
};

struct TemplateDeclarationWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	FunctionDefinitions* deferred;
	TemplateDeclarationWalker(WalkerBase& base, FunctionDefinitions* deferred = 0)
		: WalkerBase(base), deferred(deferred)
	{
	}
	void visit(cpp::type_parameter_default* symbol)
	{
		pointOfDeclaration(enclosing, symbol->id->value, &gClass, 0, DECLSPEC_TYPEDEF);
		printSymbol(symbol);
	}
	void visit(cpp::type_parameter_template* symbol)
	{
		// TODO
		printSymbol(symbol);
	}
	void visit(cpp::parameter_declaration* symbol)
	{
		SimpleDeclarationWalker walker(*this);
		symbol->accept(walker);
	}
	void visit(cpp::template_parameter_list* symbol)
	{
		Scope* params = new Scope(makeIdentifier("$params"), SCOPETYPE_TEMPLATE);
		pushScope(params);
		symbol->accept(*this);
		enclosing = params->parent;
		pushTemplateParams(params);
	}
	void visit(cpp::declaration* symbol)
	{
		DeclarationWalker walker(*this);
		walker.visit(symbol);
	}
	void visit(cpp::member_declaration* symbol)
	{
		MemberDeclarationWalker walker(*this, deferred);
		walker.visit(symbol);
	}
};

struct DeclarationWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	DeclarationWalker(WalkerBase& base)
		: WalkerBase(base)
	{
	}
	void visit(cpp::namespace_definition* symbol)
	{
		Scope* scope = enclosing;
		if(symbol->id.p != 0)
		{
			Identifier id = symbol->id->value;
			Declaration* declaration = pointOfDeclaration(enclosing, id, &gNamespace, 0);
			if(declaration->enclosed == 0)
			{
				declaration->enclosed = new Scope(id, SCOPETYPE_NAMESPACE);
			}
			scope = declaration->enclosed;
		}
		NamespaceWalker walker(*this, scope);
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
			SimpleDeclarationWalker walker(*this);
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
			SimpleDeclarationWalker walker(*this);
			symbol->accept(walker);
			SEMANTIC_ASSERT(walker.type != 0);
		}
	}
	void visit(cpp::template_declaration* symbol)
	{
		TemplateDeclarationWalker walker(*this);
		symbol->accept(walker);
	}
};

struct NamespaceWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;


	NamespaceWalker(WalkerContext& context)
		: WalkerBase(context)
	{
		pushScope(&global);
	}

	NamespaceWalker(WalkerBase& base, Scope* scope)
		: WalkerBase(base)
	{
		pushScope(scope);
	}

	void visit(cpp::declaration* symbol)
	{
		DeclarationWalker walker(*this);
		walker.visit(symbol);
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
