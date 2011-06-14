
#ifndef INCLUDED_CPPPARSE_SYMBOLS_H
#define INCLUDED_CPPPARSE_SYMBOLS_H


#include "cpptree.h"
#include "copied.h"
#include "allocator.h"
#include "list.h"
#include "parser.h"

#include <list>
#include <map>
#include <set>


#define SYMBOLS_ASSERT ALLOCATOR_ASSERT
typedef AllocatorError SymbolsError;

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



// ----------------------------------------------------------------------------
// Allocator

#define TREEALLOCATOR_LINEAR

#ifdef TREEALLOCATOR_LINEAR
#define TreeAllocator DeferredAllocator
#define TREEALLOCATOR_NULL TreeAllocator<int>(NullParserAllocator())
#else
#define TreeAllocator DebugAllocator
#define TREEALLOCATOR_NULL TreeAllocator<int>()
#endif

template<typename T, typename A>
class DeferredList : public List<T, A>
{
	typedef List<T, A> Base;
public:
	DeferredList()
	{
	}
	DeferredList(const A& allocator)
		: Base(allocator)
	{
	}
	~DeferredList()
	{
		A allocator(getAllocator());
		new (static_cast<Base*>(this)) Base(allocator);
	}
};

// ----------------------------------------------------------------------------
// type

#if 1
typedef DeferredList<struct TemplateArgument, TreeAllocator<struct TemplateArgument> > TemplateArguments2;

struct TemplateArguments : public TemplateArguments2
{
	TemplateArguments(const TreeAllocator<int>& allocator)
		: TemplateArguments2(allocator)
	{
	}
private:
	TemplateArguments()
	{
	}
	//TemplateArguments& operator=(const TemplateArguments&);
};
#else
typedef DeferredList<struct TemplateArgument> TemplateArguments;
#endif


typedef DeferredList<struct Type, TreeAllocator<int> > Types2;

/// A list of Type objects.
struct Types : public Types2
{
	Types(const TreeAllocator<int>& allocator)
		: Types2(allocator)
	{
	}
private:
	Types()
	{
	}
};

typedef Copied<Type, TreeAllocator<int> > CopiedType;
typedef CopiedType Qualifying;


class Declaration;
typedef SafePtr<Declaration> DeclarationPtr;

struct Type
{
	DeclarationPtr declaration;
	TemplateArguments arguments;
	Qualifying qualifying;
	bool isImplicitTemplateId; // true if template-argument-clause has not been specified
	mutable bool visited;
	Type(Declaration* declaration, const TreeAllocator<int>& allocator)
		: declaration(declaration), arguments(allocator), qualifying(allocator), isImplicitTemplateId(false), visited(false)
	{
	}
	void swap(Type& other)
	{
		std::swap(declaration, other.declaration);
		arguments.swap(other.arguments);
		qualifying.swap(other.qualifying);
		std::swap(isImplicitTemplateId, other.isImplicitTemplateId);
	}
	Type& operator=(Declaration* declaration)
	{
		SYMBOLS_ASSERT(arguments.empty());
		SYMBOLS_ASSERT(qualifying.empty());
		this->declaration = declaration;
		return *this;
	}
private:
	Type();
#if 0
private:
	Type(const Type&);
	Type& operator=(const Type&);
#endif
};

#define TYPE_NULL Type(0, TREEALLOCATOR_NULL)


// ----------------------------------------------------------------------------
// dependent-name

struct Scope;

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

typedef CopiedType TypeRef;

struct DependencyNode
{
	DependencyCallback isDependent;
	TypeRef type;
	DependencyNode(const DependencyCallback& isDependent, const TreeAllocator<int>& allocator)
		: isDependent(isDependent), type(allocator)
	{
	}
	DependencyNode(const DependencyNode& other)
		: isDependent(other.isDependent), type(other.type)
	{
		if(type.get() != 0)
		{
			isDependent.context = type.get();
		}
	}
	DependencyNode& operator=(const DependencyNode& other)
	{
		DependencyNode tmp(other);
		tmp.swap(*this);
		return *this;
	}
	void swap(DependencyNode& other)
	{
		std::swap(isDependent, other.isDependent);
		std::swap(type, other.type);
	}
};

typedef DeferredList<DependencyNode, TreeAllocator<int> > Dependent2;

