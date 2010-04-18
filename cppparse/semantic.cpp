

#include "semantic.h"

#include "cpptree.h"
#include "printer.h"
#include "profiler.h"

#include "parser/symbols.h"

#include <fstream>


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
	bool isDependent;
	TemplateArgument(const Type& type, bool isDependent) : type(type), isDependent(isDependent)
	{
	}
	TemplateArgument(bool isDependent) : type(0), isDependent(isDependent)
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
	bool isValueDependent;

	Declaration(Scope* scope, Identifier name, const Type& type, Scope* enclosed, DeclSpecifiers specifiers = DeclSpecifiers(), bool isTemplate = false, bool isTemplateSpecialization = false, bool isValueDependent = false)
		: scope(scope), name(name), type(type), enclosed(enclosed), overloaded(0), specifiers(specifiers), isTemplate(isTemplate), isTemplateSpecialization(isTemplateSpecialization), isValueDependent(isValueDependent)
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
	Declarations templateParamTypes;

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

bool enclosesEts(ScopeType type)
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
Declaration gStruct(0, makeIdentifier("$struct"), 0, 0);
Declaration gUnion(0, makeIdentifier("$union"), 0, 0);
Declaration gEnum(0, makeIdentifier("$enum"), 0, 0);

// types
Declaration gNamespace(0, makeIdentifier("$namespace"), 0, 0);

Declaration gCtor(0, makeIdentifier("$ctor"), &gSpecial, 0);
Declaration gTypename(0, makeIdentifier("$typename"), &gSpecial, 0);
Declaration gBuiltin(0, makeIdentifier("$builtin"), &gSpecial, 0);

Declaration gDependentType(0, makeIdentifier("$type"), &gSpecial, 0);
Declaration gDependentObject(0, makeIdentifier("$object"), &gBuiltin, 0);
Declaration gDependentTemplate(0, makeIdentifier("$template"), &gSpecial, 0, DeclSpecifiers(), true);
Declaration gDependentNested(0, makeIdentifier("$nested"), &gSpecial, 0);

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

