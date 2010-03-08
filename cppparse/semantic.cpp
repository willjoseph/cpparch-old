

#include "semantic.h"

#include "cpptree.h"
#include "printer.h"
#include "profiler.h"

#include "parser.h"

#include <fstream>

#define SYMBOL_NAME(T) (typeid(T).name() + 12)

#define ARRAY_COUNT(array) (sizeof(array) / sizeof(*array))
#define ARRAY_END(array) ((array) + ARRAY_COUNT(array))


#include <iostream>
#include <list>
#include <set>

struct SemanticError
{
	SemanticError()
	{
	}
};

#define SEMANTIC_ASSERT(condition) if(!(condition)) { throw SemanticError(); }

inline void semanticBreak()
{
}


typedef cpp::terminal_identifier Identifier;

inline Identifier makeIdentifier(const char* value)
{
	Identifier result = { value };
	return result;
}

Identifier IDENTIFIER_NULL = makeIdentifier(0);

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

typedef std::vector<struct TemplateArgument> TemplateArguments;

struct Type
{
	Declaration* declaration;
	TemplateArguments arguments;
	Type(Declaration* declaration) : declaration(declaration)
	{
	}
};

struct TemplateArgument
{
	Type type;
	int value;
	TemplateArgument(const Type& type) : type(type), value(0)
	{
	}
	TemplateArgument(int value) : type(0), value(value)
	{
	}
};

#if 0
bool operator<(const TemplateArgument& left, const TemplateArgument& right)
{
	return left.isType < right.isType ||
		!(right.isType < left.isType) && left.declaration < right.declaration;
}
#endif


struct Declaration
{
	Scope* scope;
	Identifier name;
	Type type;
	Scope* enclosed;
	Declaration* overloaded;
	DeclSpecifiers specifiers;
	bool isTemplate;
	bool isTemplateSpecialization;

	Declaration(Scope* scope, Identifier name, const Type& type, Scope* enclosed, DeclSpecifiers specifiers = DeclSpecifiers(), bool isTemplate = false, bool isTemplateSpecialization = false)
		: scope(scope), name(name), type(type), enclosed(enclosed), overloaded(0), specifiers(specifiers), isTemplate(isTemplate), isTemplateSpecialization(isTemplateSpecialization)
	{
	}
};

struct UniqueName
{
	char buffer[10];
	UniqueName(size_t index)
	{
		sprintf(buffer, "$%x", index);
	}
};
typedef std::vector<UniqueName*> UniqueNames;
UniqueNames gUniqueNames;

enum ScopeType
{
	SCOPETYPE_UNKNOWN,
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
	size_t enclosedScopeCount; // number of scopes directly enclosed by this scope
	typedef std::list<Declaration> Declarations;
	Declarations declarations;
	ScopeType type;
	typedef std::vector<Type> Bases;
	Bases bases;
	Declarations templateParams;

	Scope(const Identifier& name, ScopeType type = SCOPETYPE_UNKNOWN)
		: parent(0), name(name), enclosedScopeCount(0), type(type)
	{
	}

	const char* getUniqueName()
	{
		if(enclosedScopeCount == gUniqueNames.size())
		{
			gUniqueNames.push_back(new UniqueName(enclosedScopeCount));
		}
		return gUniqueNames[enclosedScopeCount++]->buffer;
	}
};

bool enclosesElt(ScopeType type)
{
	return type == SCOPETYPE_NAMESPACE
		|| type == SCOPETYPE_LOCAL;
}


const char* getValue(const Identifier& id)
{
	return id.value == 0 ? "$unnamed" : id.value;
}


// special-case
Declaration gUndeclared(0, makeIdentifier("$undeclared"), 0, 0);
Declaration gFriend(0, makeIdentifier("$friend"), 0, 0);


// meta types
Declaration gSpecial(0, makeIdentifier("$special"), 0, 0);
Declaration gClass(0, makeIdentifier("$class"), 0, 0);
Declaration gEnum(0, makeIdentifier("$enum"), 0, 0);

// types
Declaration gNamespace(0, makeIdentifier("$namespace"), 0, 0);

Declaration gCtor(0, makeIdentifier("$ctor"), &gSpecial, 0);
Declaration gTypename(0, makeIdentifier("$typename"), &gSpecial, 0);
Declaration gBuiltin(0, makeIdentifier("$builtin"), &gSpecial, 0);

Declaration gDependent(0, makeIdentifier("$dependent"), &gBuiltin, 0);

Declaration gParam(0, makeIdentifier("$param"), &gClass, 0);

// objects
Identifier gOperatorFunctionId = makeIdentifier("operator <op>");
Identifier gConversionFunctionId = makeIdentifier("operator T");
Identifier gOperatorFunctionTemplateId = makeIdentifier("operator () <>");
// TODO: don't declare if id is anonymous?
Identifier gAnonymousId = makeIdentifier("$anonymous");



bool isType(const Declaration& type)
{
	return type.specifiers.isTypedef
		|| type.type.declaration == &gSpecial
		|| type.type.declaration == &gEnum
		|| type.type.declaration == &gClass;
}

// int i; // type -> int
// typedef int I; // type -> int
// I i; // type -> I -> int
// typedef I J; // type -> I -> int
// J j; // type -> J -> I -> int
// struct S; // type -> struct
// typedef struct S S; // type -> S -> struct
// typedef struct S {} S; // type -> S -> struct
// typedef enum E {} E; // type -> E -> enum

// returns the type of a declaration
// int i; -> built-in
// class A a; -> A
// enum E e; -> E
// typedef int T; T t; -> built-in
const Declaration* getType(const Declaration& declaration)
{
#if 0
	if(declaration.type.declaration->type == 0)
	{
		return &declaration;
	}
#endif
	if(declaration.specifiers.isTypedef)
	{
		return getType(*declaration.type.declaration);
	}
	return declaration.type.declaration;
}

const Declaration* getBaseType(const Declaration& declaration)
{
	if(declaration.type.declaration->specifiers.isTypedef)
	{
		return getBaseType(*declaration.type.declaration);
	}
	return declaration.type.declaration;
}

const Type& getOriginalType(const Type& type)
{
	if(type.declaration->specifiers.isTypedef)
	{
		return getOriginalType(type.declaration->type);
	}
	return type;
}

bool isFunction(const Declaration& declaration)
{
	return declaration.enclosed != 0;
}

bool isMemberObject(const Declaration& declaration)
{
	return declaration.scope->type == SCOPETYPE_CLASS
		&& !isFunction(declaration);
}

bool isStatic(const Declaration& declaration)
{
	return declaration.specifiers.isStatic;
}

bool isStaticMember(const Declaration& declaration)
{
	return isMemberObject(declaration)
		&& isStatic(declaration);
}

bool isTypedef(const Declaration& declaration)
{
	return declaration.specifiers.isTypedef;
}

bool isClass(const Declaration& declaration)
{
	return declaration.type.declaration == &gClass;
}

bool isEnum(const Declaration& declaration)
{
	return declaration.type.declaration == &gEnum;
}

bool isIncomplete(const Declaration& declaration)
{
	return declaration.enclosed == 0;
}

bool isElaboratedType(const Declaration& declaration)
{
	return (isClass(declaration) || isEnum(declaration)) && isIncomplete(declaration);
}

bool isNamespace(const Declaration& declaration)
{
	return declaration.type.declaration == &gNamespace;
}

bool isObject(const Declaration& declaration)
{
	return !isType(declaration)
		&& !isNamespace(declaration);
}

bool isExtern(const Declaration& declaration)
{
	return declaration.specifiers.isExtern;
}

bool isTypeParameter(const Scope& scope, Declaration* declaration)
{
	for(Scope::Declarations::const_iterator i = scope.templateParams.begin(); i != scope.templateParams.end(); ++i)
	{
		if(&(*i) == declaration)
		{
			return true;
		}
	}
	if(scope.parent != 0)
	{
		return isTypeParameter(*scope.parent, declaration);
	}
	return false;
}

bool isDependent(const Scope& enclosing, const Type& type);

bool isDependent(const Scope& enclosing, const Scope::Bases& bases)
{
	for(Scope::Bases::const_iterator i = bases.begin(); i != bases.end(); ++i)
	{
		if(isDependent(enclosing, *i))
		{
			return true;
		}
	}
	return false;
}

bool isDependent(const Scope& enclosing, const Type& type)
{
	const Type& original = getOriginalType(type);
	if(original.declaration == &gTypename)
	{
		return true;
	}
	if(isTypeParameter(enclosing, original.declaration))
	{
		return true;
	}
	for(TemplateArguments::const_iterator i = original.arguments.begin(); i != original.arguments.end(); ++i)
	{
		if((*i).type.declaration != 0
			&& isDependent(enclosing, (*i).type))
		{
			return true;
		}
	}
	Scope* enclosed = original.declaration->enclosed;
	if(enclosed != 0)
	{
		return isDependent(enclosing, enclosed->bases);
	}
	return false;
}