struct Dependent : public Dependent2
{
	Dependent(const TreeAllocator<int>& allocator) : Dependent2(allocator)
	{
	}
	void splice(Dependent& other)
	{
		Dependent2::splice(end(), other);
	}
private:
	Dependent();
};

#define DEPENDENT_NULL Dependent(TREEALLOCATOR_NULL)

inline bool evaluateDependent(const Dependent& dependent, const DependentContext& context)
{
	for(Dependent::const_iterator i = dependent.begin(); i != dependent.end(); ++i)
	{
		if((*i).isDependent(context))
		{
			return true;
		}
	}
	return false;
}

// ----------------------------------------------------------------------------
// template-argument

struct TemplateArgument
{
	Type type;
	Dependent dependent;
#if 0
	TemplateArgument(const Type& type) : type(type)
	{
	}
#endif
	TemplateArgument(const TreeAllocator<int>& allocator)
		: type(0, allocator), dependent(allocator)
	{
	}
};

#define TEMPLATEARGUMENTS_NULL TemplateArguments(TREEALLOCATOR_NULL)



// ----------------------------------------------------------------------------
// identifier

typedef cpp::terminal_identifier Identifier;

inline Identifier makeIdentifier(const char* value)
{
	Identifier result = { value };
	return result;
}

const Identifier IDENTIFIER_NULL = makeIdentifier(0);

inline const char* getValue(const Identifier& id)
{
	return id.value == 0 ? "$unnamed" : id.value;
}

typedef SafePtr<Identifier> IdentifierPtr;

// ----------------------------------------------------------------------------
// declaration

const size_t INDEX_INVALID = size_t(-1);
typedef DeferredList<class Declaration*, TreeAllocator<int> > DeclarationList;

class Declaration
{
	Identifier* name;
public:
	Scope* scope;
	Type type;
	Scope* enclosed;
	Declaration* overloaded;
	Dependent valueDependent;
	DeclSpecifiers specifiers;
	size_t templateParameter;
	Types templateParams;
	Types templateParamDefaults;
	TemplateArguments arguments;
	bool isTemplate;

	Declaration(
		const TreeAllocator<int>& allocator,
		Scope* scope,
		Identifier& name,
		const Type& type,
		Scope* enclosed,
		DeclSpecifiers specifiers = DeclSpecifiers(),
		bool isTemplate = false,
		const TemplateArguments& arguments = TEMPLATEARGUMENTS_NULL,
		size_t templateParameter = INDEX_INVALID,
		const Dependent& valueDependent = DEPENDENT_NULL
	) : scope(scope),
		name(&name),
		type(type),
		enclosed(enclosed),
		overloaded(0),
		valueDependent(valueDependent),
		specifiers(specifiers),
		templateParameter(templateParameter),
		templateParams(allocator),
		templateParamDefaults(allocator),
		arguments(arguments),
		isTemplate(isTemplate)
	{
	}

	Identifier& getName()
	{
		return *name;
	}
	const Identifier& getName() const
	{
		return *name;
	}
	void setName(Identifier& other)
	{
		name = &other;
	}
};

typedef SafePtr<Declaration> DeclarationPtr;

inline cpp::terminal_identifier& getDeclarationId(Declaration* declaration)
{
	return declaration->getName();
}

// ----------------------------------------------------------------------------
// scope

struct UniqueName : public Identifier
{
	char buffer[10];
	UniqueName(size_t index)
	{
		sprintf(buffer, "$%x", index);
		Identifier::value = buffer;
	}
};
typedef std::vector<UniqueName*> UniqueNames;
extern UniqueNames gUniqueNames;

enum ScopeType
{
	SCOPETYPE_UNKNOWN,
	SCOPETYPE_NAMESPACE,
	SCOPETYPE_PROTOTYPE,
	SCOPETYPE_LOCAL,
	SCOPETYPE_CLASS,
	SCOPETYPE_TEMPLATE,
};

extern size_t gScopeCount;

struct ScopeCounter
{
	ScopeCounter()
	{
		++gScopeCount;
	}
	ScopeCounter(const ScopeCounter&)
	{
		++gScopeCount;
	}
	~ScopeCounter()
	{
		--gScopeCount;
	}
};

typedef SafePtr<Scope> ScopePtr;