bool isClassKey(const Declaration& declaration)
{
	return &declaration == &gClass
		|| &declaration == &gStruct
		|| &declaration == &gUnion;
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

Scope::Declarations::const_iterator findDeclaration(const Scope::Declarations& declarations, const Declaration* declaration)
{
	ProfileScope profile(gProfileIdentifier);
	Scope::Declarations::const_iterator i = declarations.begin();
	for(; i != declarations.end(); ++i)
	{
		if(&(*i) == declaration)
		{
			break;
		}
	}
	return i;
}

bool isTypeParameter(const Scope& scope, Declaration* declaration)
{
	if(findDeclaration(scope.templateParamTypes, declaration) != scope.templateParamTypes.end())
	{
		return true;
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

// returns true if \p type is an array typedef with a value-dependent size
bool isValueDependent(const Type& type)
{
	if(type.declaration->specifiers.isTypedef)
	{
		return type.declaration->isValueDependent
			|| isValueDependent(type.declaration->type);
	}
	return false;
}

bool isDependent(const Scope& enclosing, const Type& type)
{
	ProfileScope profile(gProfileIdentifier);
	if(isValueDependent(type))
	{
		return true;
	}
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
		if((*i).isDependent
			|| ((*i).type.declaration != 0
				&& isDependent(enclosing, (*i).type)))
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

bool isAnonymous(const Declaration& declaration)
{
	return *declaration.name.value == '$';
}

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
			if(first.isTemplateSpecialization)
			{
				return first; // TODO
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

#ifdef MINGLE
typedef std::list< DeferredParse<struct WalkerBase> > DeferredSymbols;
#else
typedef std::vector<struct DeferredSymbol> DeferredSymbols;
#endif

struct WalkerBase
#ifdef MINGLE
	: public ContextBase
#endif
{
	typedef WalkerBase Base;

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
		ProfileScope profile(gProfileIdentifier);
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
		ProfileScope profile(gProfileIdentifier);
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

	Declaration* pointOfDeclaration(Scope* parent, const Identifier& name, const Type& type, Scope* enclosed, DeclSpecifiers specifiers = DeclSpecifiers(), bool isTemplate = false, bool isTemplateSpecialization = false, bool isValueDependent = false)
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
		
		Declaration other(parent, name, type, enclosed, specifiers, isTemplate, isTemplateSpecialization, isValueDependent);
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

	Declaration* declareClass(Identifier* id, bool isTemplateSpecialisation = false)
	{
		Scope* enclosed = templateParams != 0 ? templateParams : new Scope(makeIdentifier("$class"));
		enclosed->type = SCOPETYPE_CLASS;
		Declaration* declaration = pointOfDeclaration(enclosing, id == 0 ? makeIdentifier(enclosing->getUniqueName()) : *id, &gClass, enclosed, DeclSpecifiers(), enclosing == templateEnclosing, isTemplateSpecialisation);
		if(id != 0)
		{
			id->dec.p = declaration;
		}
		enclosed->name = declaration->name;
		return declaration;
	}

	Declaration* declareObject(Scope* parent, Identifier* id, const Type& type, Scope* enclosed, DeclSpecifiers specifiers, bool isValueDependent)
	{
		Declaration* declaration = pointOfDeclaration(
			parent,
			*id,
			type,
			specifiers.isTypedef ? type.declaration->enclosed : enclosed,
			specifiers,
			enclosing == templateEnclosing,
			false,
			isValueDependent); // 3.3.1.1
		if(id != &gAnonymousId)
		{
			id->dec.p = declaration;
		}
		return declaration;
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

	void addBase(Declaration* declaration, const Type& base)
	{
		SEMANTIC_ASSERT(base.declaration != declaration);
		declaration->enclosed->bases.push_back(base);
	}

	void setQualifying(Declaration* declaration)
	{
#ifndef MINGLE // allow incomplete types as qualifying, while trying possible parse
		if(declaration->enclosed == 0)
		{
			// TODO
			//printPosition(symbol->id->value.position);
			std::cout << "'" << getValue(declaration->name) << "' is incomplete, declared here:" << std::endl;
			printPosition(declaration->name.position);
			throw SemanticError();
		}
#endif
		qualifying = declaration->enclosed;
	}

	void reportIdentifierMismatch(const Identifier& id, Declaration* declaration, const char* expected)
	{
#ifdef MINGLE
		result = 0;
#else
		if(ambiguity != 0)
		{
			throw IdentifierMismatch(id, declaration, expected);
		}
		printIdentifierMismatch(IdentifierMismatch(id, declaration, expected));
		printScope();
		throw SemanticError();
#endif
	}

	Scope* getEtsScope()
	{
		Scope* scope = enclosing;
		for(; !enclosesEts(scope->type); scope = scope->parent)
		{
		}
		return scope;
	}

	void declareEts(Type& type, Identifier* forward)
	{
		if(isClassKey(*type.declaration))
		{
			SEMANTIC_ASSERT(forward != 0);
			/* 3.3.1-6
			if the elaborated-type-specifier is used in the decl-specifier-seq or parameter-declaration-clause of a
			function defined in namespace scope, the identifier is declared as a class-name in the namespace that
			contains the declaration; otherwise, except as a friend declaration, the identifier is declared in the
			smallest non-class, non-function-prototype scope that contains the declaration.
			*/
			Declaration* declaration = pointOfDeclaration(getEtsScope(), *forward, &gClass, 0, DeclSpecifiers(), enclosing == templateEnclosing);
			forward->dec.p = declaration;
			type = declaration;
		}
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

#ifdef MINGLE
#define TREEWALKER_WALK(walker, symbol) SYMBOL_WALK(walker, symbol)
#define TREEWALKER_LEAF(symbol)  SYMBOL_WALK(*this, symbol)
#define TREEWALKER_DEFAULT PARSERCONTEXT_DEFAULT
#else
#define TREEWALKER_WALK(walker, symbol) symbol->accept(walker)
#define TREEWALKER_LEAF(symbol) 
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
		symbol->second = 0; \
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


#ifndef MINGLE

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

#endif

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
		TREEWALKER_LEAF(symbol);
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

struct UncheckedTemplateIdWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Identifier* id;
	TemplateArguments arguments;
	UncheckedTemplateIdWalker(const WalkerBase& base)
		: WalkerBase(base), id(0)
	{
	}
	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF(symbol);
		id = &symbol->value;
	}
	void visit(cpp::template_argument_list* symbol)
	{
		qualifying = 0;
		// TODO: store args
		TemplateArgumentListWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		arguments = walker.arguments;
	}
};

struct UnqualifiedIdWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Declaration* declaration;
	Identifier* id;
	bool isIdentifier;
	UnqualifiedIdWalker(const WalkerBase& base)
		: WalkerBase(base), declaration(0), id(0), isIdentifier(false)
	{
	}
	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF(symbol);
		id = &symbol->value;
		isIdentifier = true;
		if(qualifying != &context.dependent)
		{
			declaration = findDeclaration(*id);
		}
	}
	void visit(cpp::simple_template_id* symbol)
	{
		UncheckedTemplateIdWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		id = walker.id;
		if(qualifying != &context.dependent)
		{
			declaration = findDeclaration(*id);
			if(declaration == &gUndeclared
				|| !isTemplateName(*declaration))
			{
				reportIdentifierMismatch(*id, declaration, "template-name");
			}
		}
	}
	void visit(cpp::template_id_operator_function* symbol)
	{
		UncheckedTemplateIdWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::destructor_id* symbol)
	{
		// TODO: can destructor-id be dependent?
		TREEWALKER_WALK(*this, symbol);
	}
};

struct QualifiedIdWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Declaration* declaration;
	Identifier* id;
	QualifiedIdWalker(const WalkerBase& base)
		: WalkerBase(base), declaration(0), id(0)
	{
	}

	void visit(cpp::terminal<boost::wave::T_COLON_COLON> symbol)
	{
		if(symbol.value != 0)
		{
			qualifying = &context.global;
		}
	}
	void visit(cpp::nested_name_specifier* symbol)
	{
		NestedNameSpecifierWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		qualifying = walker.qualifying;
	}
	void visit(cpp::unqualified_id* symbol)
	{
		UnqualifiedIdWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
		id = walker.id;
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
	Identifier* id;
	bool isIdentifier;
	IdExpressionWalker(const WalkerBase& base)
		: WalkerBase(base), declaration(0), id(0), isIdentifier(false)
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
		isIdentifier = walker.isIdentifier;
	}
};

struct ExplicitTypeExpressionWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	bool isTypeDependent;
	bool isValueDependent;
	ExplicitTypeExpressionWalker(const WalkerBase& base)
		: WalkerBase(base), isTypeDependent(false), isValueDependent(false)
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
		isTypeDependent = isDependent(walker.type) | walker.isValueDependent;
	}
	void visit(cpp::new_type* symbol)
	{
		TypeIdWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		isTypeDependent = isDependent(walker.type) | walker.isValueDependent;
	}
	void visit(cpp::assignment_expression* symbol)
	{
		ExpressionWalker walker(*this);
		walker.isTypeDependent = false;
		TREEWALKER_WALK(walker, symbol);
		isValueDependent |= walker.isValueDependent;
	}
	void visit(cpp::cast_expression* symbol)
	{
		ExpressionWalker walker(*this);
		walker.isTypeDependent = false;
		TREEWALKER_WALK(walker, symbol);
		isValueDependent |= walker.isValueDependent;
	}
};

struct DependentPrimaryExpressionWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Declaration* declaration;
	Identifier* id;
	bool isTypeDependent;
	DependentPrimaryExpressionWalker(const WalkerBase& base)
		: WalkerBase(base), declaration(0), id(0), isTypeDependent(false)
	{
	}
	void visit(cpp::id_expression* symbol)
	{
		/* temp.dep.expr
		An id-expression is type-dependent if it contains:
		— an identifier that was declared with a dependent type,
		— a template-id that is dependent,
		— a conversion-function-id that specifies a dependent type,
		— a nested-name-specifier or a qualified-id that names a member of an unknown specialization.
		*/
		IdExpressionWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
		if(walker.isIdentifier // expression is 'identifier'
			&& (declaration == &gUndeclared // identifier was not previously declared
				|| (isObject(*declaration) && declaration->scope->type == SCOPETYPE_NAMESPACE))) // identifier was previously declared in namespace-scope
		{
			// defer name-lookup: this identifier may be a dependent-name.
			id = walker.id;
		}
		else
		{
			if(declaration != 0)
			{
				if(declaration == &gUndeclared
					|| !isObject(*declaration))
				{
					reportIdentifierMismatch(*walker.id, declaration, "object-name");
				}
				else
				{
					walker.id->dec.p = declaration;
					isTypeDependent |= isDependent(declaration->type);
				}
			}
			else if(walker.id != 0)
			{
				walker.id->dec.p = &gDependentObject;
				isTypeDependent = true;
			}
		}
	}
	void visit(cpp::primary_expression_parenthesis* symbol)
	{
		ExpressionWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		isTypeDependent |= walker.isTypeDependent;
	}
};

