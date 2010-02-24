
#if 1

namespace N
{
	template<typename T>
	class C
	{
		template<typename X>
		void f()
		{
			N::undeclared<X> x;
		}
	};
}

struct S
{
};

template<typename First, typename Second>
int f(First x, Second y);

int x = f(S(), int());

template<typename Second, typename First>
int f(Second a, First b)
{
	return b;
}

#endif

#include "semantic.h"

#include "cpptree.h"
#include "printer.h"

#include <fstream>

#define SYMBOL_NAME(T) (typeid(T).name() + 12)

#include <iostream>
#include <list>

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


inline void printPosition(const LexFilePosition& position)
{
	std::cout << position.get_file() << "(" << position.get_line() << "): ";
}


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
	typedef std::vector<Scope*> Bases;
	Bases bases;

	Scope(Identifier name, ScopeType type)
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

Identifier IDENTIFIER_NULL = makeIdentifier(0);

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

Declaration gParam(0, makeIdentifier("$param"), &gClass, 0);

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

// objects
Identifier gOperatorFunctionId = makeIdentifier("operator <op>");
Identifier gConversionFunctionId = makeIdentifier("operator T");
Identifier gOperatorFunctionTemplateId = makeIdentifier("operator () <>");
// TODO: don't declare if id is anonymous?
Identifier gAnonymousId = makeIdentifier("$anonymous");



bool isType(const Declaration& type)
{
	return type.specifiers.isTypedef
		|| type.type == &gSpecial
		|| type.type == &gEnum
		|| type.type == &gClass;
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
	if(declaration.type->type == 0)
	{
		return &declaration;
	}
#endif
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

const Declaration& getOriginalType(const Declaration& declaration)
{
	if(declaration.specifiers.isTypedef)
	{
		return getOriginalType(*declaration.type);
	}
	return declaration;
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
	return declaration.type == &gClass;
}

bool isEnum(const Declaration& declaration)
{
	return declaration.type == &gEnum;
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
	return declaration.type == &gNamespace;
}

bool isExtern(const Declaration& declaration)
{
	return declaration.specifiers.isExtern;
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

void printIdentifierMismatch(const IdentifierMismatch& e)
{
	printPosition(e.id.position);
	std::cout << "'" << getValue(e.id) << "' expected " << e.expected << ", " << (e.declaration == &gUndeclared ? " was undeclared" : "was declared here:") << std::endl;
	if(e.declaration != &gUndeclared)
	{
		printPosition(e.declaration->name.position);
	}
}


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
		std::cout << getValue((*i)->name) << ": ";
		printDeclarations((*i)->declarations);
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


bool isAny(const Declaration& declaration)
{
	return declaration.type != &gCtor;
}

typedef bool (*LookupFilter)(const Declaration&);


struct WalkerContext
{
	Scope global;

	WalkerContext() :
	global(makeIdentifier("$global"), SCOPETYPE_NAMESPACE)
	{
	}
};

struct WalkerBase
{
	WalkerContext& context;
	Scope* enclosing;
	Scope* templateParams;
	Scope* templateEnclosing;
	bool* ambiguity;

	WalkerBase(WalkerContext& context)
		: context(context), enclosing(0), templateParams(0), templateEnclosing(0), ambiguity(0)
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
			Declaration* result = findDeclaration((*i)->declarations, (*i)->bases, id, filter);
			if(result != 0)
			{
				return result;
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
		if(templateParams != 0)
		{
			Declaration* result = findDeclaration(*templateParams, id, filter);
			if(result != 0)
			{
				return result;
			}
		}
		{
			Declaration* result = findDeclaration(*enclosing, id, filter);
			if(result != 0)
			{
				return result;
			}
		}
		return &gUndeclared;
	}

	Declaration* pointOfDeclaration(Scope* parent, const Identifier& name, Declaration* type, Scope* enclosed, DeclSpecifiers specifiers = DeclSpecifiers(), bool isTemplate = false, bool isTemplateSpecialization = false)
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

	void setScope(Declaration* declaration)
	{
		if(declaration->enclosed == 0)
		{
			// TODO
			//printPosition(symbol->id->value.position);
			std::cout << "'" << getValue(declaration->name) << "' is incomplete, declared here:" << std::endl;
			printPosition(declaration->name.position);
			throw SemanticError();
		}
		enclosing = declaration->enclosed;
	}

	void reportIdentifierMismatch(const Identifier& id, Declaration* declaration, const char* expected)
	{
		if(ambiguity != 0)
		{
			throw IdentifierMismatch(id, declaration, expected);
		}
		printIdentifierMismatch(IdentifierMismatch(id, declaration, expected));
		printScope(*enclosing);
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
		|| isTemplateName(declaration)
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
		std::cout << "second:" << std::endl;
		printIdentifierMismatch(second);
		throw SemanticError();
	}
	throw first;
}

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
		if(walkAmbiguity(*this, symbol) != symbol->first) \
		{ \
			std::swap(symbol->first, symbol->second); \
		} \
	}

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
		symbol->accept(walker);
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

typedef std::vector<DeferredSymbol> DeferredSymbols;

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

struct TemplateIdWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Declaration* declaration;
	LookupFilter filter;
	TemplateIdWalker(const WalkerBase& base, LookupFilter filter = isAny)
		: WalkerBase(base), declaration(0), filter(filter)
	{
	}
	void visit(cpp::identifier* symbol)
	{
		declaration = findDeclaration(symbol->value, filter);
		if(declaration == &gUndeclared
			|| !isTemplateName(*declaration))
		{
			reportIdentifierMismatch(symbol->value, declaration, "template-name");
		}
		else
		{
			symbol->value.dec.p = declaration;
		}
	}
	void visit(cpp::template_argument_list* symbol)
	{
		// TODO
	}
};

