
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

#include <iostream>
#include <list>

struct SemanticError
{
	SemanticError()
	{
	}
};

#define SEMANTIC_ASSERT(condition) if(!(condition)) { throw SemanticError(); }

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
	bool isTemplate;
	bool isTemplateSpecialization;

	Declaration(Scope* scope, Identifier name, Declaration* type, Scope* enclosed, DeclSpecifiers specifiers = DeclSpecifiers(), bool isTemplate = false, bool isTemplateSpecialization = false)
		: scope(scope), name(name), type(type), enclosed(enclosed), specifiers(specifiers), isTemplate(isTemplate), isTemplateSpecialization(isTemplateSpecialization)
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

const Identifier IDENTIFIER_NULL = makeIdentifier(0);

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
Declaration gFriend(&global, makeIdentifier("$friend"), 0, &global);


// meta types
Declaration gSpecial(&global, makeIdentifier("$special"), 0, 0);
Declaration gClass(&global, makeIdentifier("$class"), 0, 0);
Declaration gEnum(&global, makeIdentifier("$enum"), 0, 0);
Declaration gCtor(&global, makeIdentifier("$ctor"), 0, 0);

// types
Declaration gNamespace(&global, makeIdentifier("$namespace"), &gSpecial, 0);
Declaration gTypename(&global, makeIdentifier("$typename"), &gSpecial, 0);
Declaration gBuiltin(&global, makeIdentifier("$builtin"), &gSpecial, 0);
Declaration gParam(&global, makeIdentifier("$param"), &gClass, 0);

Declaration gTemplateParams[] = 
{
	gParam, gParam, gParam, gParam,
	gParam, gParam, gParam, gParam,
	gParam, gParam, gParam, gParam,
	gParam, gParam, gParam, gParam,
	gParam, gParam, gParam, gParam,
	gParam, gParam, gParam, gParam,
	gParam, gParam, gParam, gParam,
};

// int i; // type -> int
// typedef int I; // type -> int
// I i; // type -> I -> int
// typedef I J; // type -> I -> int
// J j; // type -> J -> I -> int
// struct S; // type -> struct
// typedef struct S S; // type -> S -> struct
// typedef struct S {} S; // type -> S -> struct
// typedef enum E {} E; // type -> E -> enum
const Declaration* getType(const Declaration& declaration)
{
	if(declaration.type->type == 0)
	{
		return &declaration;
	}
	if(declaration.specifiers.isTypedef)
	{
		return getType(*declaration.type);
	}
	return declaration.type;
}

const Declaration* getBaseType(const Declaration& declaration)
{
	if(declaration.type->specifiers.isTypedef)
	{
		return getBaseType(*declaration.type);
	}
	return declaration.type;
}

bool isTyped(const Declaration& declaration)
{
	return declaration.type == &gBuiltin
		|| declaration.type->type != 0;
}

bool isFunction(const Declaration& declaration)
{
	return declaration.enclosed != 0;
}

bool isStaticMember(const Declaration& declaration)
{
	return declaration.scope->type == SCOPETYPE_CLASS
		&& declaration.specifiers.isStatic
		&& !isFunction(declaration);
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
	return (declaration.type == &gClass && declaration.enclosed != 0 && !declaration.isTemplateSpecialization) // class A {};
		|| declaration.type == &gEnum // enum E {};
		|| (isTyped(declaration) // int i; void f();
		&& !isTypedefDeclaration(declaration) // typedef int I;
		&& !isStaticMember(declaration) // struct S { static int i };
		&& !isExternDeclaration(declaration) // extern int i;
		&& !isFunction(declaration)); // TODO: function overloading
}

bool isRedeclaration(const Declaration& declaration, const Declaration& other)
{
	if(other.type == &gCtor) // TODO: distinguish constructor names from class name
	{
		return true;
	}
	if(other.isTemplateSpecialization) // TODO: compare template-argument-list
	{
		return true;
	}
	if(isTyped(declaration)
		&& isTyped(other)
		&& isFunction(declaration)
		&& isFunction(other))// TODO: function overloading
	{
		return true;
	}
	if(!isTypedefDeclaration(declaration)
		&& !isTypedefDeclaration(other))
	{
		return getBaseType(declaration) == getBaseType(other);
	}
	return getType(declaration) == getType(other);
}