struct DependentPostfixExpressionWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Declaration* declaration;
	Identifier* id;
	bool isTypeDependent;
	DependentPostfixExpressionWalker(const WalkerBase& base)
		: WalkerBase(base), declaration(0), id(0), isTypeDependent(false)
	{
	}
	void visit(cpp::primary_expression* symbol)
	{
		DependentPrimaryExpressionWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
		id = walker.id;
		isTypeDependent |= walker.isTypeDependent;
	}
	void visit(cpp::postfix_expression_call* symbol)
	{
		ExpressionWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		isTypeDependent |= walker.isTypeDependent;
		if(id != 0)
		{
			if(!isTypeDependent)
			{
				if(declaration != 0)
				{
					if(declaration == &gUndeclared
						|| !isObject(*declaration))
					{
						reportIdentifierMismatch(*id, declaration, "object-name");
					}
					else
					{
						id->dec.p = declaration;
						isTypeDependent |= isDependent(declaration->type);
					}
				}
			}
			else
			{
				id->dec.p = &gDependentObject;
			}
		}
	}
};

struct ExpressionWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	/* 14.6.2.2-1
	...an expression is type-dependent if any subexpression is type-dependent.
	*/
	bool isTypeDependent;
	bool isValueDependent;
	ExpressionWalker(const WalkerBase& base)
		: WalkerBase(base), isTypeDependent(false), isValueDependent(false)
	{
	}
	void visit(cpp::postfix_expression_member* symbol)
	{
		IdExpressionWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		// TODO: name-lookup for member id-expression
		// TODO: inherit type-dependent property
	}
	void visit(cpp::primary_expression_builtin* symbol)
	{
		TREEWALKER_LEAF(symbol);
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
	/* temp.dep.constexpr
	An identifier is value-dependent if it is:
	— a name declared with a dependent type,
	— the name of a non-type template parameter,
	— a constant with integral or enumeration type and is initialized with an expression that is value-dependent.
	*/
	void visit(cpp::id_expression* symbol)
	{
		IdExpressionWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		Declaration* declaration = walker.declaration;
		if(declaration != 0)
		{
			if(declaration == &gUndeclared
				|| !isObject(*declaration))
			{
				reportIdentifierMismatch(*walker.id, declaration, "object-name");
			}
			else
			{
				walker.id->dec.p = declaration;
				isTypeDependent |= isDependent(declaration->type);
				isValueDependent |= isDependent(declaration->type) || declaration->isValueDependent;
			}
		}
		else if(walker.qualifying == &context.dependent)
		{
			walker.id->dec.p = &gDependentObject;
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
		DependentPostfixExpressionWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		isTypeDependent = walker.isTypeDependent;
	}
	/* 14.6.2.2-3
	Expressions of the following forms are type-dependent only if the type specified by the type-id, simple-type-specifier
	or new-type-id is dependent, even if any subexpression is type-dependent:
	- postfix-expression-construct
	- new-expression
	- postfix-expression-cast
	- cast-expression
	*/
	/* temp.dep.constexpr
	Expressions of the following form are value-dependent if either the type-id or simple-type-specifier is dependent or the
	expression or cast-expression is value-dependent:
	simple-type-specifier ( expression-listopt )
	static_cast < type-id > ( expression )
	const_cast < type-id > ( expression )
	reinterpret_cast < type-id > ( expression )
	( type-id ) cast-expression
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
		isValueDependent = symbol->op->id != cpp::cast_operator::DYNAMIC
			? walker.isTypeDependent | walker.isValueDependent
			: walker.isValueDependent;
	}
	void visit(cpp::cast_expression_default* symbol)
	{
		ExplicitTypeExpressionWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		isTypeDependent = walker.isTypeDependent;
		isValueDependent = walker.isTypeDependent | walker.isValueDependent;
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
	/* temp.dep.constexpr
	Expressions of the following form are value-dependent if the unary-expression is type-dependent or the type-id is dependent
	(even if sizeof unary-expression and sizeof ( type-id ) are not type-dependent):
	sizeof unary-expression
	sizeof ( type-id )
	*/
	void visit(cpp::unary_expression_sizeof* symbol)
	{
		ExpressionWalker walker(*this);
		walker.isTypeDependent = false;
		TREEWALKER_WALK(walker, symbol);
		isValueDependent |= walker.isTypeDependent;
	}
	void visit(cpp::unary_expression_sizeoftype* symbol)
	{
		TypeIdWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		isValueDependent |= isDependent(walker.type) | walker.isValueDependent;
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

	TemplateArgumentListWalker(const WalkerBase& base)
		: WalkerBase(base)
	{
	}
	void visit(cpp::type_id* symbol)
	{
		TypeIdWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		arguments.push_back(TemplateArgument(walker.type, walker.isValueDependent));
	}
	void visit(cpp::assignment_expression* symbol)
	{
		ExpressionWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		arguments.push_back(TemplateArgument(walker.isTypeDependent | walker.isValueDependent)); // todo: evaluate constant-expression (unless it's dependent expression)
	}
};

struct TemplateIdWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Type type;
	LookupFilter filter;
	bool isTypename;
	TemplateIdWalker(const WalkerBase& base, LookupFilter filter = isAny, bool isTypename = false)
		: WalkerBase(base), type(0), filter(filter), isTypename(isTypename)
	{
	}
	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF(symbol);
		if(qualifying != &context.dependent)
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
				type.declaration = declaration;
			}
		}
		else if(!isTypename)
		{
			reportIdentifierMismatch(symbol->value, &gUndeclared, "typename");
		}
		else
		{
			symbol->value.dec.p = &gDependentTemplate;
		}
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
	bool isTypename;
	TypeNameWalker(const WalkerBase& base, LookupFilter filter = isAny, bool isTypename = false)
		: WalkerBase(base), type(0), filter(filter), isTypename(isTypename)
	{
	}
	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF(symbol);
		if(qualifying != &context.dependent)
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
				type.declaration = declaration;
			}
		}
		else if(!isTypename)
		{
			reportIdentifierMismatch(symbol->value, &gUndeclared, "typename");
		}
		else
		{
			symbol->value.dec.p = &gDependentType;
		}
	}
	void visit(cpp::simple_template_id* symbol)
	{
		TemplateIdWalker walker(*this, filter, isTypename);
		TREEWALKER_WALK(walker, symbol);
		type = walker.type;
	}
};

struct NestedNameSpecifierSuffixWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	bool allowDependent;
	NestedNameSpecifierSuffixWalker(const WalkerBase& base, bool allowDependent = false)
		: WalkerBase(base), allowDependent(allowDependent)
	{
	}
	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF(symbol);
		if(qualifying != &context.dependent)
		{
			Declaration* declaration = findDeclaration(qualifying->declarations, symbol->value, isNestedName);
			if(declaration == 0)
			{
				reportIdentifierMismatch(symbol->value, &gUndeclared, "nested-name");
			}
			else
			{
				symbol->value.dec.p = declaration;
				setQualifying(declaration);
			}
		}
		else
		{
			symbol->value.dec.p = &gDependentNested;
		}
	}
	void visit(cpp::simple_template_id* symbol)
	{
		UncheckedTemplateIdWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		if(qualifying != &context.dependent)
		{
			Type type = findDeclaration(qualifying->declarations, *walker.id, isNestedName);
			type.arguments.swap(walker.arguments);
			if(type.declaration == 0)
			{
				reportIdentifierMismatch(*walker.id, &gUndeclared, "nested-name");
			}
			else if(!allowDependent
				&& isDependent(type))
			{
				qualifying = &context.dependent;
			}
			else
			{
				setQualifying(type.declaration);
			}
		}
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
	void visit(cpp::nested_name_specifier_prefix* symbol)
	{
		NestedNameSpecifierWalker walker(*this, allowDependent);
		TREEWALKER_WALK(walker, symbol);
		qualifying = walker.qualifying;
	}
	void visit(cpp::nested_name_specifier_suffix* symbol)
	{
		NestedNameSpecifierSuffixWalker walker(*this, allowDependent);
		TREEWALKER_WALK(walker, symbol);
		qualifying = walker.qualifying;
	}
	void visit(cpp::namespace_name* symbol)
	{
		NamespaceNameWalker walker(*this, isNestedName);
		TREEWALKER_WALK(walker, symbol);
		setQualifying(walker.declaration);
	}
	void visit(cpp::type_name* symbol)
	{
		TypeNameWalker walker(*this, isNestedName, true);
		TREEWALKER_WALK(walker, symbol);
		if(qualifying != &context.dependent)
		{
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
		TemplateIdWalker walker(*this, isNestedName, true);
		TREEWALKER_WALK(walker, symbol);
		if(qualifying != &context.dependent)
		{
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

	void visit(cpp::simple_type_specifier_name* symbol)
	{
		TypeSpecifierWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		type = walker.type;
	}
	void visit(cpp::simple_type_specifier_template* symbol)
	{
		TypeSpecifierWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		type = walker.type;
	}
	void visit(cpp::type_name* symbol)
	{
		TypeNameWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		SEMANTIC_ASSERT(walker.type.declaration != 0);
		type = walker.type;
	}
	void visit(cpp::terminal<boost::wave::T_COLON_COLON> symbol)
	{
		if(symbol.value != 0)
		{
			qualifying = &context.global;
		}
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
		TREEWALKER_LEAF(symbol);
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

	void visit(cpp::terminal<boost::wave::T_COLON_COLON> symbol)
	{
		if(symbol.value != 0)
		{
			qualifying = &context.global;
		}
	}
	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF(symbol);
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
		UncheckedTemplateIdWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		id = walker.id;
	}
	void visit(cpp::operator_function_id* symbol) 
	{
		TREEWALKER_LEAF(symbol);
		// TODO
		id = &gOperatorFunctionId;
	}
	void visit(cpp::conversion_function_id* symbol) 
	{
		TypeIdWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		// TODO
		id = &gConversionFunctionId;
	}
	void visit(cpp::destructor_id* symbol) 
	{
		TREEWALKER_LEAF(symbol);
		id = &symbol->name->value;
	}
	void visit(cpp::template_id_operator_function* symbol) 
	{
		TREEWALKER_LEAF(symbol);
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
	}

	void visit(cpp::parameter_declaration* symbol)
	{
		SimpleDeclarationWalker walker(*this, true);
		TREEWALKER_WALK(walker, symbol);
		walker.commit();
	}
};

struct ExceptionSpecificationWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	ExceptionSpecificationWalker(const WalkerBase& base)
		: WalkerBase(base)
	{
	}
	void visit(cpp::type_id* symbol)
	{
		TypeIdWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
	}
};

struct DeclareEts
{
	WalkerBase& context;
	DeclareEts(WalkerBase& context) : context(context)
	{
	}
	void operator()(Identifier& id) const
	{
		// todo: record ETS declaration
		Declaration* declaration = context.pointOfDeclaration(context.getEtsScope(), id, &gClass, 0, DeclSpecifiers(), context.enclosing == context.templateEnclosing);
		id.dec.p = declaration;
	}
};

struct PtrOperatorWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	PtrOperatorWalker(const WalkerBase& base)
		: WalkerBase(base)
	{
	}
	void visit(cpp::nested_name_specifier* symbol)
	{
		NestedNameSpecifierWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		qualifying = walker.qualifying;
	}
};

struct DeclaratorWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Identifier* id;
	Scope* paramScope;
	bool isValueDependent;
	DeclaratorWalker(const WalkerBase& base)
		: WalkerBase(base), id(&gAnonymousId), paramScope(0), isValueDependent(false)
	{
	}

	void visit(cpp::ptr_operator* symbol)
	{
		PtrOperatorWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
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
		// hack to resolve issue: when mingled with parser, deferred parse causes constructor of ParameterDeclarationClauseWalker to be invoked twice
		WalkerBase base(*this);
		base.pushScope(new Scope(makeIdentifier("$declarator")));
		base.enclosing->type = SCOPETYPE_PROTOTYPE;
		base.templateParams = 0;
		if(templateParams != 0)
		{
			base.enclosing->declarations = templateParams->declarations;
			for(Scope::Declarations::iterator i = base.enclosing->declarations.begin(); i != base.enclosing->declarations.end(); ++i)
			{
				if(::findDeclaration(templateParams->templateParamTypes, (*i).type.declaration) != templateParams->templateParamTypes.end())
				{
					base.enclosing->templateParamTypes.push_back(gParam);
					(*i).type = &base.enclosing->templateParamTypes.back();
				}
			}
		}
		ParameterDeclarationClauseWalker walker(base);
#if 0//def MINGLE
		if(WalkerBase::deferred != 0)
		{
			defer(*WalkerBase::deferred, walker, makeSkipParenthesised(DeclareEts(walker)), symbol);
			result = symbol; // always succeeds!
		}
		else
#endif
		{
			TREEWALKER_WALK(walker, symbol);
		}
		paramScope = walker.enclosing; // store reference for later resumption
	}
	void visit(cpp::exception_specification* symbol)
	{
		ExceptionSpecificationWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::constant_expression* symbol)
	{
		ExpressionWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		isValueDependent |= walker.isValueDependent;
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
		TypeNameWalker walker(*this, isTypeName, true);
		TREEWALKER_WALK(walker, symbol);
		type = walker.type;
	}
};