struct TypeNameWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Declaration* declaration;
	LookupFilter filter;
	TypeNameWalker(const WalkerBase& base, LookupFilter filter = isAny)
		: WalkerBase(base), declaration(0), filter(filter)
	{
	}
	void visit(cpp::identifier* symbol)
	{
		declaration = findDeclaration(symbol->value, filter);
		if(declaration == &gUndeclared
			|| !isTypeName(*declaration))
		{
			reportIdentifierMismatch(symbol->value, declaration, "type-name");
		}
		else
		{
			symbol->value.dec.p = declaration;
		}
	}
	void visit(cpp::simple_template_id* symbol)
	{
		TemplateIdWalker walker(*this);
		symbol->accept(walker);
		declaration = walker.declaration;
	}
};

struct NestedNameSpecifierWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	NestedNameSpecifierWalker(const WalkerBase& base)
		: WalkerBase(base)
	{
	}
	void visit(cpp::namespace_name* symbol)
	{
		NamespaceNameWalker walker(*this, isNestedName);
		symbol->accept(walker);
		setScope(walker.declaration);
	}
	void visit(cpp::type_name* symbol)
	{
		TypeNameWalker walker(*this, isNestedName);
		symbol->accept(walker);
		setScope(walker.declaration);
	}
	void visit(cpp::simple_template_id* symbol)
	{
		TemplateIdWalker walker(*this, isNestedName);
		symbol->accept(walker);
		setScope(walker.declaration);
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

	void visit(cpp::type_name* symbol)
	{
		TypeNameWalker walker(*this);
		symbol->accept(walker);
		SEMANTIC_ASSERT(walker.declaration != 0);
		declaration = walker.declaration;
	}
	void visit(cpp::simple_type_specifier_name* symbol)
	{
		if(symbol->isGlobal.value != 0)
		{
			enclosing = &context.global;
		}
		symbol->accept(*this);
	}
	void visit(cpp::simple_type_specifier_template* symbol)
	{
		if(symbol->isGlobal.value != 0)
		{
			enclosing = &context.global;
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
		TemplateIdWalker walker(*this);
		symbol->accept(walker);
		SEMANTIC_ASSERT(walker.declaration != 0);
		declaration = walker.declaration;
	}
	void visit(cpp::simple_type_specifier_builtin* symbol)
	{
		// TODO
		declaration = &gBuiltin;
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
		enclosing = &context.global;
		symbol->accept(*this);
	}
	void visit(cpp::qualified_id_default* symbol) 
	{
		if(symbol->isGlobal.value != 0)
		{
			enclosing = &context.global;
		}
		symbol->accept(*this);
	}
	void visit(cpp::identifier* symbol)
	{
		id = &symbol->value;
	}
	void visit(cpp::nested_name_specifier* symbol)
	{
		NestedNameSpecifierWalker walker(*this);
		symbol->accept(walker);
		enclosing = walker.enclosing;
	}
	void visit(cpp::simple_template_id* symbol) 
	{
		// TODO: args
		id = &symbol->id->value;
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

	Identifier* id;
	Scope* paramScope;
	DeclaratorWalker(const WalkerBase& base)
		: WalkerBase(base), id(&gAnonymousId), paramScope(0)
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
		// TODO
	}
};

struct BaseSpecifierWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Declaration* type;
	BaseSpecifierWalker(const WalkerBase& base)
		: WalkerBase(base), type(0)
	{
	}

	void visit(cpp::nested_name_specifier* symbol)
	{
		NestedNameSpecifierWalker walker(*this);
		symbol->accept(walker);
		enclosing = walker.enclosing;
	}
	void visit(cpp::class_name* symbol)
	{
		/* 10-2
		The class-name in a base-specifier shall not be an incompletely defined class (Clause class); this class is
		called a direct base class for the class being defined. During the lookup for a base class name, non-type
		names are ignored (3.3.10)
		*/
		TypeNameWalker walker(*this, isTypeName);
		symbol->accept(walker);
		type = walker.declaration;
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
		symbol->accept(walker);
		enclosing = walker.enclosing;
	}
	void visit(cpp::simple_template_id* symbol) 
	{
		// TODO: don't declare anything - this is a template (partial) specialisation
		// 3.3.1.3 The point of declaration for a class first declared by a class-specifier is immediately after the identifier or simple-template-id (if any) in its class-head
		declaration = pointOfDeclaration(enclosing, symbol->id->value, &gClass, enclosed, DeclSpecifiers(), enclosing == templateEnclosing, true);
		symbol->id->value.dec.p = declaration;
		SEMANTIC_ASSERT(enclosed != 0);
		enclosed->name = declaration->name;
		// TODO args
	}
	void visit(cpp::class_head_anonymous* symbol)
	{
		declaration = pointOfDeclaration(enclosing, makeIdentifier(enclosing->getUniqueName()), &gClass, enclosed);
		SEMANTIC_ASSERT(enclosed != 0);
		enclosed->name = declaration->name;
		symbol->accept(*this);
	}
	void visit(cpp::base_specifier* symbol) 
	{
		BaseSpecifierWalker walker(*this);
		symbol->accept(walker);
		SEMANTIC_ASSERT(walker.type != 0);
		const Declaration& type = getOriginalType(*walker.type);
		if(type.enclosed != 0) // TODO: template-param dependent bases
		{
			SEMANTIC_ASSERT(declaration->enclosed != 0);
			declaration->enclosed->bases.push_back(type.enclosed);
		}
	}
};

