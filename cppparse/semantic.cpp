

#include "semantic.h"

#include "cpptree.h"
#include "printer.h"
#include "profiler.h"

#include "parser/symbols.h"

#include <fstream>


#include <iostream>
#include <list>
#include <set>
#include <map>

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
typedef std::vector<struct Type> Types;

template<typename T>
class Copied
{
	T* p;
public:
	Copied()
		: p(0)
	{
	}
	~Copied()
	{
		delete p;
	}
	Copied(const T& value)
		: p(new T(value))
	{
	}
	Copied(const Copied& other)
		: p(other.p == 0 ? 0 : new T(*other.p))
	{
	}
	Copied& operator=(const Copied& other)
	{
		Copied tmp(other);
		tmp.swap(*this);
		return *this;
	}
	Copied& operator=(const T& value)
	{
		Copied tmp(value);
		tmp.swap(*this);
		return *this;
	}

	void swap(Copied& other)
	{
		std::swap(p, other.p);
	}

	T* get()
	{
		return p;
	}
	const T* get() const
	{
		return p;
	}

	bool empty() const
	{
		return p == 0;
	}
	T& back()
	{
		return *p;
	}
	const T& back() const
	{
		return *p;
	}
};

typedef Copied<Type> Qualifying;

struct Type
{
	Declaration* declaration;
	TemplateArguments arguments;
	Qualifying qualifying;
	mutable bool visited;
	Type(Declaration* declaration) : declaration(declaration), visited(false)
	{
	}
	void swap(Type& other)
	{
		std::swap(declaration, other.declaration);
		std::swap(arguments, other.arguments);
		std::swap(qualifying, other.qualifying);
	}
#if 0
private:
	Type(const Type&);
	Type& operator=(const Type&);
#endif
};

struct DependentContext
{
	const Scope& enclosing;
	const Scope& templateParams;
	DependentContext(const Scope& enclosing, const Scope& templateParams)
		: enclosing(enclosing), templateParams(templateParams)
	{
	}
};


struct DependencyCallback
{
	void* context;
	typedef bool (*Function)(void*, const DependentContext&);
	Function function;

	bool operator()(const DependentContext& args) const
	{
		return function(context, args);
	}
};

template<typename T>
DependencyCallback makeDependencyCallback(T* declaration, bool (*isDependent)(T*, const DependentContext&))
{
	DependencyCallback result = { declaration, DependencyCallback::Function(isDependent) };
	return result;
}

struct DependencyNode
{
	DependencyNode* next;
	DependencyCallback isDependent;
	DependencyNode(DependencyNode* next, const DependencyCallback& isDependent)
		: next(next), isDependent(isDependent)
	{
	}
};


struct Dependent
{
	DependencyNode* head;
	Dependent() : head(0)
	{
	}
};

bool evaluateDependent(DependencyNode* dependent, const DependentContext& context)
{
	for(DependencyNode* p = dependent; p != 0; p = p->next)
	{
		if(p->isDependent(context))
		{
			return true;
		}
	}
	return false;
}

struct TemplateArgument
{
	Type type;
	Dependent dependent;
#if 0
	TemplateArgument(const Type& type) : type(type)
	{
	}
#endif
	TemplateArgument() : type(0)
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

const size_t INDEX_INVALID = size_t(-1);

struct Declaration
{
	Scope* scope;
	Identifier name;
	Type type;
	Scope* enclosed;
	Declaration* overloaded;
	Dependent valueDependent;
	DeclSpecifiers specifiers;
	size_t templateParameter;
	Types templateParamDefaults;
	TemplateArguments arguments;
	bool isTemplate;