struct ClassHeadWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Declaration* declaration;
	Identifier* id;
	bool isUnion;
	bool isTemplateSpecialisation;
	ClassHeadWalker(const WalkerBase& base)
		: WalkerBase(base), declaration(0), id(0), isUnion(false), isTemplateSpecialisation(false)
	{
	}

	void visit(cpp::class_key* symbol)
	{
		TREEWALKER_WALK(*this, symbol);
		isUnion = symbol->id == cpp::class_key::UNION;
	}
	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF(symbol);
		id = &symbol->value;
	}
	void visit(cpp::nested_name_specifier* symbol)
	{
		NestedNameSpecifierWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		qualifying = walker.qualifying;
	}
	void visit(cpp::simple_template_id* symbol) 
	{
		UncheckedTemplateIdWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		// TODO: don't declare anything - this is a template (partial) specialisation
		id = walker.id;
		isTemplateSpecialisation = true;
	}
	void visit(cpp::terminal<boost::wave::T_COLON> symbol)
	{
		// defer class declaration until we know this is a class-specifier - it may be an elaborated-type-specifier until ':' is discovered
		// 3.3.1.3 The point of declaration for a class first declared by a class-specifier is immediately after the identifier or simple-template-id (if any) in its class-head
		declaration = declareClass(id, isTemplateSpecialisation);
	}
	void visit(cpp::base_specifier* symbol) 
	{
		BaseSpecifierWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		if(walker.type.declaration != 0) // declaration == 0 if base-class is dependent
		{
			SEMANTIC_ASSERT(declaration->enclosed != 0);
			addBase(declaration, getOriginalType(walker.type));
		}
	}
};

struct UsingDeclarationWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	bool isTypename;
	UsingDeclarationWalker(const WalkerBase& base)
		: WalkerBase(base), isTypename(false)
	{
	}
	void visit(cpp::terminal<boost::wave::T_COLON_COLON> symbol)
	{
		if(symbol.value != 0)
		{
			qualifying = &context.global;
		}
	}
	void visit(cpp::using_declaration_global* symbol)
	{
		qualifying = &context.global;
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
		UnqualifiedIdWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		if(!isTypename)
		{
			Declaration* declaration = walker.declaration;
			if(declaration == &gUndeclared
				|| !(isObject(*declaration) || isTypeName(*declaration)))
			{
				reportIdentifierMismatch(*walker.id, declaration, "object-name or type-name");
			}
			else
			{
				walker.id->dec.p = declaration;
			}
		}
		else
		{
			walker.id->dec.p = &gDependentType;
		}
	}
	void visit(cpp::terminal<boost::wave::T_TYPENAME> symbol)
	{
		isTypename = true;
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
		SimpleDeclarationWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		SEMANTIC_ASSERT(walker.type.declaration != 0);
		declaration = walker.declaration;
	}
	void visit(cpp::member_declaration_nested* symbol)
	{
		QualifiedIdWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
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
	Identifier* id;
	DeferredSymbols deferred;
	bool isUnion;
	bool isTemplateSpecialisation;
	ClassSpecifierWalker(const WalkerBase& base)
		: WalkerBase(base), declaration(0), id(0), isUnion(false), isTemplateSpecialisation(false)
	{
	}
	void walkDeferred()
	{
#ifdef MINGLE
		parseDeferred(deferred, *parser);
#else
		for(DeferredSymbols::const_iterator i = deferred.begin(); i != deferred.end(); ++i)
		{
			(*i).func((*i).context, (*i).symbol);
		}
#endif
	};

	void visit(cpp::class_head* symbol)
	{
		ClassHeadWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
		id = walker.id;
		isUnion = walker.isUnion;
		isTemplateSpecialisation = walker.isTemplateSpecialisation;
	}
	void visit(cpp::terminal<boost::wave::T_LEFTBRACE> symbol)
	{
		// defer class declaration until we know this is a class-specifier - it may be an elaborated-type-specifier until '{' is discovered
		if(declaration == 0)
		{
			// 3.3.1.3 The point of declaration for a class first declared by a class-specifier is immediately after the identifier or simple-template-id (if any) in its class-head
			declaration = declareClass(id, isTemplateSpecialisation);
		}

		/* basic.scope.class-1
		The potential scope of a name declared in a class consists not only of the declarative region following
		the name’s point of declaration, but also of all function bodies, brace-or-equal-initializers of non-static
		data members, and default arguments in that class (including such things in nested classes).
		*/
		if(declaration->enclosed != 0)
		{
			pushScope(declaration->enclosed);
		}
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

struct EnumeratorDefinitionWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Declaration* declaration;
	EnumeratorDefinitionWalker(const WalkerBase& base)
		: WalkerBase(base), declaration(0)
	{
	}

	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF(symbol);
		/* 3.1-4
		The point of declaration for an enumerator is immediately after its enumerator-definition.
		*/
		// TODO: give enumerators a type
		declaration = pointOfDeclaration(enclosing, symbol->value, &gBuiltin, 0, DeclSpecifiers());
		symbol->value.dec.p = declaration;
	}
	void visit(cpp::constant_expression* symbol)
	{
		ExpressionWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		declaration->isValueDependent |= walker.isValueDependent;
	}
};

struct EnumSpecifierWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Declaration* declaration;
	Identifier* id;
	EnumSpecifierWalker(const WalkerBase& base)
		: WalkerBase(base), declaration(0), id(0)
	{
	}

	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF(symbol);
		id = &symbol->value;
	}

	void visit(cpp::terminal<boost::wave::T_LEFTBRACE> symbol)
	{
		// defer declaration until '{' resolves ambiguity between enum-specifier and elaborated-type-specifier
		if(id != 0)
		{
			declaration = pointOfDeclaration(enclosing, *id, &gEnum, 0);
			id->dec.p = declaration;
		}
	}

	void visit(cpp::enumerator_definition* symbol)
	{
		if(declaration == 0)
		{
			// unnamed enum
			declaration = pointOfDeclaration(enclosing, makeIdentifier(enclosing->getUniqueName()), &gEnum, 0);
		}
		EnumeratorDefinitionWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
	}
};

struct ElaboratedTypeSpecifierWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Declaration* key;
	Type type;
	Identifier* id;
	ElaboratedTypeSpecifierWalker(const WalkerBase& base)
		: WalkerBase(base), key(0), type(0), id(0)
	{
	}
	void visit(cpp::terminal<boost::wave::T_COLON_COLON> symbol)
	{
		if(symbol.value != 0)
		{
			qualifying = &context.global;
		}
	}
	void visit(cpp::elaborated_type_specifier_default* symbol)
	{
		TREEWALKER_WALK(*this, symbol);
		if(!isUnqualified(symbol)
			|| !isClassKey(*type.declaration))
		{
			id = 0;
		}
	}
	void visit(cpp::nested_name_specifier* symbol)
	{
		NestedNameSpecifierWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		qualifying = walker.qualifying;
	}
	void visit(cpp::class_key* symbol)
	{
		TREEWALKER_LEAF(symbol);
		key = &gClass;
	}
	void visit(cpp::enum_key* symbol)
	{
		TREEWALKER_LEAF(symbol);
		key = &gEnum;
	}
	void visit(cpp::simple_template_id* symbol)
	{
		SEMANTIC_ASSERT(key == &gClass);
		TemplateIdWalker walker(*this, isType);
		TREEWALKER_WALK(walker, symbol);
		type = walker.type;
	}
	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF(symbol);
		/* 3.4.4-2
		If the elaborated-type-specifier has no nested-name-specifier ...
		... the identifier is looked up according to 3.4.1 but ignoring any non-type names that have been declared. If
		the elaborated-type-specifier is introduced by the enum keyword and this lookup does not find a previously
		declared type-name, the elaborated-type-specifier is ill-formed. If the elaborated-type-specifier is introduced by
		the class-key and this lookup does not find a previously declared type-name ...
		the elaborated-type-specifier is a declaration that introduces the class-name as described in 3.3.1.
		*/
		id = &symbol->value;
		Declaration* declaration = findDeclaration(symbol->value, isType);
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
			if(declaration->type.declaration != key)
			{
				printPosition(symbol->value.position);
				std::cout << "'" << symbol->value.value << "': elaborated-type-specifier key does not match declaration" << std::endl;
				printPosition(declaration->name.position);
				throw SemanticError();
			}
			type = declaration;
		}
		else
		{
			/* 3.4.4-2
			... If the elaborated-type-specifier is introduced by the enum keyword and this lookup does not find a previously
			declared type-name, the elaborated-type-specifier is ill-formed. If the elaborated-type-specifier is introduced by
			the class-key and this lookup does not find a previously declared type-name ...
			the elaborated-type-specifier is a declaration that introduces the class-name as described in 3.3.1.
			*/
			if(key != &gClass)
			{
				SEMANTIC_ASSERT(key == &gEnum);
				printPosition(symbol->value.position);
				std::cout << "'" << symbol->value.value << "': elaborated-type-specifier refers to undefined enum" << std::endl;
				throw SemanticError();
			}
			type = key;
		}
	}
};

struct TypenameSpecifierWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	TypenameSpecifierWalker(const WalkerBase& base)
		: WalkerBase(base)
	{
	}

	void visit(cpp::terminal<boost::wave::T_COLON_COLON> symbol)
	{
		if(symbol.value != 0)
		{
			qualifying = &context.global;
		}
	}
	void visit(cpp::terminal<boost::wave::T_TEMPLATE> symbol)
	{
		// TODO
	}
	void visit(cpp::nested_name_specifier* symbol)
	{
		NestedNameSpecifierWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		qualifying = walker.qualifying;
	}
	void visit(cpp::type_name* symbol)
	{
		TypeNameWalker walker(*this, isAny, true);
		TREEWALKER_WALK(walker, symbol);
	}
};

struct DeclSpecifierSeqWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Type type;
	DeclSpecifiers specifiers;
	Identifier* forward;
	bool isUnion;
	bool isTemplateParameter;
	DeclSpecifierSeqWalker(const WalkerBase& base, bool isTemplateParameter = false)
		: WalkerBase(base), type(0), forward(0), isUnion(false), isTemplateParameter(isTemplateParameter)
	{
	}

	void visit(cpp::simple_type_specifier* symbol)
	{
		TypeSpecifierWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		type = walker.type;
	}
	void visit(cpp::elaborated_type_specifier* symbol)
	{
#ifdef MINGLE
		if(isTemplateParameter)
		{
			result = 0;
			return;
		}
#endif
		ElaboratedTypeSpecifierWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		forward = walker.id;
		type = walker.type;
	}
	void visit(cpp::typename_specifier* symbol)
	{
		TypenameSpecifierWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		type = &gTypename;
	}
	void visit(cpp::class_specifier* symbol)
	{
		ClassSpecifierWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		type = walker.declaration;
		walker.walkDeferred();
		isUnion = walker.isUnion;
	}
	void visit(cpp::enum_specifier* symbol)
	{
		EnumSpecifierWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		type = walker.declaration;
	}
	void visit(cpp::decl_specifier_default* symbol)
	{
		TREEWALKER_LEAF(symbol);
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
		TREEWALKER_LEAF(symbol);
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
		SimpleDeclarationWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		SEMANTIC_ASSERT(walker.type.declaration != 0);
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
		TREEWALKER_LEAF(symbol);
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
	void visit(cpp::terminal<boost::wave::T_COLON_COLON> symbol)
	{
		if(symbol.value != 0)
		{
			qualifying = &context.global;
		}
	}
	void visit(cpp::nested_name_specifier* symbol)
	{
		NestedNameSpecifierWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		qualifying = walker.qualifying;
	}
	void visit(cpp::class_name* symbol)
	{
		TypeNameWalker walker(*this, isAny, true);
		TREEWALKER_WALK(walker, symbol);
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
		QualifiedTypeNameWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF(symbol);
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

struct MemberDeclaratorBitfieldWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Identifier* id;
	MemberDeclaratorBitfieldWalker(const WalkerBase& base)
		: WalkerBase(base), id(0)
	{
	}
	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF(symbol);
		id = &symbol->value;
	}
	void visit(cpp::constant_expression* symbol)
	{
		ExpressionWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
	}
};