struct MemberDeclarationWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Declaration* declaration;
	DeferredSymbols* deferred;
	MemberDeclarationWalker(const WalkerBase& base, DeferredSymbols* deferred)
		: WalkerBase(base), declaration(0), deferred(deferred)
	{
	}
	void visit(cpp::member_template_declaration* symbol)
	{
		TemplateDeclarationWalker walker(*this, deferred);
		symbol->accept(walker);
		declaration = walker.declaration;
	}
	void visit(cpp::member_declaration_implicit* symbol)
	{
		SimpleDeclarationWalker walker(*this, deferred);
		symbol->accept(walker);
		declaration = walker.declaration;
	}
	void visit(cpp::member_declaration_default* symbol)
	{
		if(typeid(*symbol->suffix.p) == typeid(cpp::forward_declaration_suffix)
			&& isForwardDeclaration(symbol->spec))
		{
			ForwardDeclarationWalker walker(*this);
			symbol->accept(walker);
			SEMANTIC_ASSERT(walker.declaration != 0);
			declaration = walker.declaration;
		}
		else
		{
			SimpleDeclarationWalker walker(*this, deferred);
			symbol->accept(walker);
			SEMANTIC_ASSERT(walker.type != 0);
			declaration = walker.declaration;
		}
	}
	void visit(cpp::member_declaration_nested* symbol)
	{
		SimpleDeclarationWalker walker(*this, deferred);
		symbol->accept(walker);
		declaration = walker.declaration;
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
			if(declaration->type != type)
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
		if(isUnqualified(symbol))
		{
			ElaboratedTypeSpecifierWalker walker(*this);
			symbol->accept(walker);
			declaration = walker.declaration;
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
		Identifier id = symbol->id.p != 0 ? symbol->id->value : makeIdentifier(enclosing->getUniqueName());
		declaration = pointOfDeclaration(enclosing, id, &gEnum, 0);
		if(symbol->id.p != 0)
		{
			symbol->id->value.dec.p = declaration;
		}
		EnumSpecifierWalker walker(*this);
		symbol->accept(walker);
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
	void visit(cpp::compound_statement* symbol)
	{
		CompoundStatementWalker walker(*this);
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
		pushScope(new Scope(makeIdentifier(enclosing->getUniqueName()), SCOPETYPE_LOCAL)); // local scope
	}

	void visit(cpp::statement* symbol)
	{
		StatementWalker walker(*this);
		symbol->accept(walker);
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
		symbol->accept(walker);
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
		symbol->accept(walker);
	}
};

struct SimpleDeclarationWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Declaration* type;
	Declaration* declaration;
	DeclSpecifiers specifiers;
	DeferredSymbols* deferred;

	SimpleDeclarationWalker(const WalkerBase& base, DeferredSymbols* deferred = 0)
		: WalkerBase(base), type(&gCtor), declaration(0), deferred(deferred)
	{
	}


	void visit(cpp::decl_specifier_seq* symbol)
	{
		DeclSpecifierSeqWalker walker(*this);
		symbol->accept(walker);
		type = walker.declaration;
		declaration = type; // if no declarator is specified later, this is probably a class-declaration
		specifiers = walker.specifiers;
	}
	void visit(cpp::type_specifier_seq* symbol)
	{
		DeclSpecifierSeqWalker walker(*this);
		symbol->accept(walker);
		type = walker.declaration;
		declaration = type; // if no declarator is specified later, this is probably a class-declaration
	}

	template<typename T>
	void walkDeclarator(T* symbol)
	{
		DeclaratorWalker walker(*this);
		symbol->accept(walker);
		declaration = pointOfDeclaration(
			walker.enclosing,
			*walker.id,
			type,
#if 0
			walker.paramScope,
#else	
			specifiers.isTypedef ? type->enclosed : walker.paramScope,
#endif
			specifiers,
			enclosing == templateEnclosing); // 3.3.1.1
		if(walker.id != &gAnonymousId)
		{
			walker.id->dec.p = declaration;
		}
		if(declaration->enclosed != 0
			&& !specifiers.isTypedef)
		{
			declaration->enclosed->name = declaration->name;
		}
		if(walker.paramScope != 0)
		{
			pushScope(walker.paramScope); // 3.3.2.1 parameter scope
		}
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
		symbol->accept(walker);
	}
	void visit(cpp::member_declarator_bitfield* symbol)
	{
		if(symbol->id.p != 0)
		{
			declaration = pointOfDeclaration(enclosing, symbol->id->value, type, 0, specifiers); // 3.3.1.1
			symbol->id->value.dec.p = declaration;
		}
	}

	void visit(cpp::initializer* symbol)
	{
		// TODO
	}
	template<typename Walker, typename T>
	void walkDeferable(Walker& walker, T* symbol)
	{
		// TODO: also defer name-lookup for default-arguments and initializers
		if(deferred != 0)
		{
			deferred->push_back(makeDeferredSymbol(walker, symbol));
		}
		else
		{
			symbol->accept(walker);
		}
	}
	void visit(cpp::function_body* symbol)
	{
		FunctionBodyWalker walker(*this);
		walkDeferable(walker, symbol);
	}
	void visit(cpp::ctor_initializer* symbol)
	{
		// TODO
		walkDeferable(*this, symbol);
	}
	void visit(cpp::handler_seq* symbol)
	{
		HandlerSeqWalker walker(*this);
		walkDeferable(*this, symbol);
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

	Declaration* paramType;
	TemplateParameterListWalker(WalkerBase& base)
		: WalkerBase(base), paramType(gTemplateParams)
	{
	}
	void visit(cpp::type_parameter_default* symbol)
	{
		if(symbol->id != 0)
		{
			symbol->id->value.dec.p = pointOfDeclaration(enclosing, symbol->id->value, paramType++, 0, DECLSPEC_TYPEDEF);
		}
	}
	void visit(cpp::type_parameter_template* symbol)
	{
		// TODO
		if(symbol->id != 0)
		{
			symbol->id->value.dec.p = pointOfDeclaration(enclosing, symbol->id->value, paramType++, 0, DECLSPEC_TYPEDEF, true);
		}
	}
	void visit(cpp::parameter_declaration* symbol)
	{
		SimpleDeclarationWalker walker(*this);
		symbol->accept(walker);
	}
};

struct TemplateDeclarationWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Declaration* declaration;
	DeferredSymbols* deferred;
	TemplateDeclarationWalker(WalkerBase& base, DeferredSymbols* deferred = 0)
		: WalkerBase(base), declaration(0), deferred(deferred)
	{
		templateEnclosing = enclosing;
	}
	void visit(cpp::template_parameter_list* symbol)
	{
		// collect template-params into a new scope
		Scope* params = new Scope(makeIdentifier("$params"), SCOPETYPE_TEMPLATE);
		pushScope(params);
		TemplateParameterListWalker walker(*this);
		symbol->accept(walker);
		// move template-param scope onto special stack
		// NOTE: cannot inject template-params into scope of class/function, because template params may be named differently in multiple declarations of same class/function.
		enclosing = params->parent;
		pushTemplateParams(params);
	}
	void visit(cpp::declaration* symbol)
	{
		DeclarationWalker walker(*this);
		symbol->accept(walker);
		declaration = walker.declaration;
		SEMANTIC_ASSERT(declaration != 0);
		if(templateParams != 0) // explicit-specialization has no params
		{
			templateParams->name = declaration->name;
			templateParams->parent = declaration->scope;
		}
	}
	void visit(cpp::member_declaration* symbol)
	{
		MemberDeclarationWalker walker(*this, deferred);
		symbol->accept(walker);
		declaration = walker.declaration;
		SEMANTIC_ASSERT(declaration != 0);
		if(templateParams != 0) // explicit-specialization has no params
		{
			templateParams->name = declaration->name;
			templateParams->parent = declaration->scope;
		}
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
		NamespaceWalker walker(*this, symbol->id != 0 ? symbol->id->value : IDENTIFIER_NULL);
		symbol->accept(walker);
		declaration = walker.declaration;
	}
	void visit(cpp::general_declaration* symbol)
	{
		if(typeid(*symbol->suffix.p) == typeid(cpp::forward_declaration_suffix)
			&& isForwardDeclaration(symbol->spec))
		{
			ForwardDeclarationWalker walker(*this);
			symbol->accept(walker);
			SEMANTIC_ASSERT(walker.declaration != 0);
			declaration = walker.declaration;
		}
		else
		{
			SimpleDeclarationWalker walker(*this);
			symbol->accept(walker);
			SEMANTIC_ASSERT(walker.type != 0);
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
			symbol->accept(walker);
			SEMANTIC_ASSERT(walker.declaration != 0);
			declaration = walker.declaration;
		}
		else
		{
			SimpleDeclarationWalker walker(*this);
			symbol->accept(walker);
			SEMANTIC_ASSERT(walker.type != 0);
			declaration = walker.declaration;
		}
	}
	void visit(cpp::template_declaration* symbol)
	{
		TemplateDeclarationWalker walker(*this);
		symbol->accept(walker);
		declaration = walker.declaration;
	}
	void visit(cpp::explicit_specialization* symbol)
	{
		TemplateDeclarationWalker walker(*this);
		symbol->accept(walker);
		declaration = walker.declaration;
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

	NamespaceWalker(WalkerBase& base, Identifier& id)
		: WalkerBase(base)
	{
		if(id.value != 0)
		{
			declaration = pointOfDeclaration(enclosing, id, &gNamespace, 0);
			id.dec.p = declaration;
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