struct WalkerBase : public PrintingWalker
{
	WalkerContext& context;
	Scope* enclosing;
	Scope* templateParams;
	Scope* templateEnclosing;

	WalkerBase(WalkerContext& context)
		: PrintingWalker(context.printer), context(context), enclosing(0), templateParams(0), templateEnclosing(0)
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

	const char* getValue(const Identifier& id)
	{
		return id.value == 0 ? "$unnamed" : id.value;
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
		std::cout << "'" << getValue(id) << "' was not declared" << std::endl;
		printer.out << "/* undeclared: " << getValue(id) << " */";
		return &gUndeclared;
	}

	void printName(Scope* scope)
	{
		if(scope->parent != 0)
		{
			printName(scope->parent);
			printer.out << getValue(scope->name) << "::";
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
			printer.out << getValue(name->name);
		}
	}

	void printName(const char* caption, Declaration* type, Declaration* name)
	{
		printer.out << "/* ";
		printer.out << caption;
		printName(type);
		printer.out << ": ";
		printName(name);
		printer.out << " */";
	}

	inline void printPosition(const LexFilePosition& position)
	{
		std::cout << position.get_file() << "(" << position.get_line() << "): ";
	}

	Declaration* pointOfDeclaration(Scope* parent, const Identifier& name, Declaration* type, Scope* enclosed, DeclSpecifiers specifiers = DeclSpecifiers(), bool isTemplate = false, bool isTemplateSpecialization = false)
	{
		if(specifiers.isFriend)
		{
			// TODO
			return &gFriend;
		}
		
		if(enclosed != 0)
		{
			enclosed->name = name;
		}
		Declaration other(parent, name, type, enclosed, specifiers, isTemplate, isTemplateSpecialization);
		if(name.value != 0) // unnamed class/struct/union/enum
		{
			Declaration* declaration = findDeclaration(parent->declarations, name);
			if(declaration != 0)
			{
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
				printName("redeclared: ", type, declaration);
				return declaration;
			}
		}
		parent->declarations.push_front(other);
		if(enclosed != 0)
		{
			enclosed->name = name;
		}
		Declaration* result = &parent->declarations.front();
		printName("", type, result);
		return result;
	}

	void pushScope(Scope* scope)
	{
		SEMANTIC_ASSERT(scope != templateParams);
		scope->parent = enclosing;
		enclosing = scope;
	}