struct TypeIdWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Type type;
	bool isValueDependent;
	TypeIdWalker(const WalkerBase& base)
		: WalkerBase(base), type(0), isValueDependent(false)
	{
	}
	void visit(cpp::type_specifier_seq* symbol)
	{
		DeclSpecifierSeqWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		type = walker.type;
		declareEts(type, walker.forward);
	}
	void visit(cpp::abstract_declarator* symbol)
	{
		DeclaratorWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		isValueDependent |= walker.isValueDependent;
	}
};

struct IsTemplateName
{
	WalkerBase& context;
	IsTemplateName(WalkerBase& context) : context(context)
	{
	}
	bool operator()(Identifier& id) const
	{
		Declaration* declaration = context.findDeclaration(id);
		return declaration != &gUndeclared && isTemplateName(*declaration);
	}
};


struct SimpleDeclarationWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Declaration* declaration;
	Scope* parent;
	Identifier* id;
	Type type;
	Scope* enclosed;
	DeclSpecifiers specifiers;
	Identifier* forward;
#ifdef MINGLE
	DeferredSymbols deferred;
#endif
	bool isParameter;
	bool isTemplateParameter;
	bool isUnion;
	bool isValueDependent;

	SimpleDeclarationWalker(const WalkerBase& base, bool isParameter = false, bool isTemplateParameter = false) : WalkerBase(base),
		declaration(0),
		parent(0),
		id(0),
		type(&gCtor),
		enclosed(0),
		forward(0),
		isParameter(isParameter),
		isTemplateParameter(isTemplateParameter),
		isUnion(false),
		isValueDependent(false)
	{
	}

	void commit()
	{
		if(id != 0)
		{
			declaration = declareObject(parent, id, type, enclosed, specifiers, isValueDependent);

			enclosing = parent;

			if(enclosed != 0)
			{
				enclosed->name = declaration->name;
				enclosing = enclosed; // 3.3.2.1 parameter scope
			}
			templateParams = 0;

			id = 0;
		}
#ifdef MINGLE
		if(WalkerBase::deferred != 0
			&& !deferred.empty())
		{
			WalkerBase::deferred->splice(WalkerBase::deferred->end(), deferred);
		}
#endif
	}

	void visit(cpp::decl_specifier_seq* symbol)
	{
		DeclSpecifierSeqWalker walker(*this, isTemplateParameter);
		TREEWALKER_WALK(walker, symbol);
		type = walker.type;
		declaration = type.declaration; // if no declarator is specified later, this is probably a class-declaration
		specifiers = walker.specifiers;
		forward = walker.forward;
		isUnion = walker.isUnion;
	}
	void visit(cpp::type_specifier_seq* symbol)
	{
		DeclSpecifierSeqWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		type = walker.type;
		declaration = type.declaration; // if no declarator is specified later, this is probably a class-declaration
		forward = walker.forward;
		isUnion = walker.isUnion;
	}

#ifdef MINGLE
	void visit(cpp::init_declarator_disambiguate* symbol)
	{
		result = 0;
	}
#endif

	template<typename T>
	void walkDeclarator(T* symbol)
	{
		DeclaratorWalker walker(*this);
#ifdef MINGLE
		if(WalkerBase::deferred != 0)
		{
			walker.deferred = &deferred;
		}
#endif
		TREEWALKER_WALK(walker, symbol);
		parent = walker.enclosing;
		id = walker.id;
		enclosed = walker.paramScope;
		isValueDependent = isTemplateParameter | walker.isValueDependent;
	}
	void visit(cpp::declarator* symbol)
	{
		if(isParameter)
		{
			declareEts(type, forward);
		}
		walkDeclarator(symbol);
	}
	void visit(cpp::declarator_disambiguate* symbol)
	{
		if(isParameter)
		{
			declareEts(type, forward);
		}
		walkDeclarator(symbol);
	}
	void visit(cpp::abstract_declarator* symbol)
	{
		if(isParameter)
		{
			declareEts(type, forward);
		}
		DeclaratorWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::member_declarator_bitfield* symbol)
	{
		MemberDeclaratorBitfieldWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		if(walker.id != 0)
		{
			declaration = pointOfDeclaration(enclosing, *walker.id, type, 0, specifiers); // 3.3.1.1
			walker.id->dec.p = declaration;
		}
	}
	void visit(cpp::terminal<boost::wave::T_ASSIGN> symbol)
	{
		if(symbol.value != 0)
		{
			commit();
		}
	}
	void visit(cpp::terminal<boost::wave::T_TRY> symbol)
	{
		if(symbol.value != 0)
		{
			commit();
		}
	}
	void visit(cpp::terminal<boost::wave::T_LEFTBRACE> symbol)
	{
		if(symbol.value != 0)
		{
			commit();
		}
	}
	void visit(cpp::terminal<boost::wave::T_COMMA> symbol)
	{
		if(symbol.value != 0)
		{
			commit();
		}
	}
	void visit(cpp::terminal<boost::wave::T_SEMICOLON> symbol)
	{
		if(symbol.value != 0)
		{
			commit();
		}
	}
	void visit(cpp::terminal<boost::wave::T_COLON> symbol)
	{
		if(symbol.value != 0)
		{
			commit();
		}
	}

	// handle assignment-expression(s) in initializer
	void visit(cpp::default_parameter* symbol)
	{
#ifdef MINGLE
		// todo: this fails if default-argument contains a template-name that is declared later in the class.
		// Comeau fails in this case too..
		if(WalkerBase::deferred != 0)
		{
			result = defer(*WalkerBase::deferred, *this, makeSkipDefaultArgument(IsTemplateName(*this)), symbol);
		}
		else
#endif
		{
			walkDeferable(*this, symbol);
		}
	}
	// handle assignment-expression(s) in initializer
	void visit(cpp::assignment_expression* symbol)
	{
		ExpressionWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::constant_expression* symbol)
	{
		ExpressionWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		SEMANTIC_ASSERT(declaration != 0);
		declaration->isValueDependent |= walker.isValueDependent;
	}
	template<typename Walker, typename T>
	void walkDeferable(Walker& walker, T* symbol)
	{
#ifndef MINGLE
		// defer name-lookup for function-body, default-arguments and ctor-initializers
		if(deferred != 0)
		{
			deferred->push_back(makeDeferredSymbol(walker, symbol));
		}
		else
#endif
		{
			TREEWALKER_WALK(walker, symbol);
		}
	}
	void visit(cpp::statement_seq* symbol)
	{
#if 0
		ParserOpaque* parser = 0;
		TokenBuffer buffer;
		parser->lexer.history.swap(buffer);
		skipBraced(*parser);
#endif
		CompoundStatementWalker walker(*this);
#ifdef MINGLE
		if(WalkerBase::deferred != 0)
		{
			result = defer(deferred, walker, skipBraced, symbol);
		}
		else
#endif
		{
			walkDeferable(walker, symbol);
		}
	}
	void visit(cpp::mem_initializer* symbol)
	{
		MemInitializerWalker walker(*this);
		walkDeferable(walker, symbol);
	}
	void visit(cpp::handler_seq* symbol)
	{
		HandlerSeqWalker walker(*this);
		walkDeferable(walker, symbol);
	}