const char* getDeclarationType(const Declaration& declaration)
{
	if(isNamespace(declaration))
	{
		return "namespace";
	}
	if(isType(declaration))
	{
		return declaration.isTemplate ? "template" : "type";
	}
	return "object";
}


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
			TREEWALKER_WALK(*this, symbol);
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
		TREEWALKER_WALK(walker, symbol);
	}

	void printName(Scope* scope)
	{
		if(scope != 0
			&& scope->parent != 0)
		{
			printName(scope->parent);
			printer.out << getValue(scope->name) << ".";
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

};

const char* escapeTerminal(LexTokenId id, const char* value)
{
	switch(id)
	{
	case boost::wave::T_LESS: return "&lt;";
	case boost::wave::T_LESSEQUAL: return "&lt;=";
	case boost::wave::T_SHIFTLEFT: return "&lt;&lt;";
	case boost::wave::T_SHIFTLEFTASSIGN: return "&lt;&lt;=";
	case boost::wave::T_AND_ALT:
	case boost::wave::T_AND: return "&amp;";
	case boost::wave::T_ANDAND_ALT:
	case boost::wave::T_ANDAND: return "&amp;&amp;";
	case boost::wave::T_ANDASSIGN_ALT:
	case boost::wave::T_ANDASSIGN: return "&amp;=";
	default: break;
	}

	return value;
}

template<LexTokenId id> 
const char* escapeTerminal(cpp::terminal<id> symbol)
{
	return escapeTerminal(id, symbol.value);
}


const char* escapeTerminal(cpp::terminal_choice2 symbol)
{
	return escapeTerminal(symbol.id, symbol.value);
}


bool isPrimary(const Identifier& id)
{
	// TODO: optimise
	return id.dec.p != 0 && id.position == id.dec.p->name.position;
}

struct SymbolPrinter : PrintingWalker
{
	std::ofstream out;
	FileTokenPrinter printer;

	SymbolPrinter(const char* path)
		: PrintingWalker(printer),
		out(path),
		printer(out)
	{
		printer.out << "<html>\n"
			"<head>\n"
			"<link rel='stylesheet' type='text/css' href='identifier.css'/>\n"
			"</style>\n"
			"</head>\n"
			"<body>\n"
			"<pre style='color:#000000;background:#ffffff;'>\n";
	}
	~SymbolPrinter()
	{
		printer.out << "</pre>\n"
			"</body>\n"
			"</html>\n";
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
		printer.printToken(symbol.id, escapeTerminal(symbol));
	}

	template<LexTokenId id>
	void visit(cpp::terminal<id> symbol)
	{
		if(symbol.value != 0)
		{
			printer.printToken(id, escapeTerminal(symbol));
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

	void visit(cpp::identifier* symbol)
	{
		printer.formatToken(boost::wave::T_IDENTIFIER);

		const char* type = symbol->value.dec.p != 0 ? getDeclarationType(*symbol->value.dec.p) : "unknown";
		if(isPrimary(symbol->value))
		{
			printer.out << "<a name='";
			printName(symbol->value.dec.p);
			printer.out << "'></a>";
		}
		else
		{
			printer.out << "<a href='#";
			printName(symbol->value.dec.p);
			printer.out << "'>";
		}
		printer.out << "<" << type << ">";
		printer.out << getValue(symbol->value);
		printer.out << "</" << type << ">";
		if(!isPrimary(symbol->value))
		{
			printer.out << "</a>";
		}
	}

	void visit(cpp::simple_type_specifier_builtin* symbol)
	{
		printer.formatToken(symbol->value.id);

		printer.out << "<type>";
		printer.out << symbol->value.value;
		printer.out << "</type>";
	}
	void visit(cpp::primary_expression_builtin* symbol)
	{
		printer.formatToken(symbol->key.ID);

		printer.out << "<object>";
		printer.out << symbol->key.value;
		printer.out << "</object>";
	}
};



struct DeclarationError
{
	const char* description;
	DeclarationError(const char* description) : description(description)
	{
	}
};

inline const Declaration& getPrimaryDeclaration(const Declaration& first, const Declaration& second)
{
	if(isNamespace(first))
	{
		if(!isNamespace(second))
		{
			throw DeclarationError("non-namespace already declared as namespace");
		}
		return first; // namespace continuation
	}
	if(isType(first))
	{
		if(!isType(second))
		{
			throw DeclarationError("non-type already declared as type");
		}
		if(getType(first) != getType(second))
		{
			throw DeclarationError("type already declared as different type");
		}
		if(isTypedef(first))
		{
			return second; // redefinition of typedef, or definition of type previously used in typedef
		}
		if(isTypedef(second))
		{
			return first; // typedef of type previously declared
		}
		if(isClass(first))
		{
			if(second.isTemplateSpecialization)
			{
				return second; // TODO
			}
			if(isIncomplete(second))
			{
				return first; // forward-declaration of previously-defined class
			}
			if(isIncomplete(first))
			{
				return second; // definition of forward-declared class
			}
			throw DeclarationError("class-definition already defined");
		}
		if(isEnum(first))
		{
			throw DeclarationError("enum-definition already defined");
		}
		throw SemanticError(); // should not be reachable
	}
	if(isType(second))
	{
		throw DeclarationError("type already declared as non-type");
	}
	if(isFunction(first)
		|| isFunction(second))// TODO: function overloading
	{
		return second; // multiple declarations allowed
	}
#if 0 // fails when comparing types if type is template-param dependent
	if(getBaseType(first) != getBaseType(second))
	{
		throw DeclarationError("variable already declared with different type");
	}
#endif
	if(isStaticMember(first))
	{
		// TODO: disallow inline definition of static member: class C { static int i; int i; };
		if(!isMemberObject(second))
		{
			throw DeclarationError("non-member-object already declared as static member-object");
		}
		return second; // multiple declarations allowed
	}
	if(isExtern(first)
		|| isExtern(second))
	{
		return first; // multiple declarations allowed
	}
	throw DeclarationError("symbol already defined");
}

struct IdentifierMismatch
{
	Identifier id;
	const char* expected;
	Declaration* declaration;
	IdentifierMismatch()
	{
	}
	IdentifierMismatch(const Identifier& id, Declaration* declaration, const char* expected) :
		id(id), declaration(declaration), expected(expected)
	{
	}
};



void printDeclarations(const Scope::Declarations& declarations)
{
	std::cout << "{ ";
	for(Scope::Declarations::const_iterator i = declarations.begin(); i != declarations.end();)
	{
		std::cout << getValue((*i).name);
		if(++i != declarations.end())
		{
			std::cout << ", ";
		}
	}
	std::cout << " }";
}

void printBases(const Scope::Bases& bases)
{
	std::cout << "{ ";
	for(Scope::Bases::const_iterator i = bases.begin(); i != bases.end();)
	{
		std::cout << getValue((*i).declaration->name) << ": ";
		Scope* scope = (*i).declaration->enclosed;
		if(scope != 0)
		{
			printDeclarations((*i).declaration->enclosed->declarations);
		}
		if(++i != bases.end())
		{
			std::cout << ", ";
		}
	}
	std::cout << " }";
}

void printScope(const Scope& scope)
{
	std::cout << getValue(scope.name) << ": ";
	std::cout << std::endl;
	std::cout << "  declarations: ";
	printDeclarations(scope.declarations);
	std::cout << std::endl;
	std::cout << "  bases: ";
	printBases(scope.bases);
	std::cout << std::endl;
	if(scope.parent != 0)
	{
		printScope(*scope.parent);
	}
}

void printIdentifierMismatch(const IdentifierMismatch& e)
{
	printPosition(e.id.position);
	std::cout << "'" << getValue(e.id) << "' expected " << e.expected << ", " << (e.declaration == &gUndeclared ? "was undeclared" : "was declared here:") << std::endl;
	if(e.declaration != &gUndeclared)
	{
		printPosition(e.declaration->name.position);
		std::cout << std::endl;
	}
}

#if 0
struct TemplateInstantiation
{
	Declaration* declaration;
	TemplateArguments arguments;
	TemplateInstantiation(Declaration* declaration, const TemplateArguments& arguments)
		: declaration(declaration), arguments(arguments)
	{
	}
};

bool operator<(const TemplateInstantiation& left, const TemplateInstantiation& right)
{
	return left.declaration < right.declaration ||
		!(right.declaration < left.declaration) && left.arguments < right.arguments;
}

typedef std::set<TemplateInstantiation> TemplateInstantiations;
#endif

bool isAny(const Declaration& declaration)
{
	return declaration.type.declaration != &gCtor;
}

typedef bool (*LookupFilter)(const Declaration&);


struct WalkerContext
{
	Scope global;
	Scope dependent;
#if 0
	TemplateInstantiations instantiations;
#endif

	WalkerContext() :
		global(makeIdentifier("$global"), SCOPETYPE_NAMESPACE),
		dependent(makeIdentifier("$dependent"), SCOPETYPE_CLASS)
	{
	}
};


typedef std::vector<struct DeferredSymbol> DeferredSymbols;

struct WalkerBase
{
	WalkerContext& context;
	Scope* enclosing;
	Scope* qualifying;
	Scope* templateParams;
	Scope* templateEnclosing;
	bool* ambiguity;
	DeferredSymbols* deferred;

	WalkerBase(WalkerContext& context)
		: context(context), enclosing(0), qualifying(0), templateParams(0), templateEnclosing(0), ambiguity(0), deferred(0)
	{
	}

	Declaration* findDeclaration(Scope::Declarations& declarations, const Identifier& id, LookupFilter filter = isAny)
	{
		for(Scope::Declarations::iterator i = declarations.begin(); i != declarations.end(); ++i)
		{
			if((*i).name.value == id.value
				&& filter(*i))
			{
				return &(*i);
			}
		}
		return 0;
	}

	Declaration* findDeclaration(Scope::Bases& bases, const Identifier& id, LookupFilter filter = isAny)
	{
		for(Scope::Bases::iterator i = bases.begin(); i != bases.end(); ++i)
		{
			Scope* scope = (*i).declaration->enclosed;
			if(scope != 0)
			{
				Declaration* result = findDeclaration(scope->declarations, scope->bases, id, filter);
				if(result != 0)
				{
					return result;
				}
			}
		}
		return 0;
	}

	Declaration* findDeclaration(Scope::Declarations& declarations, Scope::Bases& bases, const Identifier& id, LookupFilter filter = isAny)
	{
		{
			Declaration* result = findDeclaration(declarations, id, filter);
			if(result != 0)
			{
				return result;
			}
		}
		{
			Declaration* result = findDeclaration(bases, id, filter);
			if(result != 0)
			{
				return result;
			}
		}
		return 0;
	}

	Declaration* findDeclaration(Scope& scope, const Identifier& id, LookupFilter filter = isAny)
	{
		Declaration* result = findDeclaration(scope.declarations, scope.bases, id, filter);
		if(result != 0)
		{
			return result;
		}
		if(scope.parent != 0)
		{
			return findDeclaration(*scope.parent, id, filter);
		}
		return 0;
	}

	Declaration* findDeclaration(const Identifier& id, LookupFilter filter = isAny)
	{
#if 1
		if(templateParams != 0)
		{
			Declaration* result = findDeclaration(*templateParams, id, filter);
			if(result != 0)
			{
				return result;
			}
		}
#endif
		if(qualifying != 0)
		{
			Declaration* result = findDeclaration(*qualifying, id, filter);
			if(result != 0)
			{
				return result;
			}
		}
		else
		{
			Declaration* result = findDeclaration(*enclosing, id, filter);
			if(result != 0)
			{
				return result;
			}
		}
		return &gUndeclared;
	}

	Declaration* pointOfDeclaration(Scope* parent, const Identifier& name, const Type& type, Scope* enclosed, DeclSpecifiers specifiers = DeclSpecifiers(), bool isTemplate = false, bool isTemplateSpecialization = false)
	{
		if(ambiguity != 0)
		{
			*ambiguity = true;
		}

		if(specifiers.isFriend)
		{
			// TODO
			return &gFriend;
		}
		
		Declaration other(parent, name, type, enclosed, specifiers, isTemplate, isTemplateSpecialization);
		if(name.value != 0) // unnamed class/struct/union/enum
		{
			/* 3.4.4-1
			An elaborated-type-specifier (7.1.6.3) may be used to refer to a previously declared class-name or enum-name
			even though the name has been hidden by a non-type declaration (3.3.10).
			*/
			Declaration* declaration = findDeclaration(parent->declarations, name);
			if(declaration != 0)
			{
				try
				{
					const Declaration& primary = getPrimaryDeclaration(*declaration, other);
					if(&primary == declaration)
					{
						return declaration;
					}
				}
				catch(DeclarationError& e)
				{
					printPosition(name.position);
					std::cout << "'" << name.value << "': " << e.description << std::endl;
					printPosition(declaration->name.position);
					throw SemanticError();
				}
			}
		}
		parent->declarations.push_front(other);
		Declaration* result = &parent->declarations.front();
		return result;
	}

#if 0
	const TemplateInstantiation& pointOfInstantiation(Declaration* declaration, const TemplateArguments& arguments)
	{
		return *context.instantiations.insert(TemplateInstantiation(declaration, arguments)).first;
	}
#endif

	Scope* findScope(Scope* scope, Scope* other)
	{
		if(scope == 0)
		{
			return 0;
		}
		if(scope == other)
		{
			return scope;
		}
		return findScope(scope->parent, other);
	}

	void pushScope(Scope* scope)
	{
		SEMANTIC_ASSERT(findScope(enclosing, scope) == 0);
		scope->parent = enclosing;
		enclosing = scope;
	}

	void pushTemplateParams(Scope* scope)
	{
		SEMANTIC_ASSERT(findScope(templateParams, scope) == 0);
		scope->parent = templateParams;
		templateParams = scope;
	}

	void setQualifying(Declaration* declaration)
	{
		if(declaration->enclosed == 0)
		{
			// TODO
			//printPosition(symbol->id->value.position);
			std::cout << "'" << getValue(declaration->name) << "' is incomplete, declared here:" << std::endl;
			printPosition(declaration->name.position);
			throw SemanticError();
		}
		qualifying = declaration->enclosed;
	}

	void reportIdentifierMismatch(const Identifier& id, Declaration* declaration, const char* expected)
	{
		if(ambiguity != 0)
		{
			throw IdentifierMismatch(id, declaration, expected);
		}
		printIdentifierMismatch(IdentifierMismatch(id, declaration, expected));
		printScope();
		throw SemanticError();
	}

	Scope* getEltScope()
	{
		Scope* scope = enclosing;
		for(; !enclosesElt(scope->type); scope = scope->parent)
		{
		}
		return scope;
	}

	Scope* getClassScope()
	{
		Scope* scope = enclosing;
		for(; scope->type != SCOPETYPE_CLASS; scope = scope->parent)
		{
		}
		return scope;
	}

	void printScope()
	{
#if 1
		if(templateParams != 0)
		{
			std::cout << "template-params:" << std::endl;
			::printScope(*templateParams);
		}
#endif
		if(qualifying != 0)
		{
			std::cout << "qualifying:" << std::endl;
			::printScope(*qualifying);
		}
		else
		{
			std::cout << "enclosing:" << std::endl;
			::printScope(*enclosing);
		}
	}

	bool isDependent(const Type& type)
	{
		return (templateParams != 0 && ::isDependent(*templateParams, type))
			|| ::isDependent(*enclosing, type);
	}

	bool isDependent(const Scope::Bases& bases)
	{
		return (templateParams != 0 && ::isDependent(*templateParams, bases))
			|| ::isDependent(*enclosing, bases);
	}

	bool isDependentPrimaryExpression(cpp::primary_expression* symbol)
	{
		cpp::identifier* p = dynamic_cast<cpp::identifier*>(symbol);
		if(p != 0) // TODO: operator-function
		{
			Declaration* declaration = findDeclaration(p->value);
			if(declaration == &gUndeclared
				|| (isObject(*declaration) && declaration->scope->type == SCOPETYPE_NAMESPACE))
			{
				return true;
			}
		}
		return false;
	}
};

typedef bool (*IdentifierFunc)(const Declaration& declaration);

bool isTypeName(const Declaration& declaration)
{
	return isType(declaration);
}

bool isNamespaceName(const Declaration& declaration)
{
	return isNamespace(declaration);
}

bool isTemplateName(const Declaration& declaration)
{
	return declaration.isTemplate;
}

bool isNestedName(const Declaration& declaration)
{
	return isTypeName(declaration)
		|| isNamespaceName(declaration);
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

template<typename Walker, typename T>
inline T* walkAmbiguity(Walker& walker, cpp::ambiguity<T>* symbol)
{
	semanticBreak();
	bool ambiguousDeclaration = false;
	Walker tmp(walker);
	IdentifierMismatch first;
	IdentifierMismatch second;
	try
	{
		walker.ambiguity = &ambiguousDeclaration;
		symbol->first->accept(walker);
		walker.ambiguity = 0;
		return symbol->first;
	}
	catch(IdentifierMismatch& e)
	{
		SEMANTIC_ASSERT(!ambiguousDeclaration);
		walker.~Walker();
		new(&walker) Walker(tmp);
		first = e;
	}

	try
	{
		walker.ambiguity = &ambiguousDeclaration;
		symbol->second->accept(walker);
		walker.ambiguity = 0;
		return symbol->second;
	}
	catch(IdentifierMismatch& e)
	{
		SEMANTIC_ASSERT(!ambiguousDeclaration);
		walker.~Walker();
		new(&walker) Walker(tmp);
		second = e;
	}
	if(walker.ambiguity == 0)
	{
		std::cout << "first:" << std::endl;
		printIdentifierMismatch(first);
		walker.printScope();
		std::cout << "second:" << std::endl;
		printIdentifierMismatch(second);
		walker.printScope();
		throw SemanticError();
	}
	throw first;
}
#if 0
#define TREEWALKER_DEFAULT PARSERCONTEXT_DEFAULT
#else
#define TREEWALKER_WALK(walker, symbol) symbol->accept(walker)
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
	} \
	template<typename T> \
	void visit(T* symbol) \
	{ \
		TREEWALKER_WALK(*this, symbol); \
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
		if(walkAmbiguity(*this, symbol) != symbol->first) \
		{ \
			std::swap(symbol->first, symbol->second); \
		} \
	}
#endif

bool isUnqualified(cpp::elaborated_type_specifier_default* symbol)
{
	return symbol != 0
		&& symbol->isGlobal.value == 0
		&& symbol->context.p == 0;
}

bool isForwardDeclaration(cpp::decl_specifier_seq* symbol)
{
	return symbol != 0
		&& symbol->prefix == 0
		&& symbol->suffix == 0
		&& isUnqualified(dynamic_cast<cpp::elaborated_type_specifier_default*>(symbol->type.p));
}


template<typename Walker, typename T>
struct DeferredSymbolThunk
{
	static void thunk(const WalkerBase& context, void* p)
	{
		Walker walker(context);
		T* symbol = static_cast<T*>(p);
		TREEWALKER_WALK(walker, symbol);
	}
};

struct DeferredSymbol
{
	typedef void (*Func)(const WalkerBase&, void*);
	WalkerBase context;
	void* symbol;
	Func func;

	// hack!
	DeferredSymbol& operator=(const DeferredSymbol& other)
	{
		if(&other != this)
		{
			this->~DeferredSymbol();
			new(this) DeferredSymbol(other);
		}
		return *this;
	}
};

template<typename Walker, typename T>
DeferredSymbol makeDeferredSymbol(const Walker& context, T* symbol)
{
	DeferredSymbol result = { context, symbol, DeferredSymbol::Func(DeferredSymbolThunk<Walker, T>::thunk) };
	return result;
}

struct Walker
{

struct NamespaceNameWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Declaration* declaration;
	LookupFilter filter;
	NamespaceNameWalker(const WalkerBase& base, LookupFilter filter = isAny)
		: WalkerBase(base), declaration(0), filter(filter)
	{
	}
	void visit(cpp::identifier* symbol)
	{
		declaration = findDeclaration(symbol->value, filter);
		if(declaration == &gUndeclared
			|| !isNamespaceName(*declaration))
		{
			reportIdentifierMismatch(symbol->value, declaration, "namespace-name");
		}
		else
		{
			symbol->value.dec.p = declaration;
		}
	}
};

struct TypeIdWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Type type;
	TypeIdWalker(const WalkerBase& base)
		: WalkerBase(base), type(0)
	{
	}
	void visit(cpp::type_specifier_seq* symbol)
	{
		DeclSpecifierSeqWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		type = walker.type;
	}
};

struct UncheckedTemplateIdWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Declaration* declaration;
	cpp::identifier* id;
	UncheckedTemplateIdWalker(const WalkerBase& base)
		: WalkerBase(base), declaration(0)
	{
	}
	void visit(cpp::identifier* symbol)
	{
		id = symbol;
		declaration = findDeclaration(symbol->value);
	}
	void visit(cpp::template_argument_list* symbol)
	{
		qualifying = 0;
		// TODO: store args
		TemplateArgumentListWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
	}
};

struct UnqualifiedIdWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Declaration* declaration;
	cpp::identifier* id;
	UnqualifiedIdWalker(const WalkerBase& base)
		: WalkerBase(base), declaration(0), id(0)
	{
	}
	void visit(cpp::identifier* symbol)
	{
		id = symbol;
		declaration = findDeclaration(symbol->value);
	}
	void visit(cpp::simple_template_id* symbol)
	{
		UncheckedTemplateIdWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		id = walker.id;
		declaration = walker.declaration;
	}
	void visit(cpp::destructor_id* symbol)
	{
		// TODO: can destructor-id be dependent?
	}
};