	Declaration(Scope* scope, Identifier name, const Type& type, Scope* enclosed, DeclSpecifiers specifiers = DeclSpecifiers(), bool isTemplate = false, const TemplateArguments& arguments = TemplateArguments(), size_t templateParameter = INDEX_INVALID)
		: scope(scope), name(name), type(type), enclosed(enclosed), overloaded(0), specifiers(specifiers), templateParameter(templateParameter), arguments(arguments), isTemplate(isTemplate)
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
#if 1
	typedef std::multimap<const char*, Declaration> Declarations;
#else
	typedef std::list<Declaration> Declarations;
#endif
	Declarations declarations;
	ScopeType type;
	Types bases;

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
Declaration gEnumerator(0, makeIdentifier("$enumerator"), &gSpecial, 0);
Declaration gUnknown(0, makeIdentifier("$unknown"), &gSpecial, 0);

Declaration gChar(0, makeIdentifier("$char"), &gSpecial, 0);
Declaration gSignedChar(0, makeIdentifier("$signed-char"), &gSpecial, 0);
Declaration gUnsignedChar(0, makeIdentifier("$unsigned-char"), &gSpecial, 0);
Declaration gSignedShortInt(0, makeIdentifier("$signed-short-int"), &gSpecial, 0);
Declaration gUnsignedShortInt(0, makeIdentifier("$unsigned-short-int"), &gSpecial, 0);
Declaration gSignedInt(0, makeIdentifier("$signed-int"), &gSpecial, 0);
Declaration gUnsignedInt(0, makeIdentifier("$unsigned-int"), &gSpecial, 0);
Declaration gSignedLongInt(0, makeIdentifier("$signed-long-int"), &gSpecial, 0);
Declaration gUnsignedLongInt(0, makeIdentifier("$unsigned-long-int"), &gSpecial, 0);
Declaration gSignedLongLongInt(0, makeIdentifier("$signed-long-long-int"), &gSpecial, 0);
Declaration gUnsignedLongLongInt(0, makeIdentifier("$unsigned-long-long-int"), &gSpecial, 0);
Declaration gWCharT(0, makeIdentifier("$wchar_t"), &gSpecial, 0);
Declaration gBool(0, makeIdentifier("$bool"), &gSpecial, 0);
Declaration gFloat(0, makeIdentifier("$float"), &gSpecial, 0);
Declaration gDouble(0, makeIdentifier("$double"), &gSpecial, 0);
Declaration gLongDouble(0, makeIdentifier("$long-double"), &gSpecial, 0);
Declaration gVoid(0, makeIdentifier("$void"), &gSpecial, 0);

unsigned combineFundamental(unsigned fundamental, unsigned token)
{
	unsigned mask = 1 << token;
	if((fundamental & mask) != 0)
	{
		mask <<= 16;
	}
	return fundamental | mask;
}

#define MAKE_FUNDAMENTAL(token) (1 << cpp::simple_type_specifier_builtin::token)
#define MAKE_FUNDAMENTAL2(token) (MAKE_FUNDAMENTAL(token) << 16)

Declaration* getFundamentalType(unsigned fundamental)
{
	switch(fundamental)
	{
	case MAKE_FUNDAMENTAL(CHAR): return &gChar;
	case MAKE_FUNDAMENTAL(SIGNED) | MAKE_FUNDAMENTAL(CHAR): return &gSignedChar;
	case MAKE_FUNDAMENTAL(UNSIGNED) | MAKE_FUNDAMENTAL(CHAR): return &gUnsignedChar;
	case MAKE_FUNDAMENTAL(SHORT):
	case MAKE_FUNDAMENTAL(SIGNED) | MAKE_FUNDAMENTAL(SHORT):
	case MAKE_FUNDAMENTAL(SIGNED) | MAKE_FUNDAMENTAL(SHORT) | MAKE_FUNDAMENTAL(INT): return &gSignedShortInt;
	case MAKE_FUNDAMENTAL(UNSIGNED) | MAKE_FUNDAMENTAL(SHORT):
	case MAKE_FUNDAMENTAL(UNSIGNED) | MAKE_FUNDAMENTAL(SHORT) | MAKE_FUNDAMENTAL(INT): return &gUnsignedShortInt;
	case MAKE_FUNDAMENTAL(INT):
	case MAKE_FUNDAMENTAL(SIGNED) | MAKE_FUNDAMENTAL(INT): return &gSignedInt;
	case MAKE_FUNDAMENTAL(UNSIGNED):
	case MAKE_FUNDAMENTAL(UNSIGNED) | MAKE_FUNDAMENTAL(INT): return &gUnsignedInt;
	case MAKE_FUNDAMENTAL(LONG):
	case MAKE_FUNDAMENTAL(SIGNED) | MAKE_FUNDAMENTAL(LONG):
	case MAKE_FUNDAMENTAL(SIGNED) | MAKE_FUNDAMENTAL(LONG) | MAKE_FUNDAMENTAL(INT): return &gSignedLongInt;
	case MAKE_FUNDAMENTAL(UNSIGNED) | MAKE_FUNDAMENTAL(LONG):
	case MAKE_FUNDAMENTAL(UNSIGNED) | MAKE_FUNDAMENTAL(LONG) | MAKE_FUNDAMENTAL(INT): return &gUnsignedLongInt;
	case MAKE_FUNDAMENTAL(LONG) | MAKE_FUNDAMENTAL2(LONG):
	case MAKE_FUNDAMENTAL(SIGNED) | MAKE_FUNDAMENTAL(LONG) | MAKE_FUNDAMENTAL2(LONG):
	case MAKE_FUNDAMENTAL(SIGNED) | MAKE_FUNDAMENTAL(LONG) | MAKE_FUNDAMENTAL2(LONG) | MAKE_FUNDAMENTAL(INT): return &gSignedLongLongInt;
	case MAKE_FUNDAMENTAL(UNSIGNED) | MAKE_FUNDAMENTAL(LONG) | MAKE_FUNDAMENTAL2(LONG):
	case MAKE_FUNDAMENTAL(UNSIGNED) | MAKE_FUNDAMENTAL(LONG) | MAKE_FUNDAMENTAL2(LONG) | MAKE_FUNDAMENTAL(INT): return &gUnsignedLongLongInt;
	case MAKE_FUNDAMENTAL(WCHAR_T): return &gWCharT;
	case MAKE_FUNDAMENTAL(BOOL): return &gBool;
	case MAKE_FUNDAMENTAL(FLOAT): return &gFloat;
	case MAKE_FUNDAMENTAL(DOUBLE): return &gDouble;
	case MAKE_FUNDAMENTAL(LONG) | MAKE_FUNDAMENTAL(DOUBLE): return &gLongDouble;
	case MAKE_FUNDAMENTAL(VOID): return &gVoid;
	}
	return 0;
}

Declaration gDependentType(0, makeIdentifier("$type"), &gSpecial, 0);
Declaration gDependentObject(0, makeIdentifier("$object"), &gUnknown, 0);
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

const Type& getInstantiatedType(const Type& type)
{
	if(type.declaration->specifiers.isTypedef
		&& type.declaration->templateParameter == INDEX_INVALID)
	{
		const Type& original = getInstantiatedType(type.declaration->type);

		size_t index = original.declaration->templateParameter;
		if(index != INDEX_INVALID)
		{
			// original type is a template-parameter
			// find template-specialisation in list of qualifiers
			for(const Type* i = type.qualifying.get(); i != 0; i = (*i).qualifying.get())
			{
				const Type& instantiated = getInstantiatedType(*i);
				if(instantiated.declaration->enclosed == original.declaration->scope)
				{
					if(index < instantiated.arguments.size())
					{
						SEMANTIC_ASSERT(instantiated.arguments[index].type.declaration != 0);
						return instantiated.arguments[index].type;
					}
					else
					{
						SEMANTIC_ASSERT(index < instantiated.declaration->templateParamDefaults.size()
							&& instantiated.declaration->templateParamDefaults[index].declaration != 0);
						return instantiated.declaration->templateParamDefaults[index];
					}
				}
			}
		}

		return original;
	}
	return type;
}

bool isFunction(const Declaration& declaration)
{
	return declaration.enclosed != 0 && declaration.enclosed->type == SCOPETYPE_PROTOTYPE;
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

bool isVisible(Declaration* declaration, const Scope& scope)
{
	if(declaration->scope == &scope)
	{
		return true;
	}
	if(scope.parent != 0)
	{
		return isVisible(declaration, *scope.parent);
	}
	return false;
}

bool isTemplateParameter(Declaration* declaration, const DependentContext& context)
{
	return declaration->templateParameter != INDEX_INVALID && (isVisible(declaration, context.templateParams) || isVisible(declaration, context.enclosing));
}


bool isDependent(const Type& type, const DependentContext& context);

bool isDependent(const Types& bases, const DependentContext& context)
{
	for(Types::const_iterator i = bases.begin(); i != bases.end(); ++i)
	{
		if((*i).visited)
		{
			continue;
		}
		if(isDependent(*i, context))
		{
			return true;
		}
	}
	return false;
}

// returns true if \p type is an array typedef with a value-dependent size
bool isValueDependent(const Type& type, const DependentContext& context)
{
	if(type.declaration->specifiers.isTypedef)
	{
		return evaluateDependent(type.declaration->valueDependent.head, context)
			|| isValueDependent(type.declaration->type, context);
	}
	return false;
}

bool isDependent(const TemplateArguments& arguments, const DependentContext& context)
{
	for(TemplateArguments::const_iterator i = arguments.begin(); i != arguments.end(); ++i)
	{
		if((*i).type.visited)
		{
			continue;
		}
		if(evaluateDependent((*i).dependent.head, context) // array-size or constant-initializer
			|| ((*i).type.declaration != 0
				&& isDependent((*i).type, context)))
		{
			return true;
		}
	}
	return false;
}

bool isDependentInternal(const Type& type, const DependentContext& context)
{
	ProfileScope profile(gProfileIdentifier);
	if(isValueDependent(type, context))
	{
		return true;
	}
	const Type& original = getInstantiatedType(type);
	if(original.declaration == &gTypename)
	{
		return true;
	}
	if(isTemplateParameter(original.declaration, context))
	{
		return true;
	}
	if(isDependent(original.arguments, context))
	{
		return true;
	}
	Scope* enclosed = original.declaration->enclosed;
	if(enclosed != 0)
	{
		return isDependent(enclosed->bases, context);
	}
	return false;
}

bool isDependent(const Type& type, const DependentContext& context)
{
	type.visited = true;
	bool result = isDependentInternal(type, context);
	type.visited = false;
	return result;
}

bool isDependentName(Declaration* declaration, const DependentContext& context)
{
	return isTemplateParameter(declaration, context)
		|| isDependent(declaration->type, context)
		|| evaluateDependent(declaration->valueDependent.head, context);
}

bool isDependentType(Type* type, const DependentContext& context)
{
	return isDependent(*type, context);
}

bool isDependentClass(Scope* scope, const DependentContext& context)
{
	return isDependent(scope->bases, context);
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
			if(!second.arguments.empty())
			{
				return second; // TODO
			}
			if(!first.arguments.empty())
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
	// HACK: ignore multiple declarations for members of template - e.g. const char Tmpl<char>::VALUE; const int Tmpl<int>::VALUE;
	if(!first.templateParamDefaults.empty())
	{
		// if enclosing is a template
		return first;
	}
	throw DeclarationError("symbol already defined");
}

Declaration* findPrimaryTemplate(Declaration* declaration)
{
	SEMANTIC_ASSERT(declaration->isTemplate);
	for(;declaration != 0; declaration = declaration->overloaded)
	{
		if(declaration->arguments.empty())
		{
			SEMANTIC_ASSERT(declaration->isTemplate);
			return declaration;
		}
	}
	SEMANTIC_ASSERT(false); // primary template not declared!
	return 0;
}

bool isEqual(const Types& params, const TemplateArguments& left, const TemplateArguments& right);

bool isEqual(const Type& l, const Type& r)
{
	if(l.declaration == 0
		|| r.declaration == 0)
	{
		// TODO: non-type parameters
		return l.declaration == r.declaration; // match any non-type param value
	}
	const Type& left = getInstantiatedType(l);
	const Type& right = getInstantiatedType(r);
	if(left.declaration != right.declaration)
	{
		return false;
	}
	if(left.declaration->isTemplate)
	{
		Declaration* primary = findPrimaryTemplate(left.declaration);
		return isEqual(primary->templateParamDefaults, left.arguments, right.arguments);
	}
	return true;
}

bool isEqual(const Types& params, const TemplateArguments& left, const TemplateArguments& right)
{
	for(size_t i = 0; i != params.size(); ++i)
	{
		if(!isEqual(
			i < left.size() ? left[i].type : params[i],
			i < right.size() ? right[i].type : params[i]
		))
		{
			return false;
		}
	}
	return true;
}

Declaration* findTemplateSpecialization(Declaration* declaration, const TemplateArguments& arguments)
{
	SEMANTIC_ASSERT(declaration->isTemplate);
	Declaration* primary = findPrimaryTemplate(declaration);
	for(;declaration != 0; declaration = declaration->overloaded)
	{
		if(!declaration->arguments.empty()
			&& isEqual(primary->templateParamDefaults, declaration->arguments, arguments))
		{
			return declaration;
		}
	}
	return primary;
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
		std::cout << getValue((*i).second.name);
		if(++i != declarations.end())
		{
			std::cout << ", ";
		}
	}
	std::cout << " }";
}

void printBases(const Types& bases)
{
	std::cout << "{ ";
	for(Types::const_iterator i = bases.begin(); i != bases.end();)
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
	Declaration globalDecl;
#if 0
	TemplateInstantiations instantiations;
#endif

	WalkerContext() :
		global(makeIdentifier("$global"), SCOPETYPE_NAMESPACE),
		globalDecl(0, makeIdentifier("$global"), 0, &global)
	{
	}
};

#ifdef MINGLE
typedef std::list< DeferredParse<struct WalkerBase> > DeferredSymbols;
#else
typedef std::vector<struct DeferredSymbol> DeferredSymbols;
#endif

#if 0//def MINGLE
#define TREEWALKER_NEW(T, args) (new(parser->lexer.allocator.allocate(sizeof(T))) T args)
#else
#define TREEWALKER_NEW(T, args) new T args
#endif

struct WalkerBase
#ifdef MINGLE
	: public ContextBase
#endif
{
	typedef WalkerBase Base;

	WalkerContext& context;
	Scope* enclosing;
	Qualifying qualifying;
	Scope* templateParams;
	Scope* templateEnclosing;
	bool* ambiguity;
	DeferredSymbols* deferred;

	WalkerBase(WalkerContext& context)
		: context(context), enclosing(0), templateParams(0), templateEnclosing(0), ambiguity(0), deferred(0)
	{
	}

	Declaration* findDeclaration(Scope::Declarations& declarations, const Identifier& id, LookupFilter filter = isAny)
	{
		ProfileScope profile(gProfileIdentifier);
#if 1
		Scope::Declarations::iterator i = declarations.upper_bound(id.value);
		
		for(; i != declarations.begin()
			&& (*--i).first == id.value;)
		{
			if(filter((*i).second))
			{
				return &(*i).second;
			}
		}
		return 0;
#else
		for(Scope::Declarations::iterator i = declarations.begin(); i != declarations.end(); ++i)
		{
			if((*i).name.value == id.value
				&& filter(*i))
			{
				return &(*i);
			}
		}
		return 0;
#endif
	}

	Declaration* findDeclaration(Types& bases, const Identifier& id, LookupFilter filter = isAny)
	{
		ProfileScope profile(gProfileIdentifier);
		for(Types::iterator i = bases.begin(); i != bases.end(); ++i)
		{
			Scope* scope = getInstantiatedType(*i).declaration->enclosed;
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

	Declaration* findDeclaration(Scope::Declarations& declarations, Types& bases, const Identifier& id, LookupFilter filter = isAny)
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
		if(getQualifyingScope() != 0)
		{
			Declaration* result = findDeclaration(*getQualifyingScope(), id, filter);
			if(result != 0)
			{
				return result;
			}
		}
		else
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
			Declaration* result = findDeclaration(*enclosing, id, filter);
			if(result != 0)
			{
				return result;
			}
		}
		return &gUndeclared;
	}

	Declaration* pointOfDeclaration(Scope* parent, const Identifier& name, const Type& type, Scope* enclosed, DeclSpecifiers specifiers = DeclSpecifiers(), bool isTemplate = false, const TemplateArguments& arguments = TemplateArguments(), size_t templateParameter = INDEX_INVALID)
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
		
		Declaration other(parent, name, type, enclosed, specifiers, isTemplate, arguments, templateParameter);
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
				if(!isNamespace(other)
					&& !isType(other)
					&& isFunction(other))
				{
					// quick hack - if any template overload of a function has been declared, all subsequent declarations are template functions
					if(declaration->isTemplate)
					{
						other.isTemplate = true;
					}
				}
				other.overloaded = declaration;
			}
		}
#if 1
		return &(*parent->declarations.insert(Scope::Declarations::value_type(name.value, other))).second;
#else
		parent->declarations.push_front(other);
		Declaration* result = &parent->declarations.front();
		return result;
#endif
	}

	Declaration* declareClass(Identifier* id, const TemplateArguments& arguments)
	{
		Scope* enclosed = templateParams != 0 ? templateParams : TREEWALKER_NEW(Scope, (makeIdentifier("$class")));
		enclosed->type = SCOPETYPE_CLASS;
		Declaration* declaration = pointOfDeclaration(enclosing, id == 0 ? makeIdentifier(enclosing->getUniqueName()) : *id, &gClass, enclosed, DeclSpecifiers(), enclosing == templateEnclosing, arguments);
		if(id != 0)
		{
			id->dec.p = declaration;
		}
		enclosed->name = declaration->name;
		return declaration;
	}

	Declaration* declareObject(Scope* parent, Identifier* id, const Type& type, Scope* enclosed, DeclSpecifiers specifiers, Dependent valueDependent, size_t templateParameter)
	{
		Declaration* declaration = pointOfDeclaration(parent, *id, type, enclosed, specifiers, enclosing == templateEnclosing, TemplateArguments(), templateParameter); // 3.3.1.1
		declaration->valueDependent = valueDependent;
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
		if(getInstantiatedType(base).declaration == declaration)
		{
			return; // TODO: implement template-instantiation, and disallow inheriting from current-instantiation
		}
		declaration->enclosed->bases.push_back(base);
	}

	Scope* getQualifyingScope()
	{
		if(qualifying.empty())
		{
			return 0;
		}
		return getInstantiatedType(qualifying.back()).declaration->enclosed;
	}

	void clearQualifying()
	{
		qualifying = Qualifying();
	}

	void setQualifying(const Type& type)
	{
#if 0 // allow incomplete types as qualifying, for nested-name-specifier in ptr-operator (defining member-function-ptr)
		if(type.declaration->enclosed == 0)
		{
			// TODO
			//printPosition(symbol->id->value.position);
			std::cout << "'" << getValue(type.declaration->name) << "' is incomplete, declared here:" << std::endl;
			printPosition(type.declaration->name.position);
			throw SemanticError();
		}
#endif
		qualifying = type;
	}
	void setQualifying(const Qualifying& other)
	{
		qualifying = other;
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
		if(getQualifyingScope() != 0)
		{
			std::cout << "qualifying:" << std::endl;
			::printScope(*getQualifyingScope());
		}
		else
		{
			std::cout << "enclosing:" << std::endl;
			::printScope(*enclosing);
		}
	}

	bool isDependent(const Type& type)
	{
		return ::isDependent(type, DependentContext(*enclosing, templateParams != 0 ? *templateParams : Scope(IDENTIFIER_NULL)));
	}

	bool isDependent(const Types& bases)
	{
		return ::isDependent(bases, DependentContext(*enclosing, templateParams != 0 ? *templateParams : Scope(IDENTIFIER_NULL)));
	}

	bool isDependent(const Qualifying& qualifying)
	{
		if(qualifying.empty())
		{
			return false;
		}
		const Type& instantiated = getInstantiatedType(qualifying.back());
		if(isDependent(instantiated.qualifying))
		{
			return true;
		}
		return isDependent(instantiated);
	}

	void addDependent(Dependent& dependent, const DependencyCallback& callback)
	{
		dependent.head = TREEWALKER_NEW(DependencyNode, (dependent.head, callback));
	}
	void addDependent(Dependent& dependent, Dependent& other)
	{
		if(other.head != 0)
		{
			addDependent(dependent, makeDependencyCallback(other.head, evaluateDependent));
		}
	}
	void addDependentName(Dependent& dependent, Declaration* declaration)
	{
		addDependent(dependent, makeDependencyCallback(declaration, isDependentName));
	}
	void addDependentType(Dependent& dependent, Declaration* declaration)
	{
		addDependent(dependent, makeDependencyCallback(&declaration->type, isDependentType));
	}
	void addDependent(Dependent& dependent, const Type& type)
	{
		addDependent(dependent, makeDependencyCallback(TREEWALKER_NEW(Type, (type)), isDependentType));
	}
	void addDependent(Dependent& dependent, Scope* scope)
	{
		addDependent(dependent, makeDependencyCallback(scope, isDependentClass));
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
	// returns true if \p declaration is a template class, function or template-parameter
	return declaration.isTemplate && (isClass(declaration) || isFunction(declaration) || isTypedef(declaration));
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
#define TREEWALKER_LEAF(symbol) SYMBOL_WALK(*this, symbol)
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
		clearQualifying();
		// TODO: store args
		TemplateArgumentListWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		arguments.swap(walker.arguments);
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
		if(!isDependent(qualifying))
		{
			declaration = findDeclaration(*id);
		}
	}
	void visit(cpp::simple_template_id* symbol)
	{
		UncheckedTemplateIdWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		id = walker.id;
		if(!isDependent(qualifying))
		{
			declaration = findDeclaration(*id);
			if(declaration == &gUndeclared
				|| !isTemplateName(*declaration))
			{
				reportIdentifierMismatch(*id, declaration, "template-name");
			}
			else
			{
				declaration = findTemplateSpecialization(declaration, walker.arguments);
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
			setQualifying(&context.globalDecl);
		}
	}
	void visit(cpp::nested_name_specifier* symbol)
	{
		NestedNameSpecifierWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		setQualifying(walker.qualifying);
	}
	void visit(cpp::unqualified_id* symbol)
	{
		UnqualifiedIdWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
		id = walker.id;
	}
	void visit(cpp::qualified_id_suffix* symbol)
	{
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
		setQualifying(walker.qualifying);
	}
	void visit(cpp::unqualified_id* symbol)
	{
		// TODO
		UnqualifiedIdWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
		id = walker.id;
		setQualifying(walker.qualifying);
		isIdentifier = walker.isIdentifier;
	}
};

struct ExplicitTypeExpressionWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Dependent typeDependent;
	Dependent valueDependent;
	ExplicitTypeExpressionWalker(const WalkerBase& base)
		: WalkerBase(base)
	{
	}
	void visit(cpp::simple_type_specifier* symbol)
	{
		TypeSpecifierWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		addDependent(typeDependent, walker.type);
	}
	void visit(cpp::type_id* symbol)
	{
		TypeIdWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		addDependent(typeDependent, walker.type);
		addDependent(typeDependent, walker.valueDependent);
	}
	void visit(cpp::new_type* symbol)
	{
		TypeIdWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		addDependent(typeDependent, walker.type);
		addDependent(typeDependent, walker.valueDependent);
	}
	void visit(cpp::assignment_expression* symbol)
	{
		ExpressionWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		addDependent(valueDependent, walker.valueDependent);
	}
	void visit(cpp::cast_expression* symbol)
	{
		ExpressionWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		addDependent(valueDependent, walker.valueDependent);
	}
};

struct DependentPrimaryExpressionWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Declaration* declaration;
	Identifier* id;
	Dependent typeDependent;
	DependentPrimaryExpressionWalker(const WalkerBase& base)
		: WalkerBase(base), declaration(0), id(0)
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
					addDependentType(typeDependent, declaration);
				}
			}
			else if(walker.id != 0)
			{
				walker.id->dec.p = &gDependentObject;
				if(!walker.qualifying.empty())
				{
					addDependent(typeDependent, walker.qualifying.back());
				}
			}
		}
	}
	void visit(cpp::primary_expression_parenthesis* symbol)
	{
		ExpressionWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		addDependent(typeDependent, walker.typeDependent);
	}
};