struct Scope : public ScopeCounter
{
	ScopePtr parent;
	Identifier name;
	size_t enclosedScopeCount; // number of scopes directly enclosed by this scope
	typedef std::less<const char*> IdentifierLess;

	typedef std::multimap<const char*, Declaration, IdentifierLess, TreeAllocator<int> > Declarations2;

	struct Declarations : public Declarations2
	{
		Declarations(const TreeAllocator<int>& allocator)
			: Declarations2(IdentifierLess(), allocator)
		{
		}
		Declarations(const Declarations& other)
			: Declarations2(other)
		{
			SYMBOLS_ASSERT(other.empty());
		}
		~Declarations()
		{
			SYMBOLS_ASSERT(Declarations2::empty());
			// hack: stop declarations being cleared
			new(static_cast<Declarations2*>(this)) Declarations2(IdentifierLess(), TREEALLOCATOR_NULL);
		}
	};


	Declarations declarations;
	ScopeType type;
	Types bases;
	typedef DeferredList<ScopePtr, TreeAllocator<int> > Scopes;
	Scopes usingDirectives;
	Declarations friendDeclarations;

	Scope(const TreeAllocator<int>& allocator, const Identifier& name, ScopeType type = SCOPETYPE_UNKNOWN)
		: parent(0), name(name), enclosedScopeCount(0), declarations(allocator), type(type), bases(allocator), usingDirectives(allocator), friendDeclarations(allocator)
	{
	}

	Identifier& getUniqueName()
	{
		if(enclosedScopeCount == gUniqueNames.size())
		{
			gUniqueNames.push_back(new UniqueName(enclosedScopeCount));
		}
		return *gUniqueNames[enclosedScopeCount++];
	}

	Declaration* insert(const Declaration& other)
	{
		Scope::Declarations::iterator result = declarations.insert(Scope::Declarations::value_type(other.getName().value, other));
		return &(*result).second;
	}
};

inline void undeclareObject(Declaration* declaration, LexerAllocator& allocator)
{
	SYMBOLS_ASSERT(&(*declaration->scope->declarations.find(declaration->getName().value)).second == declaration);
	declaration->scope->declarations.erase(declaration->getName().value);
}

inline DestroyCallback makeUndeclareCallback(Declaration* p)
{
	DestroyCallback result = { DestroyCallbackThunk<Declaration, undeclareObject>::thunk, p };
	return result;
}



const Scope SCOPE_NULL = Scope(TREEALLOCATOR_NULL, IDENTIFIER_NULL);

inline bool enclosesEts(ScopeType type)
{
	return type == SCOPETYPE_NAMESPACE
		|| type == SCOPETYPE_LOCAL;
}




// ----------------------------------------------------------------------------
// built-in symbols

// special-case
extern Declaration gUndeclared;


// meta types
extern Declaration gSpecial;
extern Declaration gClass;
extern Declaration gEnum;

#define TYPE_SPECIAL Type(&gSpecial, TREEALLOCATOR_NULL)
#define TYPE_CLASS Type(&gClass, TREEALLOCATOR_NULL)
#define TYPE_ENUM Type(&gEnum, TREEALLOCATOR_NULL)

// types
extern Declaration gNamespace;

#define TYPE_NAMESPACE Type(&gNamespace, TREEALLOCATOR_NULL)

extern Declaration gCtor;
extern Declaration gEnumerator;
extern Declaration gUnknown;

#define TYPE_CTOR Type(&gCtor, TREEALLOCATOR_NULL)
#define TYPE_ENUMERATOR Type(&gEnumerator, TREEALLOCATOR_NULL)
#define TYPE_UNKNOWN Type(&gUnknown, TREEALLOCATOR_NULL)

// fundamental types
extern Declaration gChar;
extern Declaration gSignedChar;
extern Declaration gUnsignedChar;
extern Declaration gSignedShortInt;
extern Declaration gUnsignedShortInt;
extern Declaration gSignedInt;
extern Declaration gUnsignedInt;
extern Declaration gSignedLongInt;
extern Declaration gUnsignedLongInt;
extern Declaration gSignedLongLongInt;
extern Declaration gUnsignedLongLongInt;
extern Declaration gWCharT;
extern Declaration gBool;
extern Declaration gFloat;
extern Declaration gDouble;
extern Declaration gLongDouble;
extern Declaration gVoid;