struct QualifiedIdWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Declaration* declaration;
	cpp::identifier* id;
	QualifiedIdWalker(const WalkerBase& base)
		: WalkerBase(base), declaration(0), id(0)
	{
	}

	void visit(cpp::qualified_id_default* symbol)
	{
		if(symbol->isGlobal.value != 0)
		{
			qualifying = &context.global;
		}
		TREEWALKER_WALK(*this, symbol);
	}
	void visit(cpp::nested_name_specifier* symbol)
	{
		NestedNameSpecifierWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		qualifying = walker.qualifying;
	}
	void visit(cpp::unqualified_id* symbol)
	{
		// TODO
		if(qualifying != &context.dependent)
		{
			UnqualifiedIdWalker walker(*this);
			TREEWALKER_WALK(walker, symbol);
			declaration = walker.declaration;
			id = walker.id;
		}
	}
	void visit(cpp::qualified_id_global* symbol)
	{
		qualifying = &context.global;
		UnqualifiedIdWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
		id = walker.id;
	}
};

struct IdExpressionWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	/* 14.6.2.2-3
	An id-expression is type-dependent if it contains:
	— an identifier that was declared with a dependent type,
	— a template-id that is dependent,
	— a conversion-function-id that specifies a dependent type,
	— a nested-name-specifier or a qualified-id that names a member of an unknown specialization
	*/
	Declaration* declaration;
	cpp::identifier* id;
	IdExpressionWalker(const WalkerBase& base)
		: WalkerBase(base), declaration(0), id(0)
	{
	}
	void visit(cpp::qualified_id* symbol)
	{
		// TODO
		QualifiedIdWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
		id = walker.id;
		qualifying = walker.qualifying;
	}
	void visit(cpp::unqualified_id* symbol)
	{
		// TODO
		UnqualifiedIdWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
		id = walker.id;
		qualifying = walker.qualifying;
	}
};

struct ExplicitTypeExpressionWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	bool isTypeDependent;
	ExplicitTypeExpressionWalker(const WalkerBase& base)
		: WalkerBase(base), isTypeDependent(false)
	{
	}
	void visit(cpp::simple_type_specifier* symbol)
	{
		TypeSpecifierWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		isTypeDependent = isDependent(walker.type);
	}
	void visit(cpp::type_id* symbol)
	{
		TypeIdWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		isTypeDependent = isDependent(walker.type);
	}
	void visit(cpp::new_type* symbol)
	{
		TypeIdWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		isTypeDependent = isDependent(walker.type);
	}
	void visit(cpp::assignment_expression* symbol)
	{
		ExpressionWalker walker(*this);
		walker.isTypeDependent = false;
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::cast_expression* symbol)
	{
		ExpressionWalker walker(*this);
		walker.isTypeDependent = false;
		TREEWALKER_WALK(walker, symbol);
	}
};

struct ExpressionWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	/* 14.6.2.2-1
	...an expression is type-dependent if any subexpression is type-dependent.
	*/
	bool isTypeDependent;
	ExpressionWalker(const WalkerBase& base)
		: WalkerBase(base), isTypeDependent(false)
	{
	}
	void visit(cpp::postfix_expression_member* symbol)
	{
		// TODO
	}
	void visit(cpp::primary_expression_builtin* symbol)
	{
		// TODO
		/* 14.6.2.2-2
		'this' is type-dependent if the class type of the enclosing member function is dependent
		*/
		isTypeDependent |= isDependent(getClassScope()->bases);
	}
	void visit(cpp::type_id* symbol)
	{
		// TODO
		TypeIdWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::simple_type_specifier* symbol)
	{
		TypeSpecifierWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::id_expression* symbol)
	{
		// TODO
		IdExpressionWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		Declaration* declaration = walker.declaration;
		if(declaration != 0)
		{
			if(declaration == &gUndeclared
				|| !isObject(*declaration))
			{
				reportIdentifierMismatch(walker.id->value, declaration, "object-name");
			}
			else
			{
				walker.id->value.dec.p = declaration;
				isTypeDependent |= isDependent(declaration->type);
			}
		}
		else if(walker.qualifying == &context.dependent)
		{
			isTypeDependent = true;
		}
	}
	/* 14.6.2.2-1
	... an expression is type-dependent if any subexpression is type-dependent.
	*/
	void visit(cpp::postfix_expression_disambiguate* symbol)
	{
		// TODO
		/* 14.6.2-1
		In an expression of the form:
		postfix-expression ( expression-list. )
		where the postfix-expression is an unqualified-id but not a template-id, the unqualified-id denotes a dependent
		name if and only if any of the expressions in the expression-list is a type-dependent expression (
		*/
		if(isDependentPrimaryExpression(symbol->left))
		{
			ExpressionWalker walker(*this);
			walker.isTypeDependent = false;
			walker.visit(symbol->right);
			if(!walker.isTypeDependent)
			{
				walker.visit(symbol->left);
			}
			isTypeDependent |= walker.isTypeDependent;
		}
		else
		{
			TREEWALKER_WALK(*this, symbol);
		}
	}
	/* 14.6.2.2-3
	Expressions of the following forms are type-dependent only if the type specified by the type-id, simple-type-specifier
	or new-type-id is dependent, even if any subexpression is type-dependent:
	- postfix-expression-construct
	- new-expression
	- postfix-expression-cast
	- cast-expression
	*/
	void visit(cpp::postfix_expression_construct* symbol)
	{
		ExplicitTypeExpressionWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		isTypeDependent = walker.isTypeDependent;
	}
	void visit(cpp::new_expression_placement* symbol)
	{
		ExplicitTypeExpressionWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		isTypeDependent = walker.isTypeDependent;
	}
	void visit(cpp::new_expression_default* symbol)
	{
		ExplicitTypeExpressionWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		isTypeDependent = walker.isTypeDependent;
	}
	void visit(cpp::postfix_expression_cast* symbol)
	{
		ExplicitTypeExpressionWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		isTypeDependent = walker.isTypeDependent;
	}
	void visit(cpp::cast_expression_default* symbol)
	{
		ExplicitTypeExpressionWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		isTypeDependent = walker.isTypeDependent;
	}
	/* 14.6.2.2-4
	Expressions of the following forms are never type-dependent (because the type of the expression cannot be
	dependent):
	literal
	postfix-expression . pseudo-destructor-name
	postfix-expression -> pseudo-destructor-name
	sizeof unary-expression
	sizeof ( type-id )
	sizeof ... ( identifier )
	alignof ( type-id )
	typeid ( expression )
	typeid ( type-id )
	::opt delete cast-expression
	::opt delete [ ] cast-expression
	throw assignment-expressionopt
	*/
	// TODO: destructor-call is not dependent
	void visit(cpp::unary_expression_sizeof* symbol)
	{
		ExpressionWalker walker(*this);
		walker.isTypeDependent = false;
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::unary_expression_sizeoftype* symbol)
	{
		TypeIdWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::postfix_expression_typeid* symbol)
	{
		ExpressionWalker walker(*this);
		walker.isTypeDependent = false;
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::postfix_expression_typeidtype* symbol)
	{
		TypeIdWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::delete_expression* symbol)
	{
		ExpressionWalker walker(*this);
		walker.isTypeDependent = false;
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::throw_expression* symbol)
	{
		ExpressionWalker walker(*this);
		walker.isTypeDependent = false;
		TREEWALKER_WALK(walker, symbol);
	}
};