struct DependentPostfixExpressionWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Declaration* declaration;
	Identifier* id;
	Dependent typeDependent;
	DependentPostfixExpressionWalker(const WalkerBase& base)
		: WalkerBase(base), declaration(0), id(0)
	{
	}
	void visit(cpp::primary_expression* symbol)
	{
		DependentPrimaryExpressionWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
		id = walker.id;
		addDependent(typeDependent, walker.typeDependent);
	}
	void visit(cpp::postfix_expression_call* symbol)
	{
		ExpressionWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		addDependent(typeDependent, walker.typeDependent);
		if(id != 0)
		{
			if(typeDependent.head == 0)
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
						addDependentType(typeDependent, declaration);
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
	Dependent typeDependent;
	Dependent valueDependent;
	ExpressionWalker(const WalkerBase& base)
		: WalkerBase(base)
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
		addDependent(typeDependent, getClassScope());
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
				addDependentType(typeDependent, declaration);
				addDependentType(valueDependent, declaration);
				addDependentName(valueDependent, declaration);
			}
		}
		else
		{
			if(isDependent(walker.qualifying))
			{
				walker.id->dec.p = &gDependentObject;
			}
			if(!walker.qualifying.empty())
			{
				addDependent(typeDependent, walker.qualifying.back());
			}
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
		addDependent(typeDependent, walker.typeDependent);
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
		addDependent(typeDependent, walker.typeDependent);
	}
	void visit(cpp::new_expression_placement* symbol)
	{
		ExplicitTypeExpressionWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		addDependent(typeDependent, walker.typeDependent);
	}
	void visit(cpp::new_expression_default* symbol)
	{
		ExplicitTypeExpressionWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		addDependent(typeDependent, walker.typeDependent);
	}
	void visit(cpp::postfix_expression_cast* symbol)
	{
		ExplicitTypeExpressionWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		addDependent(typeDependent, walker.typeDependent);
		if(symbol->op->id != cpp::cast_operator::DYNAMIC)
		{
			addDependent(valueDependent, walker.typeDependent);
		}
		addDependent(valueDependent, walker.valueDependent);
	}
	void visit(cpp::cast_expression_default* symbol)
	{
		ExplicitTypeExpressionWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		addDependent(typeDependent, walker.typeDependent);
		addDependent(valueDependent, walker.typeDependent);
		addDependent(valueDependent, walker.valueDependent);
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
		TREEWALKER_WALK(walker, symbol);
		addDependent(valueDependent, walker.typeDependent);
	}
	void visit(cpp::unary_expression_sizeoftype* symbol)
	{
		TypeIdWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		addDependent(valueDependent, walker.type);
		addDependent(valueDependent, walker.valueDependent);
	}
	void visit(cpp::postfix_expression_typeid* symbol)
	{
		ExpressionWalker walker(*this);
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
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::throw_expression* symbol)
	{
		ExpressionWalker walker(*this);
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
		arguments.push_back(TemplateArgument());
		arguments.back().type.swap(walker.type);
		arguments.back().dependent = walker.valueDependent;
	}
	void visit(cpp::assignment_expression* symbol)
	{
		ExpressionWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		arguments.push_back(TemplateArgument()); // todo: evaluate constant-expression (unless it's dependent expression)
		arguments.back().dependent = walker.typeDependent;
		addDependent(arguments.back().dependent, walker.valueDependent);
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
		if(!isDependent(qualifying))
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
				type.qualifying = qualifying;
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
		clearQualifying();
		// TODO: instantiation
		TemplateArgumentListWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
#if 0
		pointOfInstantiation(declaration, walker.arguments);
#endif
		if(type.declaration != 0)
		{
			type.declaration = findTemplateSpecialization(type.declaration, walker.arguments);
		}
		type.arguments.swap(walker.arguments);
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
		if(!isDependent(qualifying))
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
				type.qualifying = qualifying;
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

	Type type;
	bool allowDependent;
	bool isTemplate;
	NestedNameSpecifierSuffixWalker(const WalkerBase& base, bool allowDependent = false)
		: WalkerBase(base), type(0), allowDependent(allowDependent), isTemplate(false)
	{
	}
	void visit(cpp::terminal<boost::wave::T_TEMPLATE> symbol)
	{
		if(symbol.value != 0)
		{
			isTemplate = true;
		}
	}
	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF(symbol);
		if(allowDependent
			|| !isDependent(qualifying))
		{
			Declaration* declaration = findDeclaration(symbol->value, isNestedName);
			if(declaration == &gUndeclared)
			{
				reportIdentifierMismatch(symbol->value, declaration, "nested-name");
			}
			else
			{
				symbol->value.dec.p = declaration;
				type = declaration;
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
		if(!isTemplate
			&& (allowDependent
				|| !isDependent(qualifying)))
		{
			Declaration* declaration = findDeclaration(*walker.id, isNestedName);
			if(declaration == &gUndeclared)
			{
				reportIdentifierMismatch(*walker.id, declaration, "nested-name");
			}
			else
			{
				declaration = findTemplateSpecialization(declaration, walker.arguments);
				type = declaration;
				type.arguments.swap(walker.arguments);
			}
		}
	}
	void visit(cpp::terminal<boost::wave::T_COLON_COLON> symbol)
	{
		if(symbol.value != 0)
		{
			if(type.declaration != 0)
			{
				type.qualifying.swap(qualifying);
				qualifying = type;
			}
		}
	}
};

struct NestedNameSpecifierWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Type type;
	bool allowDependent;
	NestedNameSpecifierWalker(const WalkerBase& base, bool allowDependent = false)
		: WalkerBase(base), type(0), allowDependent(allowDependent)
	{
	}
	void visit(cpp::nested_name_specifier_prefix* symbol)
	{
		NestedNameSpecifierWalker walker(*this, allowDependent);
		TREEWALKER_WALK(walker, symbol);
		setQualifying(walker.qualifying);
	}
	void visit(cpp::nested_name_specifier_suffix* symbol)
	{
		NestedNameSpecifierSuffixWalker walker(*this, allowDependent);
		TREEWALKER_WALK(walker, symbol);
		setQualifying(walker.qualifying);
	}
	void visit(cpp::namespace_name* symbol)
	{
		NamespaceNameWalker walker(*this, isNestedName);
		TREEWALKER_WALK(walker, symbol);
		type = walker.declaration;
	}
	void visit(cpp::type_name* symbol)
	{
		TypeNameWalker walker(*this, isNestedName, true);
		TREEWALKER_WALK(walker, symbol);
		if(allowDependent
			|| !isDependent(qualifying))
		{
			type.swap(walker.type);
		}
	}
	void visit(cpp::terminal<boost::wave::T_COLON_COLON> symbol)
	{
		if(symbol.value != 0)
		{
			if(type.declaration != 0)
			{
				type.qualifying.swap(qualifying);
				qualifying = type;
			}
		}
	}
};