inline unsigned combineFundamental(unsigned fundamental, unsigned token)
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

inline Declaration* getFundamentalType(unsigned fundamental)
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

extern Declaration gDependentType;
extern Declaration gDependentObject;
extern Declaration gDependentTemplate;
extern Declaration gDependentNested;

extern Declaration gParam;

#define TYPE_PARAM Type(&gParam, TREEALLOCATOR_NULL)


// objects
extern Identifier gOperatorFunctionId;
extern Identifier gConversionFunctionId;
extern Identifier gOperatorFunctionTemplateId;
// TODO: don't declare if id is anonymous?
extern Identifier gAnonymousId;



inline bool isType(const Declaration& type)
{
	return type.specifiers.isTypedef
		|| type.type.declaration == &gSpecial
		|| type.type.declaration == &gEnum
		|| type.type.declaration == &gClass;
}

inline bool isFunction(const Declaration& declaration)
{
	return declaration.enclosed != 0 && declaration.enclosed->type == SCOPETYPE_PROTOTYPE;
}

inline bool isMemberObject(const Declaration& declaration)
{
	return declaration.scope->type == SCOPETYPE_CLASS
		&& !isFunction(declaration);
}

inline bool isStatic(const Declaration& declaration)
{
	return declaration.specifiers.isStatic;
}

inline bool isStaticMember(const Declaration& declaration)
{
	return isMemberObject(declaration)
		&& isStatic(declaration);
}

inline bool isTypedef(const Declaration& declaration)
{
	return declaration.specifiers.isTypedef;
}

inline bool isClassKey(const Declaration& declaration)
{
	return &declaration == &gClass;
}

inline bool isClass(const Declaration& declaration)
{
	return declaration.type.declaration == &gClass;
}

inline bool isEnum(const Declaration& declaration)
{
	return declaration.type.declaration == &gEnum;
}

inline bool isIncomplete(const Declaration& declaration)
{
	return declaration.enclosed == 0;
}

inline bool isElaboratedType(const Declaration& declaration)
{
	return (isClass(declaration) || isEnum(declaration)) && isIncomplete(declaration);
}

inline bool isNamespace(const Declaration& declaration)
{
	return declaration.type.declaration == &gNamespace;
}

inline bool isObject(const Declaration& declaration)
{
	return !isType(declaration)
		&& !isNamespace(declaration);
}