struct TemplateArgumentListWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	TemplateArguments arguments;
	bool dependent;

	TemplateArgumentListWalker(const WalkerBase& base)
		: WalkerBase(base), dependent(false)
	{
	}
	void visit(cpp::type_id* symbol)
	{
		TypeIdWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		arguments.push_back(TemplateArgument(walker.type));
	}
	void visit(cpp::assignment_expression* symbol)
	{
		ExpressionWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		arguments.push_back(TemplateArgument(0)); // todo: evaluate constant-expression (unless it's dependent expression)
	}
};

struct TemplateIdWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Type type;
	LookupFilter filter;
	TemplateIdWalker(const WalkerBase& base, LookupFilter filter = isAny)
		: WalkerBase(base), type(0), filter(filter)
	{
	}
	void visit(cpp::identifier* symbol)
	{
		Declaration* declaration = findDeclaration(symbol->value, filter);
		if(declaration == &gUndeclared
			|| !isTemplateName(*declaration))
		{
			reportIdentifierMismatch(symbol->value, declaration, "template-name");
		}
		else
		{
			symbol->value.dec.p = declaration;
		}
		type.declaration = declaration;
	}
	void visit(cpp::template_argument_list* symbol)
	{
		qualifying = 0;
		// TODO: instantiation
		TemplateArgumentListWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
#if 0
		pointOfInstantiation(declaration, walker.arguments);
#endif
		type.arguments = walker.arguments;
	}
};

struct TypeNameWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Type type;
	LookupFilter filter;
	TypeNameWalker(const WalkerBase& base, LookupFilter filter = isAny)
		: WalkerBase(base), type(0), filter(filter)
	{
	}
	void visit(cpp::identifier* symbol)
	{
		Declaration* declaration = findDeclaration(symbol->value, filter);
		if(declaration == &gUndeclared
			|| !isTypeName(*declaration))
		{
			reportIdentifierMismatch(symbol->value, declaration, "type-name");
		}
		else
		{
			symbol->value.dec.p = declaration;
		}
		type.declaration = declaration;
	}
	void visit(cpp::simple_template_id* symbol)
	{
		TemplateIdWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		type = walker.type;
	}
};

struct NestedNameSpecifierWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	bool allowDependent;
	NestedNameSpecifierWalker(const WalkerBase& base, bool allowDependent = false)
		: WalkerBase(base), allowDependent(allowDependent)
	{
	}
	void visit(cpp::namespace_name* symbol)
	{
		NamespaceNameWalker walker(*this, isNestedName);
		TREEWALKER_WALK(walker, symbol);
		setQualifying(walker.declaration);
	}
	void visit(cpp::type_name* symbol)
	{
		if(qualifying != &context.dependent)
		{
			TypeNameWalker walker(*this, isNestedName);
			TREEWALKER_WALK(walker, symbol);
			if(!allowDependent
				&& isDependent(walker.type))
			{
				qualifying = &context.dependent;
			}
			else
			{
				setQualifying(walker.type.declaration);
			}
		}
	}
	void visit(cpp::simple_template_id* symbol)
	{
		if(qualifying != &context.dependent)
		{
			TemplateIdWalker walker(*this, isNestedName);
			TREEWALKER_WALK(walker, symbol);
			if(!allowDependent
				&& isDependent(walker.type))
			{
				qualifying = &context.dependent;
			}
			else
			{
				setQualifying(walker.type.declaration);
			}
		}
	}
};

struct TypeSpecifierWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Type type;
	TypeSpecifierWalker(const WalkerBase& base)
		: WalkerBase(base), type(0)
	{
	}

	void visit(cpp::type_name* symbol)
	{
		TypeNameWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		SEMANTIC_ASSERT(walker.type.declaration != 0);
		type = walker.type;
	}
	void visit(cpp::simple_type_specifier_name* symbol)
	{
		if(symbol->isGlobal.value != 0)
		{
			qualifying = &context.global;
		}
		TREEWALKER_WALK(*this, symbol);
	}
	void visit(cpp::simple_type_specifier_template* symbol)
	{
		if(symbol->isGlobal.value != 0)
		{
			qualifying = &context.global;
		}
		TREEWALKER_WALK(*this, symbol);
	}
	void visit(cpp::nested_name_specifier* symbol)
	{
		NestedNameSpecifierWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		qualifying = walker.qualifying;
	}
	void visit(cpp::simple_template_id* symbol) 
	{
		TemplateIdWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		SEMANTIC_ASSERT(walker.type.declaration != 0);
		type = walker.type;
	}
	void visit(cpp::simple_type_specifier_builtin* symbol)
	{
		// TODO
		type = &gBuiltin;
	}
};

struct DeclaratorIdWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Identifier* id;
	DeclaratorIdWalker(const WalkerBase& base)
		: WalkerBase(base), id(&gAnonymousId)
	{
	}

	void visit(cpp::qualified_id_global* symbol) 
	{
		qualifying = &context.global;
		TREEWALKER_WALK(*this, symbol);
	}
	void visit(cpp::qualified_id_default* symbol) 
	{
		if(symbol->isGlobal.value != 0)
		{
			qualifying = &context.global;
		}
		TREEWALKER_WALK(*this, symbol);
	}
	void visit(cpp::identifier* symbol)
	{
		id = &symbol->value;
	}
	void visit(cpp::nested_name_specifier* symbol)
	{
		NestedNameSpecifierWalker walker(*this, true);
		TREEWALKER_WALK(walker, symbol);
		qualifying = walker.qualifying;
	}
	void visit(cpp::simple_template_id* symbol) 
	{
		id = &symbol->id->value;
		qualifying = 0;
		// TODO: store args
		TemplateArgumentListWalker walker(*this);
		walker.visit(symbol->args);
	}
	void visit(cpp::operator_function_id* symbol) 
	{
		// TODO
		id = &gOperatorFunctionId;
	}
	void visit(cpp::conversion_function_id* symbol) 
	{
		// TODO
		id = &gConversionFunctionId;
	}
	void visit(cpp::destructor_id* symbol) 
	{
		id = &symbol->name->value;
	}
	void visit(cpp::template_id_operator_function* symbol) 
	{
		// TODO
		id = &gOperatorFunctionTemplateId;
	}
};

struct ParameterDeclarationClauseWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	ParameterDeclarationClauseWalker(const WalkerBase& base)
		: WalkerBase(base)
	{
		pushScope(templateParams != 0 ? templateParams : new Scope(makeIdentifier("$declarator")));
		enclosing->type = SCOPETYPE_PROTOTYPE;
		templateParams = 0;
	}

	void visit(cpp::parameter_declaration* symbol)
	{
		SimpleDeclarationWalker walker(*this, true);
		TREEWALKER_WALK(walker, symbol);
	}
};