#ifdef MINGLE
	void visit(cpp::mem_initializer_clause* symbol)
	{
		result = defer(deferred, *this, skipMemInitializerClause, symbol);
	}
#endif

	void visit(cpp::simple_declaration_named* symbol)
	{
		declareEts(type, forward);
		TREEWALKER_WALK(*this, symbol);
	}
	void visit(cpp::member_declaration_named* symbol)
	{
		declareEts(type, forward);
		TREEWALKER_WALK(*this, symbol);
	}
	void visit(cpp::function_definition* symbol)
	{
		declareEts(type, forward);
		TREEWALKER_WALK(*this, symbol);
	}
	void visit(cpp::type_declaration_suffix* symbol)
	{
		TREEWALKER_LEAF(symbol);
		if(forward != 0)
		{
			declaration = pointOfDeclaration(enclosing, *forward, &gClass, 0, DeclSpecifiers(), enclosing == templateEnclosing);
			forward->dec.p = declaration;
			type = declaration;
		}

		if(isUnion
			&& isAnonymous(*declaration))
		{
			/* class.union-2
			The names of the members of an anonymous union
			shall be distinct from the names of any other entity in the scope in which the anonymous union is declared.
			For the purpose of name lookup, after the anonymous union definition, the members of the anonymous union
			are considered to have been defined in the scope in which the anonymous union is declared.
			*/
			// TODO: verify that member names are distinct
			for(Scope::Declarations::iterator i = declaration->enclosed->declarations.begin(); i != declaration->enclosed->declarations.end(); ++i)
			{
				if(isAnonymous(*i))
				{
					const Identifier& name = makeIdentifier(enclosing->getUniqueName());
					(*i).name = name;
					if((*i).enclosed != 0)
					{
						(*i).enclosed->name = name;
					}
				}
				else
				{
					Declaration* declaration = findDeclaration(enclosing->declarations, (*i).name);
					if(declaration != 0)
					{
						printPosition((*i).name.position);
						std::cout << "'" << (*i).name.value << "': anonymous union member already declared" << std::endl;
						printPosition(declaration->name.position);
						throw SemanticError();
					}
				}
				(*i).scope = enclosing;
			}
			enclosing->declarations.splice(enclosing->declarations.end(), declaration->enclosed->declarations);

		}
	}
};

struct TypeParameterWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	bool isTemplate;
	TypeParameterWalker(const WalkerBase& base, bool isTemplate = false)
		: WalkerBase(base), isTemplate(isTemplate)
	{
	}
	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF(symbol);
		enclosing->templateParamTypes.push_back(gParam);
		symbol->value.dec.p = pointOfDeclaration(enclosing, symbol->value, &enclosing->templateParamTypes.back(), 0, DECLSPEC_TYPEDEF, isTemplate);
	}
	void visit(cpp::type_id* symbol)
	{
		SEMANTIC_ASSERT(!isTemplate);
		TypeIdWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::template_parameter_list* symbol)
	{
		SEMANTIC_ASSERT(isTemplate);
		TemplateParameterListWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::id_expression* symbol)
	{
		SEMANTIC_ASSERT(isTemplate);
		IdExpressionWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		Declaration* declaration = walker.declaration;
		if(declaration != 0)
		{
			if(declaration == &gUndeclared
				|| !isTemplateName(*declaration))
			{
				reportIdentifierMismatch(*walker.id, declaration, "template-name");
			}
			else
			{
				walker.id->dec.p = declaration;
			}
		}
	}
};

struct TemplateParameterListWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	TemplateParameterListWalker(const WalkerBase& base)
		: WalkerBase(base)
	{
	}
	void visit(cpp::type_parameter_default* symbol)
	{
		TypeParameterWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::type_parameter_template* symbol)
	{
		TypeParameterWalker walker(*this, true);
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::parameter_declaration* symbol)
	{
		SimpleDeclarationWalker walker(*this, false, true);
		TREEWALKER_WALK(walker, symbol);
		walker.commit();
	}
};

struct TemplateDeclarationWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Declaration* declaration;
	TemplateDeclarationWalker(const WalkerBase& base)
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
	DeclarationWalker(const WalkerBase& base)
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
		SimpleDeclarationWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		SEMANTIC_ASSERT(walker.type.declaration != 0);
		declaration = walker.declaration;
	}
	// occurs in for-init-statement
	void visit(cpp::simple_declaration* symbol)
	{
		SimpleDeclarationWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		SEMANTIC_ASSERT(walker.type.declaration != 0);
		declaration = walker.declaration;
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

	NamespaceWalker(const WalkerBase& base)
		: WalkerBase(base)
	{
	}

	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF(symbol);
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

	template<typename T>
	void visit(cpp::ambiguity<T>* symbol)
	{
		SEMANTIC_ASSERT(symbol->second == 0);
		visit(symbol->first);
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




void printSymbol(cpp::declaration_seq* p, const char* path)
{
	try
	{
#ifndef MINGLE
		WalkerContext context;
		Walker::NamespaceWalker walker(context);
		walker.visit(makeSymbol(p));

		SymbolPrinter printer(path);
		printer.visit(makeSymbol(p));
#endif
	}
	catch(SemanticError&)
	{
	}
}

void printSymbol(cpp::statement_seq* p, const char* path)
{
	try
	{
#ifndef MINGLE
		WalkerContext context;
		Walker::NamespaceWalker walker(context);
		walker.visit(makeSymbol(p));

		SymbolPrinter printer(path);
		printer.visit(makeSymbol(p));
#endif
	}
	catch(SemanticError&)
	{
	}

}




#ifdef MINGLE
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
	catch(SemanticError&)
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
	dumpProfile(gProfileTemplateId);
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