inline bool isExtern(const Declaration& declaration)
{
	return declaration.specifiers.isExtern;
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
inline const Declaration* getType(const Declaration& declaration)
{
	if(declaration.specifiers.isTypedef)
	{
		return getType(*declaration.type.declaration);
	}
	return declaration.type.declaration;
}


inline const Type& getTemplateArgument(const Type& type, size_t index)
{
	TemplateArguments::const_iterator a = type.arguments.begin();
	Types::const_iterator p = type.declaration->templateParamDefaults.begin();
	for(;; --index)
	{
		if(a != type.arguments.end())
		{
			if(index == 0)
			{
				SYMBOLS_ASSERT((*a).type.declaration != 0);
				return (*a).type;
			}
			++a;
		}
		SYMBOLS_ASSERT(p != type.declaration->templateParamDefaults.end());
		if(index == 0)
		{
			SYMBOLS_ASSERT((*p).declaration != 0);
			return *p;
		}
		++p;
	}
}

inline const Type& getInstantiatedType(const Type& type)
{
	if(type.declaration->specifiers.isTypedef // if this is a typedef..
		&& type.declaration->templateParameter == INDEX_INVALID) // .. and not a template-parameter
	{
		const Type& original = getInstantiatedType(type.declaration->type);

		size_t index = original.declaration->templateParameter;
		if(index != INDEX_INVALID) // if the original type is a template-parameter.
		{
			// Find the template-specialisation it belongs to:
			for(const Type* i = type.qualifying.get(); i != 0; i = (*i).qualifying.get())
			{
				const Type& instantiated = getInstantiatedType(*i);
				if(instantiated.declaration->enclosed == original.declaration->scope)
				{
					return getTemplateArgument(instantiated, index);
				}
			}
		}

		return original;
	}
	return type;
}


inline bool isVisible(Declaration* declaration, const Scope& scope)
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

inline bool isTemplateParameter(Declaration* declaration, const DependentContext& context)
{
	return declaration->templateParameter != INDEX_INVALID && (isVisible(declaration, context.templateParams) || isVisible(declaration, context.enclosing));
}


inline bool isDependent(const Type& type, const DependentContext& context);

inline bool isDependent(const Type* qualifying, const DependentContext& context)
{
	if(qualifying == 0)
	{
		return false;
	}
	const Type& instantiated = getInstantiatedType(*qualifying);
	if(isDependent(instantiated.qualifying.get(), context))
	{
		return true;
	}
	return isDependent(instantiated, context);
}

inline bool isDependent(const Types& bases, const DependentContext& context)
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
inline bool isValueDependent(const Type& type, const DependentContext& context)
{
	if(type.declaration->specifiers.isTypedef)
	{
		return evaluateDependent(type.declaration->valueDependent, context)
			|| isValueDependent(type.declaration->type, context);
	}
	return false;
}

inline bool isDependent(const TemplateArguments& arguments, const DependentContext& context)
{
	for(TemplateArguments::const_iterator i = arguments.begin(); i != arguments.end(); ++i)
	{
		if((*i).type.visited)
		{
			continue;
		}
		if(evaluateDependent((*i).dependent, context) // array-size or constant-initializer
			|| ((*i).type.declaration != 0
				&& isDependent((*i).type, context)))
		{
			return true;
		}
	}
	return false;
}

inline bool isDependentInternal(const Type& type, const DependentContext& context)
{
	if(isValueDependent(type, context))
	{
		return true;
	}
	const Type& original = getInstantiatedType(type);
	if(original.declaration == &gDependentType
		|| original.declaration == &gDependentTemplate)
	{
		if(isDependent(original.qualifying.get(), context))
		{
			return true;
		}
	}
	if(isTemplateParameter(original.declaration, context))
	{
		return true;
	}
	if(original.isImplicitTemplateId)
	{
		if(original.declaration->templateParams.empty())
		{
			// we haven't finished parsing the class-declaration.
			// we can assume 'original' refers to the current-instantation.
			return true;
		}
		for(Types::const_iterator i = original.declaration->templateParams.begin(); i != original.declaration->templateParams.end(); ++i)
		{
			if(isDependent(*i, context))
			{
				return true;
			}
		}
	}
	else
	{
		if(isDependent(original.arguments, context))
		{
			return true;
		}
	}
	Scope* enclosed = original.declaration->enclosed;
	if(enclosed != 0)
	{
		bool result = isDependent(enclosed->bases, context);
		if(!result)
		{
			//std::cout << "not dependent: " << &type << " " << getValue(type.declaration->getName()) << std::endl;
		}
		return result;
	}
	//std::cout << "not dependent: " << &type << " " << getValue(type.declaration->getName()) << std::endl;
	return false;
}

typedef std::set<const Type*> TypeSet;

inline void findTypes(const Type& type, TypeSet& result);

inline void findTypes(const Type* qualifying, TypeSet& result)
{
	if(qualifying == 0)
	{
		return;
	}
	const Type& instantiated = getInstantiatedType(*qualifying);
	findTypes(instantiated.qualifying.get(), result);
	findTypes(instantiated, result);
}

inline void findTypes(const Types& bases, TypeSet& result)
{
	for(Types::const_iterator i = bases.begin(); i != bases.end(); ++i)
	{
		findTypes(*i, result);
	}
}

inline void findTypes(const TemplateArguments& arguments, TypeSet& result)
{
	for(TemplateArguments::const_iterator i = arguments.begin(); i != arguments.end(); ++i)
	{
		if((*i).type.declaration != 0)
		{
			findTypes((*i).type, result);
		}
	}
}

inline void findTypes(const Type& type, TypeSet& result)
{
	if(!result.insert(&type).second)
	{
		return;
	}

	const Type& original = getInstantiatedType(type);
	if(original.declaration == &gDependentType
		|| original.declaration == &gDependentTemplate)
	{
		findTypes(original.qualifying.get(), result);
	}
	if(original.isImplicitTemplateId)
	{
		for(Types::const_iterator i = original.declaration->templateParams.begin(); i != original.declaration->templateParams.end(); ++i)
		{
			findTypes(*i, result);
		}
	}
	else
	{
		findTypes(original.arguments, result);
	}
	Scope* enclosed = original.declaration->enclosed;
	if(enclosed != 0)
	{
		findTypes(enclosed->bases, result);
	}
}

inline bool isDependentNonRecursive(const Type& type, const DependentContext& context)
{
	if(isValueDependent(type, context))
	{
		return true;
	}
	const Type& original = getInstantiatedType(type);
	if(isTemplateParameter(original.declaration, context))
	{
		return true;
	}
	if(original.isImplicitTemplateId)
	{
		if(original.declaration->templateParams.empty())
		{
			// we haven't finished parsing the class-declaration.
			// we can assume 'original' refers to the current-instantation.
			return true;
		}
	}
	else
	{
		for(TemplateArguments::const_iterator i = original.arguments.begin(); i != original.arguments.end(); ++i)
		{
			if(evaluateDependent((*i).dependent, context)) // array-size or constant-initializer
			{
				return true;
			}
		}
	}	
	return false;
}

inline bool isDependentFast(const Type& type, const DependentContext& context)
{
	TypeSet types;
	findTypes(type, types);
	for(TypeSet::const_iterator i = types.begin(); i != types.end(); ++i)
	{
		if(isDependentNonRecursive(*(*i), context))
		{
			return true;
		}
	}
	return false;
}



inline bool isDependent(const Type& type, const DependentContext& context)
{
	type.visited = true;
	bool result = isDependentInternal(type, context);
#if 0
	bool alternative = isDependentFast(type, context);
	if(result != alternative)
	{
		__debugbreak();
	}
#endif
	type.visited = false;
	return result;
}

inline bool isDependentName(Declaration* declaration, const DependentContext& context)
{
	return isTemplateParameter(declaration, context)
		|| isDependent(declaration->type, context)
		|| evaluateDependent(declaration->valueDependent, context);
}

inline bool isDependentType(Type* type, const DependentContext& context)
{
	return isDependent(*type, context);
}

inline bool isDependentClass(Scope* scope, const DependentContext& context)
{
	return isDependent(scope->bases, context);
}

inline const char* getDeclarationType(const Declaration& declaration)
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

inline bool isAnonymous(const Declaration& declaration)
{
	return *declaration.getName().value == '$';
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
		throw SymbolsError(); // should not be reachable
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

inline Declaration* findPrimaryTemplate(Declaration* declaration)
{
	SYMBOLS_ASSERT(declaration->isTemplate);
	for(;declaration != 0; declaration = declaration->overloaded)
	{
		if(declaration->arguments.empty())
		{
			SYMBOLS_ASSERT(declaration->isTemplate);
			return declaration;
		}
	}
	SYMBOLS_ASSERT(false); // primary template not declared!
	return 0;
}

inline bool isEqual(const Types& params, const TemplateArguments& left, const TemplateArguments& right);

inline bool isEqual(const Type& l, const Type& r)
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

inline bool isEqual(const Types& params, const TemplateArguments& left, const TemplateArguments& right)
{
	TemplateArguments::const_iterator l = left.begin();
	TemplateArguments::const_iterator r = right.begin();
	Types::const_iterator p = params.begin();
	for(; p != params.end(); ++p)
	{
		if(!isEqual(
			l != left.end() ? (*l).type : *p,
			r != right.end() ? (*r).type : *p
		))
		{
			return false;
		}
		if(l != left.end())
		{
			++l;
		}
		if(r != right.end())
		{
			++r;
		}
	}
	return true;
}

inline Declaration* findTemplateSpecialization(Declaration* declaration, const TemplateArguments& arguments)
{
	SYMBOLS_ASSERT(declaration->isTemplate);
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







struct LookupFilter
{
	typedef bool (*Function)(void* context, const Declaration& declaration);
	Function function;
	void* context;

	bool operator()(const Declaration& declaration)
	{
		return function(context, declaration);
	}
};

inline bool isAny(const Declaration& declaration)
{
	// always ignore constructors during name-lookup
	return declaration.type.declaration != &gCtor;
}

template<bool filter(const Declaration& declaration)>
struct LookupFilterDefault : LookupFilter
{
	LookupFilterDefault()
	{
		LookupFilter::function = apply;
		LookupFilter::context = 0;
	}
	static bool apply(void*, const Declaration& declaration)
	{
		return filter(declaration);
	}
};

typedef LookupFilterDefault<isAny> IsAny;

template<typename T>
struct LookupFilterThunk
{
	static bool apply(void* context, const Declaration& declaration)
	{
		return (*static_cast<T*>(context))(declaration);
	}
};

template<typename T>
LookupFilter makeLookupFilter(T& filter)
{
	LookupFilter result = { LookupFilterThunk<T>::apply, &filter };
	return result;
}


struct LookupResult
{
	Declaration* filtered; // the declaration found by the name-lookup, using the filter

	LookupResult()
		: filtered(0)
	{
	}
	operator Declaration*() const
	{
		return filtered;
	}

	// Combines the result of a subsequent lookup, returns true if lookup succeeded
	bool append(const LookupResult& other)
	{
		filtered = other.filtered;
		return filtered != 0;
	}
};


inline LookupResult findDeclaration(Scope::Declarations& declarations, const Identifier& id, LookupFilter filter = IsAny(), bool isBase = false)
{
	Scope::Declarations::iterator i = declarations.upper_bound(id.value);

	LookupResult result;
	
	for(; i != declarations.begin()
		&& (*--i).first == id.value;)
	{
		if(!isBase
			|| (*i).second.templateParameter == INDEX_INVALID) // template-params of base-class are not visible outside the class
		{
			if(filter((*i).second))
			{
				result.filtered = &(*i).second;
				break;
			}
		}
	}

	return result;
}

inline LookupResult findDeclaration(Scope::Declarations& declarations, Types& bases, const Identifier& id, LookupFilter filter = IsAny(), bool isBase = false);

inline LookupResult findDeclaration(Types& bases, const Identifier& id, LookupFilter filter = IsAny())
{
	LookupResult result;
	for(Types::iterator i = bases.begin(); i != bases.end(); ++i)
	{
		Scope* scope = getInstantiatedType(*i).declaration->enclosed;
		if(scope != 0)
		{
			/* namespace.udir
			A using-directive shall not appear in class scope, but may appear in namespace scope or in block scope.
			*/
			SYMBOLS_ASSERT(scope->usingDirectives.empty());
			if(result.append(findDeclaration(scope->declarations, scope->bases, id, filter, true)))
			{
				return result;
			}
		}
	}
	return result;
}

inline LookupResult findDeclaration(Scope::Scopes& scopes, const Identifier& id, LookupFilter filter = IsAny())
{
	LookupResult result;
	for(Scope::Scopes::iterator i = scopes.begin(); i != scopes.end(); ++i)
	{
		Scope& scope = *(*i);
		SYMBOLS_ASSERT(scope.bases.empty());

#ifdef LOOKUP_DEBUG
		std::cout << "searching '";
		printName(scope);
		std::cout << "'" << std::endl;
#endif

		if(result.append(findDeclaration(scope.declarations, scope.bases, id, filter)))
		{
			return result;
		}
		if(result.append(findDeclaration(scope.usingDirectives, id, filter)))
		{
			return result;
		}
	}
	return result;
}

inline LookupResult findDeclaration(Scope::Declarations& declarations, Types& bases, const Identifier& id, LookupFilter filter, bool isBase)
{
	LookupResult result;
	if(result.append(findDeclaration(declarations, id, filter, isBase)))
	{
		return result;
	}
	if(result.append(findDeclaration(bases, id, filter)))
	{
		return result;
	}
	return result;
}

inline LookupResult findDeclaration(Scope& scope, const Identifier& id, LookupFilter filter = IsAny())
{
#ifdef LOOKUP_DEBUG
	std::cout << "searching '";
	printName(scope);
	std::cout << "'" << std::endl;
#endif

	LookupResult result;
	if(result.append(findDeclaration(scope.declarations, scope.bases, id, filter)))
	{
		return result;
	}
	if(scope.parent != 0)
	{
		if(result.append(findDeclaration(*scope.parent, id, filter)))
		{
			return result;
		}
	}
	/* basic.lookup.unqual
	The declarations from the namespace nominated by a using-directive become visible in a namespace enclosing
	the using-directive; see 7.3.4. For the purpose of the unqualified name lookup rules described in 3.4.1, the
	declarations from the namespace nominated by the using-directive are considered members of that enclosing
	namespace.
	*/
	result.append(findDeclaration(scope.usingDirectives, id, filter));
	return result;
}


#endif