struct DeclaratorWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Identifier* id;
	Scope* paramScope;
	DeclaratorWalker(const WalkerBase& base)
		: WalkerBase(base), id(&gAnonymousId), paramScope(0)
	{
	}

	void visit(cpp::declarator_id* symbol)
	{
		DeclaratorIdWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		id = walker.id;

		if(walker.qualifying != 0)
		{
			enclosing = walker.qualifying; // names in declarator suffix (array-size, parameter-declaration) are looked up in declarator-id's qualifying scope
		}
	}
	void visit(cpp::parameter_declaration_clause* symbol)
	{
		ParameterDeclarationClauseWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		paramScope = walker.enclosing; // store reference for later resumption
	}
	void visit(cpp::exception_specification* symbol)
	{
		// TODO
	}
	void visit(cpp::constant_expression* symbol)
	{
		ExpressionWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
	}
};

struct BaseSpecifierWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Type type;
	BaseSpecifierWalker(const WalkerBase& base)
		: WalkerBase(base), type(0)
	{
	}

	void visit(cpp::nested_name_specifier* symbol)
	{
		NestedNameSpecifierWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		qualifying = walker.qualifying;
	}
	void visit(cpp::class_name* symbol)
	{
		/* 10-2
		The class-name in a base-specifier shall not be an incompletely defined class (Clause class); this class is
		called a direct base class for the class being defined. During the lookup for a base class name, non-type
		names are ignored (3.3.10)
		*/
		TypeNameWalker walker(*this, isTypeName);
		TREEWALKER_WALK(walker, symbol);
		type = walker.type;
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
		// 3.3.1.3 The point of declaration for a class first declared by a class-specifier is immediately after the identifier or simple-template-id (if any) in its class-head
		declaration = pointOfDeclaration(enclosing, symbol->value, &gClass, enclosed, DeclSpecifiers(), enclosing == templateEnclosing);
		symbol->value.dec.p = declaration;
		SEMANTIC_ASSERT(enclosed != 0);
		enclosed->name = declaration->name;
	}
	void visit(cpp::nested_name_specifier* symbol)
	{
		NestedNameSpecifierWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		qualifying = walker.qualifying;
	}
	void visit(cpp::simple_template_id* symbol) 
	{
		// TODO: don't declare anything - this is a template (partial) specialisation
		// 3.3.1.3 The point of declaration for a class first declared by a class-specifier is immediately after the identifier or simple-template-id (if any) in its class-head
		declaration = pointOfDeclaration(enclosing, symbol->id->value, &gClass, enclosed, DeclSpecifiers(), enclosing == templateEnclosing, true);
		symbol->id->value.dec.p = declaration;
		SEMANTIC_ASSERT(enclosed != 0);
		enclosed->name = declaration->name;
		// TODO: store args
		qualifying = 0;
		TemplateArgumentListWalker walker(*this);
		walker.visit(symbol->args);
	}
	void visit(cpp::class_head_anonymous* symbol)
	{
		declaration = pointOfDeclaration(enclosing, makeIdentifier(enclosing->getUniqueName()), &gClass, enclosed);
		SEMANTIC_ASSERT(enclosed != 0);
		enclosed->name = declaration->name;
		TREEWALKER_WALK(*this, symbol);
	}
	void visit(cpp::base_specifier* symbol) 
	{
		BaseSpecifierWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		SEMANTIC_ASSERT(walker.type.declaration != 0);
		SEMANTIC_ASSERT(declaration->enclosed != 0);
		declaration->enclosed->bases.push_back(getOriginalType(walker.type));
	}
};

struct UsingDeclarationWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	UsingDeclarationWalker(const WalkerBase& base)
		: WalkerBase(base)
	{
	}
	void walkUnqualifiedId(cpp::symbol<cpp::unqualified_id> symbol)
	{
		UnqualifiedIdWalker walker(*this);
		walker.visit(symbol);
		Declaration* declaration = walker.declaration;
		if(declaration == &gUndeclared
			|| !(isObject(*declaration) || isTypeName(*declaration)))
		{
			reportIdentifierMismatch(walker.id->value, declaration, "object-name or type-name");
		}
		else
		{
			walker.id->value.dec.p = declaration;
		}
	}
	void visit(cpp::using_declaration_global* symbol)
	{
		qualifying = &context.global;
		walkUnqualifiedId(symbol->id);
	}
	void visit(cpp::using_declaration_nested* symbol)
	{
		if(symbol->isGlobal.value != 0)
		{
			qualifying = &context.global;
		}
		NestedNameSpecifierWalker walker(*this);
		walker.visit(symbol->context);
		qualifying = walker.qualifying;
		if(symbol->isTypename.value == 0)
		{
			walkUnqualifiedId(symbol->id);
		}
	}
};

struct MemberDeclarationWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Declaration* declaration;
	MemberDeclarationWalker(const WalkerBase& base)
		: WalkerBase(base), declaration(0)
	{
	}
	void visit(cpp::member_template_declaration* symbol)
	{
		TemplateDeclarationWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
	}
	void visit(cpp::member_declaration_implicit* symbol)
	{
		SimpleDeclarationWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
	}
	void visit(cpp::member_declaration_default* symbol)
	{
		if(typeid(*symbol->suffix.p) == typeid(cpp::forward_declaration_suffix)
			&& isForwardDeclaration(symbol->spec))
		{
			ForwardDeclarationWalker walker(*this);
			TREEWALKER_WALK(walker, symbol);
			SEMANTIC_ASSERT(walker.declaration != 0);
			declaration = walker.declaration;
		}
		else
		{
			SimpleDeclarationWalker walker(*this);
			TREEWALKER_WALK(walker, symbol);
			SEMANTIC_ASSERT(walker.type.declaration != 0);
			declaration = walker.declaration;
		}
	}
	void visit(cpp::member_declaration_nested* symbol)
	{
		SimpleDeclarationWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
	}
	void visit(cpp::using_declaration* symbol)
	{
		UsingDeclarationWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
	}
};


struct ClassSpecifierWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Declaration* declaration;
	DeferredSymbols deferred;
	ClassSpecifierWalker(const WalkerBase& base)
		: WalkerBase(base), declaration(0)
	{
	}
	void walkDeferred()
	{
		for(DeferredSymbols::const_iterator i = deferred.begin(); i != deferred.end(); ++i)
		{
			(*i).func((*i).context, (*i).symbol);
		}
	};

	void visit(cpp::class_head* symbol)
	{
		Scope* scope = templateParams != 0 ? templateParams : new Scope(makeIdentifier("$class"));
		scope->type = SCOPETYPE_CLASS;
		ClassHeadWalker walker(*this, scope);
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
		pushScope(scope); // 3.3.6.1.1 // class scope
		templateParams = 0;
	}
	void visit(cpp::member_declaration* symbol)
	{
		MemberDeclarationWalker walker(*this);
		if(walker.deferred == 0)
		{
			walker.deferred = &deferred;
		}
		TREEWALKER_WALK(walker, symbol);
	}
};

struct EnumSpecifierWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	EnumSpecifierWalker(const WalkerBase& base)
		: WalkerBase(base)
	{
	}

	void visit(cpp::enumerator_definition* symbol)
	{
		// TODO: init
		/* 3.1-4
		The point of declaration for an enumerator is immediately after its enumerator-definition.
		*/
		// TODO: give enumerators a type
		Declaration* declaration = pointOfDeclaration(enclosing, symbol->id->value, &gBuiltin, 0, DeclSpecifiers());
		symbol->id->value.dec.p = declaration;

		ExpressionWalker walker(*this);
		walker.visit(symbol->init);
	}
};

struct ElaboratedTypeSpecifierWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Declaration* type;
	Declaration* declaration;
	ElaboratedTypeSpecifierWalker(const WalkerBase& base)
		: WalkerBase(base), type(0), declaration(0)
	{
	}
	void visit(cpp::class_key* symbol)
	{
		type = &gClass;
	}
	void visit(cpp::enum_key* symbol)
	{
		type = &gEnum;
	}
	void visit(cpp::identifier* symbol)
	{
		/* 3.4.4-2
		If the elaborated-type-specifier has no nested-name-specifier ...
		... the identifier is looked up according to 3.4.1 but ignoring any non-type names that have been declared. If
		the elaborated-type-specifier is introduced by the enum keyword and this lookup does not find a previously
		declared type-name, the elaborated-type-specifier is ill-formed. If the elaborated-type-specifier is introduced by
		the class-key and this lookup does not find a previously declared type-name ...
		the elaborated-type-specifier is a declaration that introduces the class-name as described in 3.3.1.
		*/
		declaration = findDeclaration(symbol->value, isType);
		if(declaration != &gUndeclared)
		{
			symbol->value.dec.p = declaration;
			/* 7.1.6.3-2
			3.4.4 describes how name lookup proceeds for the identifier in an elaborated-type-specifier. If the identifier
			resolves to a class-name or enum-name, the elaborated-type-specifier introduces it into the declaration the
			same way a simple-type-specifier introduces its type-name. If the identifier resolves to a typedef-name, the
			elaborated-type-specifier is ill-formed.
			*/
			if(isTypedef(*declaration))
			{
				printPosition(symbol->value.position);
				std::cout << "'" << symbol->value.value << "': elaborated-type-specifier refers to a typedef" << std::endl;
				printPosition(declaration->name.position);
				throw SemanticError();
			}
			/* 7.1.6.3-3
			The class-key or enum keyword present in the elaborated-type-specifier shall agree in kind with the declaration
			to which the name in the elaborated-type-specifier refers.
			*/
			if(declaration->type.declaration != type)
			{
				printPosition(symbol->value.position);
				std::cout << "'" << symbol->value.value << "': elaborated-type-specifier key does not match declaration" << std::endl;
				printPosition(declaration->name.position);
				throw SemanticError();
			}
		}
		else
		{
			/* 3.4.4-2
			... If the elaborated-type-specifier is introduced by the enum keyword and this lookup does not find a previously
			declared type-name, the elaborated-type-specifier is ill-formed. If the elaborated-type-specifier is introduced by
			the class-key and this lookup does not find a previously declared type-name ...
			the elaborated-type-specifier is a declaration that introduces the class-name as described in 3.3.1.
			*/
			if(type != &gClass)
			{
				SEMANTIC_ASSERT(type == &gEnum);
				printPosition(symbol->value.position);
				std::cout << "'" << symbol->value.value << "': elaborated-type-specifier refers to undefined enum" << std::endl;
				throw SemanticError();
			}
			/* 3.3.1-6
			if the elaborated-type-specifier is used in the decl-specifier-seq or parameter-declaration-clause of a
			function defined in namespace scope, the identifier is declared as a class-name in the namespace that
			contains the declaration; otherwise, except as a friend declaration, the identifier is declared in the
			smallest non-class, non-function-prototype scope that contains the declaration.
			*/
			declaration = pointOfDeclaration(getEltScope(), symbol->value, &gClass, 0, DeclSpecifiers(), enclosing == templateEnclosing);
			symbol->value.dec.p = declaration;
		}
	}
};

struct DeclSpecifierSeqWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Type type;
	DeclSpecifiers specifiers;
	DeclSpecifierSeqWalker(const WalkerBase& base)
		: WalkerBase(base), type(0)
	{
	}

	void visit(cpp::simple_type_specifier* symbol)
	{
		TypeSpecifierWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		type = walker.type;
	}
	void visit(cpp::elaborated_type_specifier_template* symbol)
	{
		TypeSpecifierWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		type = walker.type;
	}
	void visit(cpp::elaborated_type_specifier_default* symbol)
	{
		if(isUnqualified(symbol))
		{
			ElaboratedTypeSpecifierWalker walker(*this);
			TREEWALKER_WALK(walker, symbol);
			type = walker.declaration;
		}
		else
		{
			TypeSpecifierWalker walker(*this);
			TREEWALKER_WALK(walker, symbol);
			type = walker.type;
		}
	}
	void visit(cpp::typename_specifier* symbol)
	{
		NestedNameSpecifierWalker walker(*this);
		walker.visit(symbol->context);
		type = &gTypename;
	}
	void visit(cpp::class_specifier* symbol)
	{
		ClassSpecifierWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		type = walker.declaration;
		walker.walkDeferred();
	}
	void visit(cpp::enum_specifier* symbol)
	{
		Identifier id = symbol->id.p != 0 ? symbol->id->value : makeIdentifier(enclosing->getUniqueName());
		Declaration* declaration = pointOfDeclaration(enclosing, id, &gEnum, 0);
		if(symbol->id.p != 0)
		{
			symbol->id->value.dec.p = declaration;
		}
		EnumSpecifierWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		type = declaration;
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
		if(typeid(*symbol->suffix.p) == typeid(cpp::forward_declaration_suffix)
			&& isForwardDeclaration(symbol->spec))
		{
			ForwardDeclarationWalker walker(*this);
			TREEWALKER_WALK(walker, symbol);
			SEMANTIC_ASSERT(walker.declaration != 0);
		}
		else
		{
			SimpleDeclarationWalker walker(*this);
			TREEWALKER_WALK(walker, symbol);
			SEMANTIC_ASSERT(walker.type.declaration != 0);
		}
	}
	void visit(cpp::selection_statement* symbol)
	{
		ControlStatementWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::iteration_statement* symbol)
	{
		ControlStatementWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::compound_statement* symbol)
	{
		CompoundStatementWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::expression_statement* symbol)
	{
		ExpressionWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::jump_statement_return* symbol)
	{
		ExpressionWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::jump_statement_goto* symbol)
	{
		// TODO
	}
};

struct ControlStatementWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	ControlStatementWalker(const WalkerBase& base)
		: WalkerBase(base)
	{
		pushScope(new Scope(makeIdentifier(enclosing->getUniqueName()), SCOPETYPE_LOCAL));
	}
	void visit(cpp::condition_init* symbol)
	{
		SimpleDeclarationWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		SEMANTIC_ASSERT(walker.type.declaration != 0);
	}
	void visit(cpp::simple_declaration* symbol)
	{
		SimpleDeclarationWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::statement* symbol)
	{
		StatementWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::expression* symbol)
	{
		ExpressionWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
	}
};

struct CompoundStatementWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	CompoundStatementWalker(const WalkerBase& base)
		: WalkerBase(base)
	{
		pushScope(new Scope(makeIdentifier(enclosing->getUniqueName()), SCOPETYPE_LOCAL)); // local scope
	}

	void visit(cpp::statement* symbol)
	{
		StatementWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
	}
};


struct FunctionBodyWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	FunctionBodyWalker(const WalkerBase& base)
		: WalkerBase(base)
	{
	}
	void visit(cpp::compound_statement* symbol)
	{
		CompoundStatementWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
	}
};

struct HandlerSeqWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	HandlerSeqWalker(const WalkerBase& base)
		: WalkerBase(base)
	{
	}
	// TODO: handler
	void visit(cpp::compound_statement* symbol)
	{
		CompoundStatementWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
	}
};

struct QualifiedTypeNameWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	QualifiedTypeNameWalker(const WalkerBase& base)
		: WalkerBase(base)
	{
	}
	void visit(cpp::nested_name_specifier* symbol)
	{
		NestedNameSpecifierWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		qualifying = walker.qualifying;
	}
	void visit(cpp::class_name* symbol)
	{
		if(qualifying != &context.dependent)
		{
			TypeNameWalker walker(*this);
			TREEWALKER_WALK(walker, symbol);
		}
	}
};

struct MemInitializerWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	MemInitializerWalker(const WalkerBase& base)
		: WalkerBase(base)
	{
	}
	void visit(cpp::mem_initializer_id_base* symbol)
	{
		if(symbol->isGlobal.value != 0)
		{
			qualifying = &context.global;
		}
		QualifiedTypeNameWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::identifier* symbol)
	{
		Declaration* declaration = findDeclaration(symbol->value);
		if(declaration == &gUndeclared
			|| !isObject(*declaration))
		{
			reportIdentifierMismatch(symbol->value, declaration, "object-name");
		}
		else
		{
			symbol->value.dec.p = declaration;
		}
	}
	void visit(cpp::expression_list* symbol)
	{
		ExpressionWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
	}
};

struct MemInitializerListWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	MemInitializerListWalker(const WalkerBase& base)
		: WalkerBase(base)
	{
	}
	void visit(cpp::mem_initializer* symbol)
	{
		MemInitializerWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
	}
};