struct TypeSpecifierWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Type type;
	unsigned fundamental;
	TypeSpecifierWalker(const WalkerBase& base)
		: WalkerBase(base), type(0), fundamental(0)
	{
	}

	void visit(cpp::simple_type_specifier_name* symbol)
	{
		TypeSpecifierWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		type = walker.type;
		fundamental = walker.fundamental;
	}
	void visit(cpp::simple_type_specifier_template* symbol)
	{
		TypeSpecifierWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		type = walker.type;
		fundamental = walker.fundamental;
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
			setQualifying(&context.globalDecl);
		}
	}
	void visit(cpp::nested_name_specifier* symbol)
	{
		NestedNameSpecifierWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		setQualifying(walker.qualifying);
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
		fundamental = symbol->id;
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
			setQualifying(&context.globalDecl);
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
		setQualifying(walker.qualifying);
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
		setQualifying(walker.qualifying);
	}
};

struct DeclaratorWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Identifier* id;
	Scope* paramScope;
	Dependent valueDependent;
	DeclaratorWalker(const WalkerBase& base)
		: WalkerBase(base), id(&gAnonymousId), paramScope(0)
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

		if(walker.getQualifyingScope() != 0
			&& enclosing->type != SCOPETYPE_CLASS) // in 'class C { friend Q::N(X); };' X should be looked up in the scope of C rather than Q
		{
			enclosing = walker.getQualifyingScope(); // names in declarator suffix (array-size, parameter-declaration) are looked up in declarator-id's qualifying scope
		}
	}
	void visit(cpp::parameter_declaration_clause* symbol)
	{
		// hack to resolve issue: when mingled with parser, deferred parse causes constructor of ParameterDeclarationClauseWalker to be invoked twice
		WalkerBase base(*this);
		base.pushScope(TREEWALKER_NEW(Scope, (makeIdentifier("$declarator"))));
		base.enclosing->type = SCOPETYPE_PROTOTYPE;
		base.templateParams = 0;
		if(templateParams != 0)
		{
			base.enclosing->declarations = templateParams->declarations;
			for(Scope::Declarations::iterator i = base.enclosing->declarations.begin(); i != base.enclosing->declarations.end(); ++i)
			{
				(*i).second.scope = base.enclosing;
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
		addDependent(valueDependent, walker.valueDependent);
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
		setQualifying(walker.qualifying);
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
	TemplateArguments arguments;
	bool isUnion;
	ClassHeadWalker(const WalkerBase& base)
		: WalkerBase(base), declaration(0), id(0), isUnion(false)
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
		setQualifying(walker.qualifying);
	}
	void visit(cpp::simple_template_id* symbol) 
	{
		UncheckedTemplateIdWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		// TODO: don't declare anything - this is a template (partial) specialisation
		id = walker.id;
		arguments.swap(walker.arguments);
	}
	void visit(cpp::terminal<boost::wave::T_COLON> symbol)
	{
		// defer class declaration until we know this is a class-specifier - it may be an elaborated-type-specifier until ':' is discovered
		// 3.3.1.3 The point of declaration for a class first declared by a class-specifier is immediately after the identifier or simple-template-id (if any) in its class-head
		declaration = declareClass(id, arguments);
	}
	void visit(cpp::base_specifier* symbol) 
	{
		BaseSpecifierWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		if(walker.type.declaration != 0) // declaration == 0 if base-class is dependent
		{
			SEMANTIC_ASSERT(declaration->enclosed != 0);
			addBase(declaration, walker.type);
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
			setQualifying(&context.globalDecl);
		}
	}
	void visit(cpp::nested_name_specifier* symbol)
	{
		NestedNameSpecifierWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		setQualifying(walker.qualifying);
	}
	void visit(cpp::unqualified_id* symbol)
	{
		UnqualifiedIdWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		if(!isTypename
			&& !isDependent(qualifying))
		{
			Declaration* declaration = walker.declaration;
			if(declaration == &gUndeclared
				|| !(isObject(*declaration) || isTypeName(*declaration)))
			{
				reportIdentifierMismatch(*walker.id, declaration, "object-name or type-name");
			}
			else
			{
				// TODO: check for conflicts with earlier declarations
				enclosing->declarations.insert(Scope::Declarations::value_type(declaration->name.value, *declaration));
				walker.id->dec.p = declaration;
			}
		}
		else
		{
			// TODO: introduce typename into enclosing namespace
			walker.id->dec.p = &gDependentType;
		}
	}
	void visit(cpp::terminal<boost::wave::T_TYPENAME> symbol)
	{
		isTypename = true;
	}
};

struct NamespaceAliasDefinitionWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Identifier* id;
	NamespaceAliasDefinitionWalker(const WalkerBase& base)
		: WalkerBase(base), id(0)
	{
	}
	void visit(cpp::terminal<boost::wave::T_COLON_COLON> symbol)
	{
		if(symbol.value != 0)
		{
			setQualifying(&context.globalDecl);
		}
	}
	void visit(cpp::nested_name_specifier* symbol)
	{
		NestedNameSpecifierWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		setQualifying(walker.qualifying);
	}
	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF(symbol);
		if(id == 0)
		{
			id = &symbol->value;
		}
		else
		{
			Declaration* declaration = findDeclaration(symbol->value, isNamespace);
			if(declaration == &gUndeclared)
			{
				reportIdentifierMismatch(symbol->value, declaration, "namespace-name");
			}
			else
			{
				// TODO: check for conflicts with earlier declarations
				declaration = pointOfDeclaration(enclosing, *id, &gNamespace, declaration->enclosed);
				id->dec.p = declaration;
			}
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
	TemplateArguments arguments;
	DeferredSymbols deferred;
	bool isUnion;
	ClassSpecifierWalker(const WalkerBase& base)
		: WalkerBase(base), declaration(0), id(0), isUnion(false)
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
		arguments.swap(walker.arguments);
	}
	void visit(cpp::terminal<boost::wave::T_LEFTBRACE> symbol)
	{
		// defer class declaration until we know this is a class-specifier - it may be an elaborated-type-specifier until '{' is discovered
		if(declaration == 0)
		{
			// 3.3.1.3 The point of declaration for a class first declared by a class-specifier is immediately after the identifier or simple-template-id (if any) in its class-head
			declaration = declareClass(id, arguments);
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
		declaration = pointOfDeclaration(enclosing, symbol->value, &gEnumerator, 0, DeclSpecifiers());
		symbol->value.dec.p = declaration;
	}
	void visit(cpp::constant_expression* symbol)
	{
		ExpressionWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		addDependent(declaration->valueDependent, walker.valueDependent);
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
			setQualifying(&context.globalDecl);
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
		setQualifying(walker.qualifying);
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
		if(declaration != &gUndeclared
			&& !isTypedef(*declaration))
		{
			symbol->value.dec.p = declaration;
			/* 7.1.6.3-2
			3.4.4 describes how name lookup proceeds for the identifier in an elaborated-type-specifier. If the identifier
			resolves to a class-name or enum-name, the elaborated-type-specifier introduces it into the declaration the
			same way a simple-type-specifier introduces its type-name. If the identifier resolves to a typedef-name, the
			elaborated-type-specifier is ill-formed.
			*/
#if 0 // allow hiding a typedef with a forward-declaration
			if(isTypedef(*declaration))
			{
				printPosition(symbol->value.position);
				std::cout << "'" << symbol->value.value << "': elaborated-type-specifier refers to a typedef" << std::endl;
				printPosition(declaration->name.position);
				throw SemanticError();
			}
#endif
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
			setQualifying(&context.globalDecl);
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
		setQualifying(walker.qualifying);
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
	unsigned fundamental;
	DeclSpecifiers specifiers;
	Identifier* forward;
	bool isUnion;
	bool isTemplateParameter;
	DeclSpecifierSeqWalker(const WalkerBase& base, bool isTemplateParameter = false)
		: WalkerBase(base), type(0), fundamental(0), forward(0), isUnion(false), isTemplateParameter(isTemplateParameter)
	{
	}

	void visit(cpp::simple_type_specifier* symbol)
	{
		TypeSpecifierWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		type = walker.type;
		if(walker.type.declaration == 0)
		{
			fundamental = combineFundamental(0, walker.fundamental);
			type = getFundamentalType(fundamental);
		}
	}
	void visit(cpp::simple_type_specifier_builtin* symbol)
	{
		TREEWALKER_LEAF(symbol);
		fundamental = combineFundamental(fundamental, symbol->id);
		type = getFundamentalType(fundamental);
	}
	void visit(cpp::elaborated_type_specifier* symbol)
	{
#if 0 //def MINGLE
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
		pushScope(TREEWALKER_NEW(Scope, (makeIdentifier(enclosing->getUniqueName()), SCOPETYPE_LOCAL)));
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
		pushScope(TREEWALKER_NEW(Scope, (makeIdentifier(enclosing->getUniqueName()), SCOPETYPE_LOCAL))); // local scope
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
			setQualifying(&context.globalDecl);
		}
	}
	void visit(cpp::nested_name_specifier* symbol)
	{
		NestedNameSpecifierWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		setQualifying(walker.qualifying);
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
	Dependent valueDependent;
	TypeIdWalker(const WalkerBase& base)
		: WalkerBase(base), type(0)
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
		addDependent(valueDependent, walker.valueDependent);
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
	Dependent valueDependent;
	size_t templateParameter;
	bool isParameter;
	bool isUnion;

	SimpleDeclarationWalker(const WalkerBase& base, bool isParameter = false, size_t templateParameter = INDEX_INVALID) : WalkerBase(base),
		declaration(0),
		parent(0),
		id(0),
		type(&gCtor),
		enclosed(0),
		forward(0),
		isParameter(isParameter),
		templateParameter(templateParameter),
		isUnion(false)
	{
	}

	void commit()
	{
		if(id != 0)
		{
			if(enclosed == 0
				&& templateParams != 0)
			{
				templateParams->parent = parent;
				enclosed = templateParams; // for a static-member-variable definition, store template-params with different names than those in the class definition
			}
			declaration = declareObject(parent, id, type, enclosed, specifiers, valueDependent, templateParameter);

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
		DeclSpecifierSeqWalker walker(*this, templateParameter != INDEX_INVALID);
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
		/* temp.dep.constexpr
		An identifier is value-dependent if it is:
			— a name declared with a dependent type,
			— the name of a non-type template parameter,
			— a constant with effective literal type and is initialized with an expression that is value-dependent.
		*/
		valueDependent = walker.valueDependent;
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
		addDependent(declaration->valueDependent, walker.valueDependent);
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
				Declaration& member = (*i).second;
				if(isAnonymous(member))
				{
					const Identifier& name = makeIdentifier(enclosing->getUniqueName());
					member.name = name;
					if(member.enclosed != 0)
					{
						member.enclosed->name = name;
					}
				}
				else
				{
					Declaration* declaration = findDeclaration(enclosing->declarations, member.name);
					if(declaration != 0)
					{
						printPosition(member.name.position);
						std::cout << "'" << member.name.value << "': anonymous union member already declared" << std::endl;
						printPosition(declaration->name.position);
						throw SemanticError();
					}
				}
				member.scope = enclosing;
#if 1
				enclosing->declarations.insert(*i);
#endif
			}
#if 0
			enclosing->declarations.splice(enclosing->declarations.end(), declaration->enclosed->declarations);
#endif

		}
	}
};

struct TypeParameterWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Type param; // the default argument for this param
	Types params; // the default arguments for this param's template-params (if template-template-param)
	size_t templateParameter;
	TypeParameterWalker(const WalkerBase& base, size_t templateParameter)
		: WalkerBase(base), param(0), templateParameter(templateParameter)
	{
	}
	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF(symbol);
		Declaration* declaration = pointOfDeclaration(enclosing, symbol->value, &gParam, 0, DECLSPEC_TYPEDEF, !params.empty(), TemplateArguments(), templateParameter);
		symbol->value.dec.p = declaration;
		declaration->templateParamDefaults.swap(params);
	}
	void visit(cpp::type_id* symbol)
	{
		SEMANTIC_ASSERT(params.empty());
		TypeIdWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		param.swap(walker.type);
	}
	void visit(cpp::template_parameter_list* symbol)
	{
		TemplateParameterListWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		params.swap(walker.params);
	}
	void visit(cpp::id_expression* symbol)
	{
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

	Types params;
	TemplateParameterListWalker(const WalkerBase& base)
		: WalkerBase(base)
	{
	}
	void visit(cpp::type_parameter_default* symbol)
	{
		TypeParameterWalker walker(*this, params.size());
		TREEWALKER_WALK(walker, symbol);
		params.push_back(Type(0));
		params.back().swap(walker.param);
	}
	void visit(cpp::type_parameter_template* symbol)
	{
		TypeParameterWalker walker(*this, params.size());
		TREEWALKER_WALK(walker, symbol);
		params.push_back(Type(0));
		params.back().swap(walker.param);
	}
	void visit(cpp::parameter_declaration* symbol)
	{
		SimpleDeclarationWalker walker(*this, false, params.size());
		TREEWALKER_WALK(walker, symbol);
		walker.commit();
		// push a dummy param so that we have the same number of template-params as template-arguments
		params.push_back(Type(0)); // TODO: default value for non-type template-param
	}
};

struct TemplateDeclarationWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Declaration* declaration;
	Types params;
	TemplateDeclarationWalker(const WalkerBase& base)
		: WalkerBase(base), declaration(0)
	{
		templateEnclosing = enclosing;
	}
	void visit(cpp::template_parameter_list* symbol)
	{
		// collect template-params into a new scope
		Scope* scope = templateParams != 0 ? templateParams : TREEWALKER_NEW(Scope, (makeIdentifier("$template"), SCOPETYPE_TEMPLATE));
		templateParams = 0;
		pushScope(scope);
		TemplateParameterListWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		enclosing = scope->parent;
		templateParams = scope;
		params.swap(walker.params);
	}
	void visit(cpp::declaration* symbol)
	{
		DeclarationWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
		declaration->templateParamDefaults.swap(params);
		SEMANTIC_ASSERT(declaration != 0);
	}
	void visit(cpp::member_declaration* symbol)
	{
		MemberDeclarationWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
		declaration->templateParamDefaults.swap(params);
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
	void visit(cpp::namespace_alias_definition* symbol)
	{
		NamespaceAliasDefinitionWalker walker(*this);
		TREEWALKER_WALK(walker, symbol);
	}
};

struct NamespaceWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Declaration* declaration;
	Identifier* id;
	NamespaceWalker(WalkerContext& context)
		: WalkerBase(context), declaration(0), id(0)
	{
		pushScope(&context.global);
	}

	NamespaceWalker(const WalkerBase& base)
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
		if(symbol.value != 0)
		{
			if(id != 0)
			{
				declaration = pointOfDeclaration(enclosing, *id, &gNamespace, 0);
				id->dec.p = declaration;
				if(declaration->enclosed == 0)
				{
					declaration->enclosed = TREEWALKER_NEW(Scope, (*id, SCOPETYPE_NAMESPACE));
				}
				pushScope(declaration->enclosed);
			}
		}
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