	void pushTemplateParams(Scope* scope)
	{
		SEMANTIC_ASSERT(scope != templateParams);
		scope->parent = templateParams;
		templateParams = scope;
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

typedef bool (*IdentifierFunc)(const Declaration& declaration);

bool isTypeName(const Declaration& declaration)
{
	return &declaration == &gBuiltin
		|| declaration.type == &gTypename
		|| declaration.type->type == 0
		|| declaration.specifiers.isTypedef;
}

bool isNamespaceName(const Declaration& declaration)
{
	return declaration.enclosed->type == SCOPETYPE_NAMESPACE;
}

bool isTemplateName(const Declaration& declaration)
{
	return declaration.isTemplate;
}

const char* getIdentifierType(IdentifierFunc func)
{
	if(func == isTypeName)
	{
		return "type-name";
	}
	if(func == isNamespaceName)
	{
		return "namespace-name";
	}
	if(func == isTemplateName)
	{
		return "template-name";
	}
	return "<unknown>";
}

#define SYMBOL_NAME(T) (typeid(T).name() + 12)

template<typename T>
inline T* resolveAmbiguity(WalkerBase& base, cpp::ambiguity<T>* symbol, bool diagnose = false);

struct AmbiguityCheck : public WalkerBase
{
	IdentifierFunc check;
	bool result;
	bool diagnose;
	AmbiguityCheck(WalkerBase& base, IdentifierFunc check, bool diagnose)
		: WalkerBase(base), check(check), result(false), diagnose(diagnose)
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
		if(result)
		{
			return;
		}
		symbol->accept(*this);
	}

	template<typename T>
	void visit(cpp::symbol<T> symbol)
	{
		if(result)
		{
			return;
		}
		if(symbol.p != 0)
		{
			visit(symbol.p);
		}
		if(result
			&& diagnose)
		{
			std::cout << SYMBOL_NAME(T) << std::endl;
		}
	}

	template<typename T>
	void visit(cpp::ambiguity<T>* symbol)
	{
		if(result)
		{
			return;
		}
		resolveAmbiguity(*this, symbol)->accept(*this);
	}

	void visit(cpp::identifier* symbol)
	{
		if(check != 0)
		{
			Declaration* declaration = findDeclaration(symbol->value);
			SEMANTIC_ASSERT(declaration != &gUndeclared);
			if(!check(*declaration))
			{
				result = true;
				if(diagnose)
				{
					printPosition(symbol->value.position);
					std::cout << "'" << getValue(symbol->value) << "': expected " << getIdentifierType(check) << std::endl;
					throw SemanticError();
				}
			}
		}
	}

	void visit(cpp::type_name* symbol)
	{
		if(result)
		{
			return;
		}
		AmbiguityCheck walker(*this, isTypeName, diagnose);
		symbol->accept(walker);
		result = walker.result;
	}
	void visit(cpp::namespace_name* symbol)
	{
		if(result)
		{
			return;
		}
		AmbiguityCheck walker(*this, isNamespaceName, diagnose);
		symbol->accept(walker);
		result = walker.result;
	}
	void visit(cpp::simple_template_id* symbol)
	{
		if(result)
		{
			return;
		}
		AmbiguityCheck walker(*this, isTemplateName, diagnose);
		symbol->accept(walker);
		result = walker.result;
	}
	void visit(cpp::template_argument_list* symbol)
	{
		if(result)
		{
			return;
		}
		AmbiguityCheck walker(*this, 0, diagnose);
		symbol->accept(walker);
		result = walker.result;
	}
};

template<typename T>
inline bool isValid(WalkerBase& base, T* symbol, bool diagnose)
{
	AmbiguityCheck walker(base, 0, diagnose);
	symbol->accept(walker);
	return !walker.result;
}

template<typename T>
inline T* resolveAmbiguity(WalkerBase& base, cpp::ambiguity<T>* symbol, bool diagnose)
{
	if(isValid(base, symbol->first, diagnose))
	{
		return symbol->first;
	}
	if(isValid(base, symbol->second, diagnose))
	{
		return symbol->second;
	}
	std::cout << "first:" << std::endl;
	isValid(base, symbol->first, true);
	std::cout << "second:" << std::endl;
	isValid(base, symbol->second, true);
	throw SemanticError();
}


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
	} \
	template<typename T> \
	void visit(cpp::ambiguity<T>* symbol) \
	{ \
		resolveAmbiguity(*this, symbol)->accept(*this); \
	}

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
			std::cout << "'" << getValue(declaration->name) << "' is incomplete, declared here:" << std::endl;
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
			std::cout << "'" << getValue(declaration->name) << "' is incomplete, declared here:" << std::endl;
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
	Scope* enclosed;
	ClassHeadWalker(const WalkerBase& base, Scope* enclosed)
		: WalkerBase(base), declaration(0), enclosed(enclosed)
	{
	}

	void visit(cpp::identifier* symbol)
	{
		printSymbol(symbol);
		// 3.3.1.3 The point of declaration for a class first declared by a class-specifier is immediately after the identifier or simple-template-id (if any) in its class-head
		declaration = pointOfDeclaration(enclosing, symbol->value, &gClass, enclosed, DeclSpecifiers(), enclosing == templateEnclosing);
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
		// 3.3.1.3 The point of declaration for a class first declared by a class-specifier is immediately after the identifier or simple-template-id (if any) in its class-head
		declaration = pointOfDeclaration(enclosing, symbol->id->value, &gClass, enclosed, DeclSpecifiers(), enclosing == templateEnclosing, true);
		// TODO args
	}
	void visit(cpp::class_head_anonymous* symbol)
	{
		declaration = pointOfDeclaration(enclosing, IDENTIFIER_NULL, &gClass, enclosed);
		symbol->accept(*this);
	}
	void visit(cpp::base_clause* symbol) 
	{
		// TODO
		printSymbol(symbol);
	}
};