struct SimpleDeclarationWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Type type;
	Declaration* declaration;
	DeclSpecifiers specifiers;
	bool isParameter;

	SimpleDeclarationWalker(const WalkerBase& base, bool isParameter = false)
		: WalkerBase(base), type(&gCtor), declaration(0), isParameter(isParameter)
	{
	}


	void visit(cpp::decl_specifier_seq* symbol)
	{
		DeclSpecifierSeqWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		type = walker.type;
		declaration = type.declaration; // if no declarator is specified later, this is probably a class-declaration
		specifiers = walker.specifiers;
	}
	void visit(cpp::type_specifier_seq* symbol)
	{
		DeclSpecifierSeqWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		type = walker.type;
		declaration = type.declaration; // if no declarator is specified later, this is probably a class-declaration
	}

	template<typename T>
	void walkDeclarator(T* symbol)
	{
		DeclaratorWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		declaration = pointOfDeclaration(
			walker.enclosing,
			*walker.id,
			type,
#if 0
			walker.paramScope,
#else	
			specifiers.isTypedef ? type.declaration->enclosed : walker.paramScope,
#endif
			specifiers,
			enclosing == templateEnclosing); // 3.3.1.1
		if(walker.id != &gAnonymousId)
		{
			walker.id->dec.p = declaration;
		}
		Scope* enclosed = templateParams != 0 ? templateParams : declaration->enclosed;
		if(enclosed != 0
			&& !specifiers.isTypedef)
		{
			enclosed->name = declaration->name;
		}
		enclosing = walker.enclosing;
		if(walker.paramScope != 0)
		{
			enclosing = walker.paramScope; // 3.3.2.1 parameter scope
		}
		else if(templateParams != 0)
		{
			pushScope(templateParams);
		}
		templateParams = 0;
	}
	void visit(cpp::declarator* symbol)
	{
		walkDeclarator(symbol);
	}
	void visit(cpp::declarator_disambiguate* symbol)
	{
		walkDeclarator(symbol);
	}
	void visit(cpp::abstract_declarator* symbol)
	{
		DeclaratorWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::member_declarator_bitfield* symbol)
	{
		if(symbol->id.p != 0)
		{
			declaration = pointOfDeclaration(enclosing, symbol->id->value, type, 0, specifiers); // 3.3.1.1
			symbol->id->value.dec.p = declaration;
		}
	}

	// handle assignment-expression(s) in initializer
	void visit(cpp::assignment_expression* symbol)
	{
		ExpressionWalker walker(*this);
		if(isParameter)
		{
			walkDeferable(walker, symbol);
		}
		else
		{
			TREEWALKER_WALK(walker, symbol);
		}
	}
	template<typename Walker, typename T>
	void walkDeferable(Walker& walker, T* symbol)
	{
		// defer name-lookup for function-body, default-arguments and ctor-initializers
		if(deferred != 0)
		{
			deferred->push_back(makeDeferredSymbol(walker, symbol));
		}
		else
		{
			TREEWALKER_WALK(walker, symbol);
		}
	}
	void visit(cpp::function_body* symbol)
	{
		FunctionBodyWalker walker(*this);
		walkDeferable(walker, symbol);
	}
	void visit(cpp::ctor_initializer* symbol)
	{
		MemInitializerListWalker walker(*this);
		walkDeferable(walker, symbol);
	}
	void visit(cpp::handler_seq* symbol)
	{
		HandlerSeqWalker walker(*this);
		walkDeferable(walker, symbol);
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
		declaration = pointOfDeclaration(enclosing, symbol->id->value, &gClass, 0, DeclSpecifiers(), enclosing == templateEnclosing);
		symbol->id->value.dec.p = declaration;
	}
};

struct TemplateParameterListWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	TemplateParameterListWalker(WalkerBase& base)
		: WalkerBase(base)
	{
	}
	void visit(cpp::type_parameter_default* symbol)
	{
		if(symbol->id != 0)
		{
			enclosing->templateParams.push_back(gParam);
			symbol->id->value.dec.p = pointOfDeclaration(enclosing, symbol->id->value, &enclosing->templateParams.back(), 0, DECLSPEC_TYPEDEF);
		}
		TypeIdWalker walker(*this);
		walker.visit(symbol->init);
	}
	void visit(cpp::type_parameter_template* symbol)
	{
		// TODO
		if(symbol->id != 0)
		{
			enclosing->templateParams.push_back(gParam);
			symbol->id->value.dec.p = pointOfDeclaration(enclosing, symbol->id->value, &enclosing->templateParams.back(), 0, DECLSPEC_TYPEDEF, true);
		}
		{
			TemplateParameterListWalker walker(*this);
			walker.visit(symbol->params);
		}
		{
			IdExpressionWalker walker(*this);
			walker.visit(symbol->init);
			Declaration* declaration = walker.declaration;
			if(declaration != 0)
			{
				if(declaration == &gUndeclared
					|| !isTemplateName(*declaration))
				{
					reportIdentifierMismatch(walker.id->value, declaration, "template-name");
				}
				else
				{
					walker.id->value.dec.p = declaration;
				}
			}
		}
	}
	void visit(cpp::parameter_declaration* symbol)
	{
		SimpleDeclarationWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
	}
};

struct TemplateDeclarationWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Declaration* declaration;
	TemplateDeclarationWalker(WalkerBase& base)
		: WalkerBase(base), declaration(0)
	{
		templateEnclosing = enclosing;
	}
	void visit(cpp::template_parameter_list* symbol)
	{
		// collect template-params into a new scope
		Scope* params = templateParams != 0 ? templateParams : new Scope(makeIdentifier("$template"), SCOPETYPE_TEMPLATE);
		templateParams = 0;
		pushScope(params);
		TemplateParameterListWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		enclosing = params->parent;
		templateParams = params;
	}
	void visit(cpp::declaration* symbol)
	{
		DeclarationWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
		SEMANTIC_ASSERT(declaration != 0);
	}
	void visit(cpp::member_declaration* symbol)
	{
		MemberDeclarationWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
		SEMANTIC_ASSERT(declaration != 0);
	}
};

struct DeclarationWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Declaration* declaration;
	DeclarationWalker(WalkerBase& base)
		: WalkerBase(base), declaration(0)
	{
	}
	void visit(cpp::namespace_definition* symbol)
	{
		NamespaceWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
	}
	void visit(cpp::general_declaration* symbol)
	{
		if(typeid(*symbol->suffix.p) == typeid(cpp::forward_declaration_suffix)
			&& isForwardDeclaration(symbol->spec))
		{
			ForwardDeclarationWalker walker(*this);
			TREEWALKER_WALK(walker, symbol);
			SEMANTIC_ASSERT(walker.declaration != 0);
			declaration = walker.declaration;
		}
		else
		{
			SimpleDeclarationWalker walker(*this);
			TREEWALKER_WALK(walker, symbol);
			SEMANTIC_ASSERT(walker.type.declaration != 0);
			declaration = walker.declaration;
		}
	}
	// occurs in for-init-statement
	void visit(cpp::simple_declaration* symbol)
	{
		if(typeid(*symbol->suffix.p) == typeid(cpp::forward_declaration_suffix)
			&& isForwardDeclaration(symbol->spec))
		{
			ForwardDeclarationWalker walker(*this);
			TREEWALKER_WALK(walker, symbol);
			SEMANTIC_ASSERT(walker.declaration != 0);
			declaration = walker.declaration;
		}
		else
		{
			SimpleDeclarationWalker walker(*this);
			TREEWALKER_WALK(walker, symbol);
			SEMANTIC_ASSERT(walker.type.declaration != 0);
			declaration = walker.declaration;
		}
	}
	void visit(cpp::template_declaration* symbol)
	{
		TemplateDeclarationWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
	}
	void visit(cpp::explicit_specialization* symbol)
	{
		TemplateDeclarationWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
	}
	void visit(cpp::using_declaration* symbol)
	{
		UsingDeclarationWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
	}
};

struct NamespaceWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Declaration* declaration;
	NamespaceWalker(WalkerContext& context)
		: WalkerBase(context), declaration(0)
	{
		pushScope(&context.global);
	}

	NamespaceWalker(WalkerBase& base)
		: WalkerBase(base)
	{
	}

	void visit(cpp::identifier* symbol)
	{
		declaration = pointOfDeclaration(enclosing, symbol->value, &gNamespace, 0);
		symbol->value.dec.p = declaration;
		if(declaration->enclosed == 0)
		{
			declaration->enclosed = new Scope(symbol->value, SCOPETYPE_NAMESPACE);
		}
		pushScope(declaration->enclosed);
	}
	void visit(cpp::declaration* symbol)
	{
		DeclarationWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
	}
};

};


void printSymbol(cpp::declaration_seq* p, const char* path)
{
	try
	{
		WalkerContext context;
		Walker::NamespaceWalker walker(context);
		walker.visit(makeSymbol(p));

		SymbolPrinter printer(path);
		printer.visit(makeSymbol(p));
	}
	catch(SemanticError&)
	{
	}
}

void printSymbol(cpp::statement_seq* p, const char* path)
{
	try
	{
		WalkerContext context;
		Walker::NamespaceWalker walker(context);
		walker.visit(makeSymbol(p));

		SymbolPrinter printer(path);
		printer.visit(makeSymbol(p));
	}
	catch(SemanticError&)
	{
	}

}





#if 0
cpp::declaration_seq* parseFile(Lexer& lexer)
{
	WalkerContext context;
	Walker::NamespaceWalker walker(context);
	ParserGeneric<Walker::NamespaceWalker> parser(lexer, walker);

	cpp::symbol_optional<cpp::declaration_seq> result(NULL);
	try
	{
		ProfileScope profile(gProfileParser);
		PARSE_SEQUENCE(parser, result);
	}
	catch(ParseError&)
	{
	}
	if(!lexer.finished())
	{
		printError(parser);
	}
	dumpProfile(gProfileWave);
	dumpProfile(gProfileParser);
	dumpProfile(gProfileAmbiguity);
	dumpProfile(gProfileDiagnose);
	dumpProfile(gProfileAllocator);
	dumpProfile(gProfileIdentifier);
	return result;
}

cpp::statement_seq* parseFunction(Lexer& lexer)
{
	WalkerContext context;
	Walker::CompoundStatementWalker walker(context);
	ParserGeneric<Walker::CompoundStatementWalker> parser(lexer, walker);

	cpp::symbol_optional<cpp::statement_seq> result(NULL);
	try
	{
		ProfileScope profile(gProfileParser);
		PARSE_SEQUENCE(parser, result);
	}
	catch(ParseError&)
	{
	}
	if(!lexer.finished())
	{
		printError(parser);
	}
	return result;
}
#endif