struct FunctionDefinition
{
	Scope* first;
	cpp::function_definition_suffix* second;
	Scope* templateParams;
	FunctionDefinition(Scope* first, cpp::function_definition_suffix* second, Scope* templateParams) :
		first(first), second(second), templateParams(templateParams)
	{
	}
};
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
			printer.printToken(boost::wave::T_IDENTIFIER, getValue(declaration->name));
			printer.printToken(boost::wave::T_COLON_COLON, "::");
			printer.printToken(boost::wave::T_IDENTIFIER, "<deferred>");
			printer.printToken(boost::wave::T_LEFTPAREN, "(");
			printer.printToken(boost::wave::T_IDENTIFIER, "<params>");
			printer.printToken(boost::wave::T_RIGHTPAREN, ")");

			Scope* tmp = templateParams;
			templateParams = (*i).templateParams;
			FunctionDefinitionSuffixWalker walker(*this, (*i).first);
			(*i).second->accept(walker);
			templateParams = tmp;
		}
	};

	void visit(cpp::class_head* symbol)
	{
		Scope* scope = new Scope(makeIdentifier("$class"), SCOPETYPE_CLASS);
		ClassHeadWalker walker(*this, scope);
		symbol->accept(walker);
		declaration = walker.declaration;
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
			declaration = pointOfDeclaration(getEltScope(), symbol->id->value, &gClass, 0, DeclSpecifiers(), enclosing == templateEnclosing);
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
		Identifier id = symbol->id.p != 0 ? symbol->id->value : IDENTIFIER_NULL;
		declaration = pointOfDeclaration(enclosing, id, &gEnum, 0);
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
		pointOfDeclaration(walker.enclosing, walker.id, type, specifiers.isTypedef ? type->enclosed : walker.paramScope, specifiers, enclosing == templateEnclosing); // 3.3.1.1
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
			deferred->push_back(FunctionDefinition(paramScope, symbol, templateParams));
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
		declaration = pointOfDeclaration(enclosing, symbol->id->value, &gClass, 0, DeclSpecifiers(), enclosing == templateEnclosing);
	}
};

struct TemplateDeclarationWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	FunctionDefinitions* deferred;
	Declaration* paramType;
	TemplateDeclarationWalker(WalkerBase& base, FunctionDefinitions* deferred = 0)
		: WalkerBase(base), deferred(deferred), paramType(gTemplateParams)
	{
		templateEnclosing = enclosing;
	}
	void visit(cpp::type_parameter_default* symbol)
	{
		printSymbol(symbol);
		if(symbol->id != 0)
		{
			pointOfDeclaration(enclosing, symbol->id->value, paramType++, 0, DECLSPEC_TYPEDEF);
		}
	}
	void visit(cpp::type_parameter_template* symbol)
	{
		printSymbol(symbol);
		// TODO
		if(symbol->id != 0)
		{
			pointOfDeclaration(enclosing, symbol->id->value, paramType++, 0, DECLSPEC_TYPEDEF, true);
		}
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
		symbol->accept(walker);
	}
	void visit(cpp::member_declaration* symbol)
	{
		MemberDeclarationWalker walker(*this, deferred);
		symbol->accept(walker);
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
		NamespaceWalker walker(*this, symbol->id != 0 ? symbol->id->value : IDENTIFIER_NULL);
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

	NamespaceWalker(WalkerBase& base, const Identifier& id)
		: WalkerBase(base)
	{
		if(id.value != 0)
		{
			Declaration* declaration = pointOfDeclaration(enclosing, id, &gNamespace, 0);
			if(declaration->enclosed == 0)
			{
				declaration->enclosed = new Scope(id, SCOPETYPE_NAMESPACE);
			}
			pushScope(declaration->enclosed);
		}
	}

	void visit(cpp::declaration* symbol)
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
