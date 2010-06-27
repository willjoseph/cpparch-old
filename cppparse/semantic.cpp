

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

struct ListNodeBase
{
	ListNodeBase* next;
};
template<typename T>
struct ListNode : ListNodeBase
{
	T value;
	ListNode(const T& value)
		: value(value)
	{
	}
};

template<typename T>
struct ListIterator
{
	typedef ListNode<T> Node;
	const ListNodeBase* p;
	ListIterator(const ListNodeBase* p) : p(p)
	{
	}

	const T& operator*() const
	{
		return static_cast<const Node*>(p)->value;
	}
	const T* operator->() const
	{
		return &static_cast<const Node*>(p)->value;
	}

	ListIterator<T>& operator++()
	{
		p = p->next;
		return *this;
	}
	ListIterator<T> operator++(int)
	{
		ListIterator<T> tmp = *this;
		++*this;
		return tmp;
	}
};

template<typename T>
inline bool operator==(const ListIterator<T>& left, const ListIterator<T>& right)
{
	return left.p == right.p;
}
template<typename T>
inline bool operator!=(const ListIterator<T>& left, const ListIterator<T>& right)
{
	return left.p != right.p;
}

template<typename T, typename A>
struct List : private A
{
	typedef ListNode<T> Node;
	ListNodeBase head;
	ListNodeBase* tail;
	typedef typename A::template rebind<Node>::other Allocator;

	List()
	{
		init_raw();
	}
	List(const A& allocator) :  A(allocator)
	{
		init_raw();
	}
	~List()
	{
		for(ListNodeBase* p = head.next; p != &head; )
		{
			ListNodeBase* next = p->next;
			allocatorDelete(*this, static_cast<Node*>(p));
			p = next;
		}
	}
	void init_raw()
	{
		head.next = tail = &head;
	}
	void copy_raw(const List& other)
	{
		if(other.empty())
		{
			new(this) List();
		}
		else
		{
			head = other.head;
			tail = other.tail;
			tail->next = &head;
		}
	}
private:
	List(const List& other);
	List& operator=(const List& other);
public:

	typedef ListIterator<T> const_iterator;

	const_iterator begin() const
	{
		return const_iterator(head.next);
	}
	const_iterator end() const
	{
		return const_iterator(&head);
	}
	bool empty() const
	{
		return head.next == &head;
	}
	T& back()
	{
		return static_cast<Node*>(tail)->value;
	}
	
	void push_back(const T& value)
	{
		Node* node = allocatorNew(*this, value);
		node->next = &head;
		tail->next = node;
		tail = node;
	}
	void splice(const_iterator position, List& other)
	{
		// always splice at end for now
		if(!other.empty())
		{
			tail->next = other.head.next;
			tail = other.tail;
			tail->next = &head;
		}
	}
	void swap(List& other)
	{
		std::swap(head, other.head);
		std::swap(tail, other.tail);
		if(head.next != &other.head)
		{
			tail->next = &head;
		}
		else
		{
			init_raw();
		}
		if(other.head.next != &head)
		{
			other.tail->next = &other.head;
		}
		else
		{
			other.init_raw();
		}
	}
};

#define TREEALLOCATOR_LINEAR

#ifdef TREEALLOCATOR_LINEAR
#define TreeAllocator LinearAllocatorWrapper
#define TREEALLOCATOR_NULL TreeAllocator<int>(NullAllocator())
#else
#define TreeAllocator DebugAllocator
#define TREEALLOCATOR_NULL TreeAllocator<int>()
#endif

#if 1
typedef std::list<struct TemplateArgument, TreeAllocator<struct TemplateArgument> > TemplateArguments2;

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
};
#else
typedef List<struct TemplateArgument> TemplateArguments;
#endif

typedef std::list<struct Type, TreeAllocator<int> > Types2;

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

template<typename T, typename A>
inline T* allocatorNew(const A& a, const T& value)
{
	typename A::template rebind<T>::other tmp(a);
	T* p = tmp.allocate(1);
	tmp.construct(p, value);
	return p;
}

template<typename T, typename A>
void allocatorDelete(const A& a, T* p)
{
	if(p != 0)
	{
		typename A::template rebind<T>::other tmp(a);
		tmp.destroy(p);
		tmp.deallocate(p, 1);
	}
}


template<typename T, typename A>
class Copied : private A
{
	T* p;

	A& getAllocator()
	{
		return *this;
	}
public:
	Copied(const A& allocator)
		: A(allocator), p(0)
	{
	}
	~Copied()
	{
		allocatorDelete(getAllocator(), p);
	}
	Copied(const T& value, const A& allocator)
		: A(allocator), p(allocatorNew(getAllocator(), value))
	{
	}
	Copied(const Copied& other)
		: A(other), p(other.p == 0 ? 0 : allocatorNew(getAllocator(), *other.p))
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
		Copied tmp(value, getAllocator());
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

namespace std
{
	template<typename T, typename A>
	void swap(Copied<T, A>& left, Copied<T, A>& right)
	{
		left.swap(right);
	}
}

typedef Copied<Type, TreeAllocator<int> > Qualifying;

struct Type
{
	Declaration* declaration;
	TemplateArguments arguments;
	Qualifying qualifying;
	mutable bool visited;
	Type(Declaration* declaration, const TreeAllocator<int>& allocator)
		: declaration(declaration), arguments(allocator), qualifying(allocator), visited(false)
	{
	}
	void swap(Type& other)
	{
		std::swap(declaration, other.declaration);
		std::swap(arguments, other.arguments);
		std::swap(qualifying, other.qualifying);
	}
	Type& operator=(Declaration* declaration)
	{
		SEMANTIC_ASSERT(arguments.empty());
		SEMANTIC_ASSERT(qualifying.empty());
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

typedef Copied<Type, TreeAllocator<int> > TypeRef;

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

typedef std::list<DependencyNode, TreeAllocator<int> > Dependent2;

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

bool evaluateDependent(const Dependent& dependent, const DependentContext& context)
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


const size_t INDEX_INVALID = size_t(-1);

#define TEMPLATEARGUMENTS_NULL TemplateArguments(TREEALLOCATOR_NULL)

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

	Declaration(
		const TreeAllocator<int>& allocator,
		Scope* scope,
		Identifier name,
		const Type& type,
		Scope* enclosed,
		DeclSpecifiers specifiers = DeclSpecifiers(),
		bool isTemplate = false,
		const TemplateArguments& arguments = TEMPLATEARGUMENTS_NULL,
		size_t templateParameter = INDEX_INVALID,
		const Dependent& valueDependent = DEPENDENT_NULL
	) : scope(scope),
		name(name),
		type(type),
		enclosed(enclosed),
		overloaded(0),
		valueDependent(valueDependent),
		specifiers(specifiers),
		templateParameter(templateParameter),
		templateParamDefaults(allocator),
		arguments(arguments),
		isTemplate(isTemplate)
	{
	}
};

cpp::terminal_identifier& getDeclarationId(Declaration* declaration)
{
	return declaration->name;
}

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

size_t gScopeCount = 0;

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

struct Scope : public ScopeCounter
{
	Scope* parent;
	Identifier name;
	size_t enclosedScopeCount; // number of scopes directly enclosed by this scope
	typedef std::less<const char*> IdentifierLess;
	typedef std::multimap<const char*, Declaration, IdentifierLess, TreeAllocator<int> > DeclarationMap;
	struct Declarations : DeclarationMap
	{
		struct CacheEntry
		{
			const char* id;
			DeclarationMap::iterator value;
		};
		CacheEntry recent;

		Declarations(const IdentifierLess& predicate, const TreeAllocator<int>& allocator)
			: DeclarationMap(predicate, allocator)
		{
		}

		iterator insert(const value_type& value)
		{
			recent.id = 0;
			return DeclarationMap::insert(value);
		}
		void erase(const char* key)
		{
			recent.id = 0;
			DeclarationMap::erase(key);
		}
	};
	Declarations declarations;
	ScopeType type;
	Types bases;
	typedef std::list<Scope*, TreeAllocator<int> > Scopes;
	Scopes usingDirectives;

	Scope(const TreeAllocator<int>& allocator, const Identifier& name, ScopeType type = SCOPETYPE_UNKNOWN)
		: parent(0), name(name), enclosedScopeCount(0), declarations(IdentifierLess(), allocator), type(type), bases(allocator), usingDirectives(allocator)
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

#define TYPE_NULL Type(0, TREEALLOCATOR_NULL)

// special-case
Declaration gUndeclared(TREEALLOCATOR_NULL, 0, makeIdentifier("$undeclared"), TYPE_NULL, 0);
Declaration gFriend(TREEALLOCATOR_NULL, 0, makeIdentifier("$friend"), TYPE_NULL, 0);


// meta types
Declaration gSpecial(TREEALLOCATOR_NULL, 0, makeIdentifier("$special"), TYPE_NULL, 0);
Declaration gClass(TREEALLOCATOR_NULL, 0, makeIdentifier("$class"), TYPE_NULL, 0);
Declaration gEnum(TREEALLOCATOR_NULL, 0, makeIdentifier("$enum"), TYPE_NULL, 0);

#define TYPE_SPECIAL Type(&gSpecial, TREEALLOCATOR_NULL)
#define TYPE_CLASS Type(&gClass, TREEALLOCATOR_NULL)
#define TYPE_ENUM Type(&gEnum, TREEALLOCATOR_NULL)

// types
Declaration gNamespace(TREEALLOCATOR_NULL, 0, makeIdentifier("$namespace"), TYPE_NULL, 0);

#define TYPE_NAMESPACE Type(&gNamespace, TREEALLOCATOR_NULL)

Declaration gCtor(TREEALLOCATOR_NULL, 0, makeIdentifier("$ctor"), TYPE_SPECIAL, 0);
Declaration gEnumerator(TREEALLOCATOR_NULL, 0, makeIdentifier("$enumerator"), TYPE_SPECIAL, 0);
Declaration gUnknown(TREEALLOCATOR_NULL, 0, makeIdentifier("$unknown"), TYPE_SPECIAL, 0);

#define TYPE_CTOR Type(&gCtor, TREEALLOCATOR_NULL)
#define TYPE_ENUMERATOR Type(&gEnumerator, TREEALLOCATOR_NULL)
#define TYPE_UNKNOWN Type(&gUnknown, TREEALLOCATOR_NULL)

Declaration gChar(TREEALLOCATOR_NULL, 0, makeIdentifier("$char"), TYPE_SPECIAL, 0);
Declaration gSignedChar(TREEALLOCATOR_NULL, 0, makeIdentifier("$signed-char"), TYPE_SPECIAL, 0);
Declaration gUnsignedChar(TREEALLOCATOR_NULL, 0, makeIdentifier("$unsigned-char"), TYPE_SPECIAL, 0);
Declaration gSignedShortInt(TREEALLOCATOR_NULL, 0, makeIdentifier("$signed-short-int"), TYPE_SPECIAL, 0);
Declaration gUnsignedShortInt(TREEALLOCATOR_NULL, 0, makeIdentifier("$unsigned-short-int"), TYPE_SPECIAL, 0);
Declaration gSignedInt(TREEALLOCATOR_NULL, 0, makeIdentifier("$signed-int"), TYPE_SPECIAL, 0);
Declaration gUnsignedInt(TREEALLOCATOR_NULL, 0, makeIdentifier("$unsigned-int"), TYPE_SPECIAL, 0);
Declaration gSignedLongInt(TREEALLOCATOR_NULL, 0, makeIdentifier("$signed-long-int"), TYPE_SPECIAL, 0);
Declaration gUnsignedLongInt(TREEALLOCATOR_NULL, 0, makeIdentifier("$unsigned-long-int"), TYPE_SPECIAL, 0);
Declaration gSignedLongLongInt(TREEALLOCATOR_NULL, 0, makeIdentifier("$signed-long-long-int"), TYPE_SPECIAL, 0);
Declaration gUnsignedLongLongInt(TREEALLOCATOR_NULL, 0, makeIdentifier("$unsigned-long-long-int"), TYPE_SPECIAL, 0);
Declaration gWCharT(TREEALLOCATOR_NULL, 0, makeIdentifier("$wchar_t"), TYPE_SPECIAL, 0);
Declaration gBool(TREEALLOCATOR_NULL, 0, makeIdentifier("$bool"), TYPE_SPECIAL, 0);
Declaration gFloat(TREEALLOCATOR_NULL, 0, makeIdentifier("$float"), TYPE_SPECIAL, 0);
Declaration gDouble(TREEALLOCATOR_NULL, 0, makeIdentifier("$double"), TYPE_SPECIAL, 0);
Declaration gLongDouble(TREEALLOCATOR_NULL, 0, makeIdentifier("$long-double"), TYPE_SPECIAL, 0);
Declaration gVoid(TREEALLOCATOR_NULL, 0, makeIdentifier("$void"), TYPE_SPECIAL, 0);

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

Declaration gDependentType(TREEALLOCATOR_NULL, 0, makeIdentifier("$type"), TYPE_SPECIAL, 0);
Declaration gDependentObject(TREEALLOCATOR_NULL, 0, makeIdentifier("$object"), TYPE_UNKNOWN, 0);
Declaration gDependentTemplate(TREEALLOCATOR_NULL, 0, makeIdentifier("$template"), TYPE_SPECIAL, 0, DeclSpecifiers(), true);
Declaration gDependentNested(TREEALLOCATOR_NULL, 0, makeIdentifier("$nested"), TYPE_SPECIAL, 0);

Declaration gParam(TREEALLOCATOR_NULL, 0, makeIdentifier("$param"), TYPE_CLASS, 0);

#define TYPE_PARAM Type(&gParam, TREEALLOCATOR_NULL)

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

#if 0
const Declaration* getBaseType(const Declaration& declaration)
{
	if(declaration.type.declaration->specifiers.isTypedef)
	{
		return getBaseType(*declaration.type.declaration);
	}
	return declaration.type.declaration;
}
#endif

const Type& getTemplateArgument(const Type& type, size_t index)
{
#if 1
	TemplateArguments::const_iterator a = type.arguments.begin();
	Types::const_iterator p = type.declaration->templateParamDefaults.begin();
	for(;; --index)
	{
		if(a != type.arguments.end())
		{
			if(index == 0)
			{
				SEMANTIC_ASSERT((*a).type.declaration != 0);
				return (*a).type;
			}
			++a;
		}
		SEMANTIC_ASSERT(p != type.declaration->templateParamDefaults.end());
		if(index == 0)
		{
			SEMANTIC_ASSERT((*p).declaration != 0);
			return *p;
		}
		++p;
	}
#else
	if(index < type.arguments.size())
	{
		SEMANTIC_ASSERT(type.arguments[index].type.declaration != 0);
		return type.arguments[index].type;
	}
	else
	{
		SEMANTIC_ASSERT(index < type.declaration->templateParamDefaults.size()
			&& type.declaration->templateParamDefaults[index].declaration != 0);
		return type.declaration->templateParamDefaults[index];
	}
#endif
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
					return getTemplateArgument(instantiated, index);
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
	return &declaration == &gClass;
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

bool isDependent(const Type* qualifying, const DependentContext& context)
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
		return evaluateDependent(type.declaration->valueDependent, context)
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
		if(evaluateDependent((*i).dependent, context) // array-size or constant-initializer
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
		|| evaluateDependent(declaration->valueDependent, context);
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
#if 1
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
#else
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
#endif
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

void printName(const Scope& scope)
{
	if(scope.parent != 0)
	{
		printName(*scope.parent);
		std::cout << "::";
		std::cout << getValue(scope.name);
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

bool isAny(const Declaration& declaration)
{
	return declaration.type.declaration != &gCtor;
}

typedef bool (*LookupFilter)(const Declaration&);

Declaration* findDeclaration(Scope::Declarations& declarations, const Identifier& id, LookupFilter filter = isAny)
{
	Scope::Declarations::iterator i =
#if 0 // disabled due to bug
		declarations.recent.id == id.value
		? declarations.recent.value
		: 
#endif
	declarations.upper_bound(id.value);
	
	declarations.recent.id = id.value;
	declarations.recent.value = i;

	for(; i != declarations.begin()
		&& (*--i).first == id.value;)
	{
		if(filter((*i).second))
		{
			return &(*i).second;
		}
	}
	return 0;
}

Declaration* findDeclaration(Scope::Declarations& declarations, Types& bases, const Identifier& id, LookupFilter filter = isAny);

Declaration* findDeclaration(Types& bases, const Identifier& id, LookupFilter filter = isAny)
{
	for(Types::iterator i = bases.begin(); i != bases.end(); ++i)
	{
		Scope* scope = getInstantiatedType(*i).declaration->enclosed;
		if(scope != 0)
		{
			/* namespace.udir
			A using-directive shall not appear in class scope, but may appear in namespace scope or in block scope.
			*/
			SEMANTIC_ASSERT(scope->usingDirectives.empty());
			Declaration* result = findDeclaration(scope->declarations, scope->bases, id, filter);
			if(result != 0)
			{
				return result;
			}
		}
	}
	return 0;
}

Declaration* findDeclaration(Scope::Scopes& scopes, const Identifier& id, LookupFilter filter = isAny)
{
	for(Scope::Scopes::iterator i = scopes.begin(); i != scopes.end(); ++i)
	{
		Scope& scope = *(*i);
		SEMANTIC_ASSERT(scope.bases.empty());

#ifdef LOOKUP_DEBUG
		std::cout << "searching '";
		printName(scope);
		std::cout << "'" << std::endl;
#endif

		{
			Declaration* result = findDeclaration(scope.declarations, scope.bases, id, filter);
			if(result != 0)
			{
				return result;
			}
		}
		{
			Declaration* result = findDeclaration(scope.usingDirectives, id, filter);
			if(result != 0)
			{
				return result;
			}
		}
	}
	return 0;
}

Declaration* findDeclaration(Scope::Declarations& declarations, Types& bases, const Identifier& id, LookupFilter filter)
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
		if(result != 0
			&& result->templateParameter == INDEX_INVALID) // template-params of base-class are not visible outside the class
		{
			return result;
		}
	}
	return 0;
}

Declaration* findDeclaration(Scope& scope, const Identifier& id, LookupFilter filter = isAny)
{
#ifdef LOOKUP_DEBUG
	std::cout << "searching '";
	printName(scope);
	std::cout << "'" << std::endl;
#endif

	Declaration* result = findDeclaration(scope.declarations, scope.bases, id, filter);
	if(result != 0)
	{
		return result;
	}
	if(scope.parent != 0)
	{
		Declaration* result = findDeclaration(*scope.parent, id, filter);
		if(result != 0)
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
	return findDeclaration(scope.usingDirectives, id, filter);
}


struct WalkerContext : public TreeAllocator<int>
{
	Scope global;
	Declaration globalDecl;
	Type globalType;

	WalkerContext(const TreeAllocator<int>& allocator) :
		TreeAllocator<int>(allocator),
		global(allocator, makeIdentifier("$global"), SCOPETYPE_NAMESPACE),
		globalDecl(allocator, 0, makeIdentifier("$global"), TYPE_NULL, &global),
		globalType(&globalDecl, allocator)
	{
	}
};

typedef std::list< DeferredParse<struct WalkerBase, struct WalkerState> > DeferredSymbols;


typedef bool (*IdentifierFunc)(const Declaration& declaration);
const char* getIdentifierType(IdentifierFunc func);

const IdentifierMismatch IDENTIFIERMISMATCH_NULL = IdentifierMismatch(IDENTIFIER_NULL, 0, 0);
IdentifierMismatch gIdentifierMismatch = IDENTIFIERMISMATCH_NULL;

struct WalkerState
#ifdef MINGLE
	: public ContextBase
#endif
{
	typedef WalkerState State;

	WalkerContext& context;
	Scope* enclosing;
	const Type* qualifying_p;
	Scope* templateParams;
	Scope* templateEnclosing;
	bool* ambiguity;
	DeferredSymbols* deferred;

	WalkerState(WalkerContext& context)
		: context(context), enclosing(0), qualifying_p(0), templateParams(0), templateEnclosing(0), ambiguity(0), deferred(0)
	{
	}
	const WalkerState& getState() const
	{
		return *this;
	}

	Declaration* findDeclaration(const Identifier& id, LookupFilter filter = isAny)
	{
		ProfileScope profile(gProfileLookup);
#ifdef LOOKUP_DEBUG
		std::cout << "lookup: " << getValue(id) << " (" << getIdentifierType(filter) << ")" << std::endl;
#endif
		if(getQualifyingScope() != 0)
		{
			Declaration* result = ::findDeclaration(*getQualifyingScope(), id, filter);
			if(result != 0)
			{
#ifdef LOOKUP_DEBUG
				std::cout << "HIT: qualified" << std::endl;
#endif
				return result;
			}
		}
		else
		{
#if 1
			if(templateParams != 0)
			{
				Declaration* result = ::findDeclaration(*templateParams, id, filter);
				if(result != 0)
				{
#ifdef LOOKUP_DEBUG
					std::cout << "HIT: templateParams" << std::endl;
#endif
					return result;
				}
			}
#endif
			Declaration* result = ::findDeclaration(*enclosing, id, filter);
			if(result != 0)
			{
#ifdef LOOKUP_DEBUG
				std::cout << "HIT: unqualified" << std::endl;
#endif
				return result;
			}
		}
#ifdef LOOKUP_DEBUG
		std::cout << "FAIL" << std::endl;
#endif
		return &gUndeclared;
	}

	Declaration* pointOfDeclaration(
		const TreeAllocator<int>& allocator,
		Scope* parent,
		const Identifier& name,
		const Type& type,
		Scope* enclosed,
		DeclSpecifiers specifiers = DeclSpecifiers(),
		bool isTemplate = false,
		const TemplateArguments& arguments = TEMPLATEARGUMENTS_NULL,
		size_t templateParameter = INDEX_INVALID,
		const Dependent& valueDependent = DEPENDENT_NULL)
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
		
		Declaration other(allocator, parent, name, type, enclosed, specifiers, isTemplate, arguments, templateParameter, valueDependent);
		if(name.value != 0) // unnamed class/struct/union/enum
		{
			/* 3.4.4-1
			An elaborated-type-specifier (7.1.6.3) may be used to refer to a previously declared class-name or enum-name
			even though the name has been hidden by a non-type declaration (3.3.10).
			*/
			Declaration* declaration = ::findDeclaration(parent->declarations, name);
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

		return &(*parent->declarations.insert(Scope::Declarations::value_type(name.value, other))).second;
	}

	TreeAllocator<int> getAllocator()
	{
#ifdef TREEALLOCATOR_LINEAR
		return parser->lexer.allocator;
#else
		return DebugAllocator<int>();
#endif
	}

	Declaration* declareObject(Scope* parent, Identifier* id, const Type& type, Scope* enclosed, DeclSpecifiers specifiers, size_t templateParameter, const Dependent& valueDependent)
	{
		Declaration* declaration = pointOfDeclaration(context, parent, *id, type, enclosed, specifiers, enclosing == templateEnclosing, TEMPLATEARGUMENTS_NULL, templateParameter, valueDependent); // 3.3.1.1
		if(id != &gAnonymousId)
		{
			id->dec.p = declaration;
		}
		return declaration;
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
		if(qualifying_p == 0)
		{
			return 0;
		}
		return getInstantiatedType(*qualifying_p).declaration->enclosed;
	}

	void clearQualifying()
	{
		qualifying_p = 0;
	}

	template<typename T>
	void reportIdentifierMismatch(T* symbol, const Identifier& id, Declaration* declaration, const char* expected)
	{
#ifdef MINGLE
		if(!IsConcrete<T>::RESULT)
		{
			deleteSymbol(symbol, parser->lexer.allocator);
		}
		result = 0;
		gIdentifierMismatch = IdentifierMismatch(id, declaration, expected);
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
		return ::isDependent(type, DependentContext(*enclosing, templateParams != 0 ? *templateParams : Scope(getAllocator(), IDENTIFIER_NULL)));
	}

	bool isDependent(const Types& bases)
	{
		return ::isDependent(bases, DependentContext(*enclosing, templateParams != 0 ? *templateParams : Scope(getAllocator(), IDENTIFIER_NULL)));
	}

	bool isDependent(const Type* qualifying)
	{
		return ::isDependent(qualifying, DependentContext(*enclosing, templateParams != 0 ? *templateParams : Scope(getAllocator(), IDENTIFIER_NULL)));
	}


	void addDependent(Dependent& dependent, const DependencyCallback& callback)
	{
		dependent.push_back(DependencyNode(callback, context));
	}
	void addDependentName(Dependent& dependent, Declaration* declaration)
	{
		addDependent(dependent, makeDependencyCallback(declaration, isDependentName));
	}
	void addDependentType(Dependent& dependent, Declaration* declaration)
	{
		addDependent(dependent, makeDependencyCallback(&declaration->type, isDependentType));
	}
	void addDependent(Dependent& dependent, Type& type)
	{
		TypeRef tmp(TYPE_NULL, context);
		tmp.back().swap(type);
		addDependent(dependent, makeDependencyCallback(tmp.get(), isDependentType));
		dependent.back().type.swap(tmp);
	}
	void addDependent(Dependent& dependent, Scope* scope)
	{
		addDependent(dependent, makeDependencyCallback(scope, isDependentClass));
	}
};


struct WalkerBase : public WalkerState
{
	typedef WalkerBase Base;

#if 1
	typedef std::list<Scope, TreeAllocator<int> > Scopes;
#else
	typedef List<Scope, TreeAllocator<int> > Scopes;
#endif

	// Contains all elaborated-type-specifiers declared during the current parse.
	// If these declarations have been added to a scope that has already been committed,
	// when the parse fails, these declarations must be removed from their containing scope.
	struct Declarations
	{
		typedef std::list<Declaration*, TreeAllocator<int> > List;

		List declarations;

		typedef List::iterator iterator;
		iterator begin()
		{
			return declarations.begin();
		}
		iterator end()
		{
			return declarations.end();
		}

		Declarations(const TreeAllocator<int>& allocator)
			: declarations(allocator)
		{
		}
	private:
		Declarations(const Declarations&);
		Declarations operator=(const Declarations&);
	public:
		~Declarations()
		{
			for(Declarations::iterator i = declarations.begin(); i != declarations.end(); ++i)
			{
				SEMANTIC_ASSERT(&(*(*i)->scope->declarations.find((*i)->name.value)).second == (*i));

				(*i)->scope->declarations.erase((*i)->name.value);
			}
		}

		void push_back(Declaration* declaration)
		{
			declarations.push_back(declaration);
		}
		void splice(iterator pos, Declarations& other)
		{
			declarations.splice(pos, other.declarations);
		}
		void swap(Declarations& other)
		{
			declarations.swap(other.declarations);
		}
		void erase(iterator first, iterator last)
		{
			declarations.erase(first, last);
		}
	};

#ifdef ALLOCATOR_DEBUG
	Scopes scopes; // under-construction scopes
#endif
	Declarations declarations; // declarations must be destroyed before scopes!

	WalkerBase(WalkerContext& context)
		: WalkerState(context),
#ifdef ALLOCATOR_DEBUG
		scopes(context),
#endif
		declarations(context)
	{
	}
	WalkerBase(const WalkerState& state)
		: WalkerState(state),
#ifdef ALLOCATOR_DEBUG
		scopes(context),
#endif
		declarations(context)
	{
	}
	Scope* newScope(const Identifier& name, ScopeType type = SCOPETYPE_UNKNOWN)
	{
#ifdef ALLOCATOR_DEBUG
		scopes.push_back(Scope(context, name, type));
		return &scopes.back();
#else
		return allocatorNew(context, Scope(context, name, type));
#endif
	}
	void hit(WalkerBase& other)
	{
#ifdef ALLOCATOR_DEBUG
		scopes.splice(scopes.end(), other.scopes);
#endif
		declarations.splice(declarations.end(), other.declarations);
	}

	Declaration* declareClass(Identifier* id, const TemplateArguments& arguments)
	{
		Scope* enclosed = templateParams != 0 ? templateParams : newScope(makeIdentifier("$class"));
		enclosed->type = SCOPETYPE_CLASS; // convert template-param-scope to class-scope if present
		Declaration* declaration = pointOfDeclaration(context, enclosing, id == 0 ? makeIdentifier(enclosing->getUniqueName()) : *id, TYPE_CLASS, enclosed, DeclSpecifiers(), enclosing == templateEnclosing, arguments);
		if(id != 0)
		{
			id->dec.p = declaration;
		}
		enclosed->name = declaration->name;
		return declaration;
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
			Declaration* declaration = pointOfDeclaration(context, getEtsScope(), *forward, TYPE_CLASS, 0, DeclSpecifiers(), enclosing == templateEnclosing);
			forward->dec.p = declaration;
			type = declaration;

			declarations.push_back(declaration);
		}
	}
};

// saves and restores the state in walker-base
#if 0 // TODO: this optimisation is broken: fix it!
struct ScopeGuard
{
	WalkerBase& base;
#ifdef ALLOCATOR_DEBUG
	WalkerBase::Scopes::iterator scopes;
#endif
	WalkerBase::Declarations::iterator declarations;
	ScopeGuard(WalkerBase& base)
		: base(base),
#ifdef ALLOCATOR_DEBUG
		scopes(base.scopes.end()),
#endif
		declarations(base.declarations.end())
	{
	}
	~ScopeGuard()
	{
#ifdef ALLOCATOR_DEBUG
		base.scopes.erase(scopes, base.scopes.end());
#endif
		base.declarations.erase(declarations, base.declarations.end());
	}
	// if parse succeeds, append new state to previous state
	void hit()
	{
#ifdef ALLOCATOR_DEBUG
		scopes = base.scopes.end();
#endif
		declarations = base.declarations.end();
	}
};
#else
struct ScopeGuard
{
	WalkerBase& base;
#ifdef ALLOCATOR_DEBUG
	WalkerBase::Scopes scopes;
#endif
	WalkerBase::Declarations declarations;
	ScopeGuard(WalkerBase& base)
		: base(base),
#ifdef ALLOCATOR_DEBUG
		scopes(base.context),
#endif
		declarations(base.context)
	{
#ifdef ALLOCATOR_DEBUG
		scopes.swap(base.scopes);
#endif
		declarations.swap(base.declarations);
	}
	~ScopeGuard()
	{
#ifdef ALLOCATOR_DEBUG
		scopes.swap(base.scopes);
#endif
		declarations.swap(base.declarations);
	}
	// if parse succeeds, append new state to previous state
	void hit()
	{
#ifdef ALLOCATOR_DEBUG
		scopes.splice(scopes.end(), base.scopes);
#endif
		declarations.splice(declarations.end(), base.declarations);
	}
};
#endif

struct WalkerQualified : public WalkerBase
{
	Qualifying qualifying;
	WalkerQualified(const WalkerState& state)
		: WalkerBase(state), qualifying(context)
	{
	}

	void setQualifyingGlobal()
	{
		SEMANTIC_ASSERT(qualifying.empty());
		qualifying_p = &context.globalType;
	}

	void swapQualifying(Type& type)
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
		Qualifying tmp(TYPE_NULL, context);
		tmp.back().swap(type);
		swapQualifying(tmp);
	}
	void swapQualifying(Qualifying& other)
	{
		qualifying.swap(other);
		qualifying_p = qualifying.get();
	}
};



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
	if(func == isNestedName)
	{
		return "nested-name";
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
#define TREEWALKER_WALK(walker, symbol) SYMBOL_WALK(walker, symbol); hit(walker)
#define TREEWALKER_WALK_NOHIT(walker, symbol) SYMBOL_WALK(walker, symbol)
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


struct Walker
{


struct NamespaceNameWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Declaration* declaration;
	LookupFilter filter;
	NamespaceNameWalker(const WalkerState& state, LookupFilter filter = isAny)
		: WalkerBase(state), declaration(0), filter(filter)
	{
	}
	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF(symbol);
		declaration = findDeclaration(symbol->value, filter);
		if(declaration == &gUndeclared
			|| !isNamespaceName(*declaration))
		{
			return reportIdentifierMismatch(symbol, symbol->value, declaration, "namespace-name");
		}
		symbol->value.dec.p = declaration;
	}
};

struct UncheckedTemplateIdWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Identifier* id;
	TemplateArguments arguments;
	UncheckedTemplateIdWalker(const WalkerState& state)
		: WalkerBase(state), id(0), arguments(context)
	{
	}
	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF(symbol);
		id = &symbol->value;
	}
	void visit(cpp::template_argument_clause* symbol)
	{
		clearQualifying();
		// TODO: store args
		TemplateArgumentListWalker walker(getState());
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
	bool isTemplate;
	UnqualifiedIdWalker(const WalkerState& state, bool isTemplate = false)
		: WalkerBase(state), declaration(0), id(0), isIdentifier(false), isTemplate(isTemplate)
	{
	}
	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF(symbol);
		id = &symbol->value;
		isIdentifier = true;
		if(!isDependent(qualifying_p))
		{
			declaration = findDeclaration(*id);
		}
	}
	void visit(cpp::simple_template_id* symbol)
	{
		UncheckedTemplateIdWalker walker(getState());
		TREEWALKER_WALK_NOHIT(walker, symbol);
		id = walker.id;
		if(!isTemplate
			&& !isDependent(qualifying_p))
		{
			declaration = findDeclaration(*id);
			if(declaration == &gUndeclared
				|| !isTemplateName(*declaration))
			{
				return reportIdentifierMismatch(symbol, *id, declaration, "template-name");
			}
			else
			{
				declaration = findTemplateSpecialization(declaration, walker.arguments);
			}
		}
		hit(walker);
	}
	void visit(cpp::template_id_operator_function* symbol)
	{
		UncheckedTemplateIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::operator_function_id* symbol) 
	{
		TREEWALKER_LEAF(symbol);
		// TODO
		id = &gOperatorFunctionId;
	}
	void visit(cpp::conversion_function_id* symbol) 
	{
		TypeIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		// TODO
		id = &gConversionFunctionId;
	}
	void visit(cpp::destructor_id* symbol)
	{
		// TODO: can destructor-id be dependent?
		TREEWALKER_WALK(*this, symbol);
		id = &symbol->name->value;
	}
};

struct QualifiedIdWalker : public WalkerQualified
{
	TREEWALKER_DEFAULT;

	Declaration* declaration;
	Identifier* id;
	bool isTemplate;
	QualifiedIdWalker(const WalkerState& state)
		: WalkerQualified(state), declaration(0), id(0), isTemplate(false)
	{
	}

	void visit(cpp::terminal<boost::wave::T_COLON_COLON> symbol)
	{
		setQualifyingGlobal();
	}
	void visit(cpp::nested_name_specifier* symbol)
	{
		NestedNameSpecifierWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		swapQualifying(walker.qualifying);
	}
	void visit(cpp::terminal<boost::wave::T_TEMPLATE> symbol)
	{
		isTemplate = true;
	}
	void visit(cpp::unqualified_id* symbol)
	{
		UnqualifiedIdWalker walker(getState(), isTemplate);
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
		id = walker.id;
	}
	void visit(cpp::qualified_id_suffix* symbol)
	{
		UnqualifiedIdWalker walker(getState(), isTemplate);
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
		id = walker.id;
	}
};

struct IdExpressionWalker : public WalkerQualified
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
	bool isTemplate;
	IdExpressionWalker(const WalkerState& state)
		: WalkerQualified(state), declaration(0), id(0), isIdentifier(false), isTemplate(false)
	{
	}
	// HACK: for postfix-expression-member 
	void visit(cpp::terminal<boost::wave::T_TEMPLATE> symbol)
	{
		isTemplate = true;
	}
	void visit(cpp::qualified_id_default* symbol)
	{
		// TODO
		QualifiedIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
		id = walker.id;
		swapQualifying(walker.qualifying);
	}
	void visit(cpp::qualified_id_global* symbol)
	{
		// TODO
		QualifiedIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
		id = walker.id;
		swapQualifying(walker.qualifying);
	}
	void visit(cpp::unqualified_id* symbol)
	{
		// TODO
		UnqualifiedIdWalker walker(getState(), isTemplate);
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
		id = walker.id;
		isIdentifier = walker.isIdentifier;
	}
};

struct ExplicitTypeExpressionWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Dependent typeDependent;
	Dependent valueDependent;
	ExplicitTypeExpressionWalker(const WalkerState& state)
		: WalkerBase(state), typeDependent(context), valueDependent(context)
	{
	}
	void visit(cpp::simple_type_specifier* symbol)
	{
		TypeSpecifierWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		addDependent(typeDependent, walker.type);
	}
	void visit(cpp::type_id* symbol)
	{
		TypeIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		addDependent(typeDependent, walker.type);
		typeDependent.splice(walker.valueDependent);
	}
	void visit(cpp::new_type* symbol)
	{
		TypeIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		addDependent(typeDependent, walker.type);
		typeDependent.splice(walker.valueDependent);
	}
	void visit(cpp::assignment_expression* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		valueDependent.splice(walker.valueDependent);
	}
	void visit(cpp::cast_expression* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		valueDependent.splice(walker.valueDependent);
	}
};

struct DependentPrimaryExpressionWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Declaration* declaration;
	Identifier* id;
	Dependent typeDependent;
	DependentPrimaryExpressionWalker(const WalkerState& state)
		: WalkerBase(state), declaration(0), id(0), typeDependent(context)
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
		IdExpressionWalker walker(getState());
		TREEWALKER_WALK_NOHIT(walker, symbol);
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
					return reportIdentifierMismatch(symbol, *walker.id, declaration, "object-name");
				}
				else
				{
					addDependentType(typeDependent, declaration);
				}
				walker.id->dec.p = declaration;
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
		hit(walker);
	}
	void visit(cpp::primary_expression_parenthesis* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		typeDependent.splice(walker.typeDependent);
	}
};

struct DependentPostfixExpressionWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Declaration* declaration;
	Identifier* id;
	Dependent typeDependent;
	DependentPostfixExpressionWalker(const WalkerState& state)
		: WalkerBase(state), declaration(0), id(0), typeDependent(context)
	{
	}
	void visit(cpp::primary_expression* symbol)
	{
		DependentPrimaryExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
		id = walker.id;
		typeDependent.splice(walker.typeDependent);
	}
	void visit(cpp::postfix_expression_call* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK_NOHIT(walker, symbol);
		typeDependent.splice(walker.typeDependent);
		if(id != 0)
		{
			if(typeDependent.empty())
			{
				if(declaration != 0)
				{
					if(declaration == &gUndeclared
						|| !isObject(*declaration))
					{
						return reportIdentifierMismatch(symbol, *id, declaration, "object-name");
					}
					else
					{
						addDependentType(typeDependent, declaration);
					}
					id->dec.p = declaration;
				}
			}
			else
			{
				id->dec.p = &gDependentObject;
			}
		}
		hit(walker);
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
	ExpressionWalker(const WalkerState& state)
		: WalkerBase(state), typeDependent(context), valueDependent(context)
	{
	}
	void visit(cpp::postfix_expression_member* symbol)
	{
		IdExpressionWalker walker(getState());
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
		TypeIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::simple_type_specifier* symbol)
	{
		TypeSpecifierWalker walker(getState());
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
		IdExpressionWalker walker(getState());
		TREEWALKER_WALK_NOHIT(walker, symbol);
		Declaration* declaration = walker.declaration;
		if(declaration != 0)
		{
			if(declaration == &gUndeclared
				|| !isObject(*declaration))
			{
				return reportIdentifierMismatch(symbol, *walker.id, declaration, "object-name");
			}
			else
			{
				addDependentType(typeDependent, declaration);
				addDependentType(valueDependent, declaration);
				addDependentName(valueDependent, declaration);
			}
			walker.id->dec.p = declaration;
		}
		else
		{
			if(isDependent(walker.qualifying.get()))
			{
				walker.id->dec.p = &gDependentObject;
			}
			if(!walker.qualifying.empty())
			{
				addDependent(typeDependent, walker.qualifying.back());
			}
		}
		hit(walker);
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
		DependentPostfixExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		typeDependent.splice(walker.typeDependent);
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
		ExplicitTypeExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		typeDependent.splice(walker.typeDependent);
	}
	void visit(cpp::new_expression_placement* symbol)
	{
		ExplicitTypeExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		typeDependent.splice(walker.typeDependent);
	}
	void visit(cpp::new_expression_default* symbol)
	{
		ExplicitTypeExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		typeDependent.splice(walker.typeDependent);
	}
	void visit(cpp::postfix_expression_cast* symbol)
	{
		ExplicitTypeExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		if(symbol->op->id != cpp::cast_operator::DYNAMIC)
		{
			Dependent tmp(walker.typeDependent);
			valueDependent.splice(tmp);
		}
		typeDependent.splice(walker.typeDependent);
		valueDependent.splice(walker.valueDependent);
	}
	void visit(cpp::cast_expression_default* symbol)
	{
		ExplicitTypeExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		Dependent tmp(walker.typeDependent);
		valueDependent.splice(tmp);
		typeDependent.splice(walker.typeDependent);
		valueDependent.splice(walker.valueDependent);
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
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		valueDependent.splice(walker.typeDependent);
	}
	void visit(cpp::unary_expression_sizeoftype* symbol)
	{
		TypeIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		addDependent(valueDependent, walker.type);
		valueDependent.splice(walker.valueDependent);
	}
	void visit(cpp::postfix_expression_typeid* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::postfix_expression_typeidtype* symbol)
	{
		TypeIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::delete_expression* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::throw_expression* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
};

struct TemplateArgumentListWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	TemplateArguments arguments;

	TemplateArgumentListWalker(const WalkerState& state)
		: WalkerBase(state), arguments(context)
	{
	}
	void visit(cpp::type_id* symbol)
	{
		TypeIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
#if 0
		TemplateArgument& argument = append(arguments);
		argument.type.swap(walker.type);
		argument.dependent.splice(walker.valueDependent);
#else
		arguments.push_back(TemplateArgument(context));
		arguments.back().type.swap(walker.type);
		arguments.back().dependent.splice(walker.valueDependent);
#endif
	}
	void visit(cpp::assignment_expression* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
#if 0
		TemplateArgument& argument = append(arguments);
		argument.dependent.splice(walker.typeDependent);
		argument.dependent.splice(walker.valueDependent);
#else
		arguments.push_back(TemplateArgument(context)); // todo: evaluate constant-expression (unless it's dependent expression)
		arguments.back().dependent = walker.typeDependent;
		arguments.back().dependent.splice(walker.valueDependent);
#endif
	}
};

struct TemplateIdWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Type type;
	LookupFilter filter;
	bool isTypename;
	TemplateIdWalker(const WalkerState& state, LookupFilter filter = isAny, bool isTypename = false)
		: WalkerBase(state), type(0, context), filter(filter), isTypename(isTypename)
	{
	}
	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF(symbol);
		if(!isDependent(qualifying_p))
		{
			Declaration* declaration = findDeclaration(symbol->value, filter);
			if(declaration == &gUndeclared
				|| !isTemplateName(*declaration))
			{
				return reportIdentifierMismatch(symbol, symbol->value, declaration, "template-name");
			}
			type.declaration = declaration;
			symbol->value.dec.p = declaration;
		}
		else if(!isTypename)
		{
			return reportIdentifierMismatch(symbol, symbol->value, &gUndeclared, "typename");
		}
		else
		{
			Declaration* declaration = &gDependentTemplate;
			type.declaration = declaration;
			symbol->value.dec.p = declaration;
		}
	}
	void visit(cpp::template_argument_clause* symbol)
	{
		clearQualifying();

		TemplateArgumentListWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);

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
	TypeNameWalker(const WalkerState& state, LookupFilter filter = isAny, bool isTypename = false)
		: WalkerBase(state), type(0, context), filter(filter), isTypename(isTypename)
	{
	}
	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF(symbol);
		if(!isDependent(qualifying_p))
		{
			Declaration* declaration = findDeclaration(symbol->value, filter);
			if(declaration == &gUndeclared
				|| !isTypeName(*declaration))
			{
				return reportIdentifierMismatch(symbol, symbol->value, declaration, "type-name");
			}
			type.declaration = declaration;
			symbol->value.dec.p = declaration;
		}
		else if(!isTypename)
		{
			return reportIdentifierMismatch(symbol, symbol->value, &gUndeclared, "typename");
		}
		else
		{
			Declaration* declaration = &gDependentType;
			type.declaration = declaration;
			symbol->value.dec.p = declaration;
		}
	}
	void visit(cpp::simple_template_id* symbol)
	{
		TemplateIdWalker walker(getState(), filter, isTypename);
		TREEWALKER_WALK(walker, symbol);
		type.swap(walker.type);
	}
};

struct NestedNameSpecifierSuffixWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Type type;
	bool allowDependent;
	bool isTemplate;
	NestedNameSpecifierSuffixWalker(const WalkerState& state, bool allowDependent = false)
		: WalkerBase(state), type(0, context), allowDependent(allowDependent), isTemplate(false)
	{
	}
	void visit(cpp::terminal<boost::wave::T_TEMPLATE> symbol)
	{
		isTemplate = true;
	}
	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF(symbol);
		if(allowDependent
			|| !isDependent(qualifying_p))
		{
			Declaration* declaration = findDeclaration(symbol->value, isNestedName);
			if(declaration == &gUndeclared)
			{
				return reportIdentifierMismatch(symbol, symbol->value, declaration, "nested-name");
			}
			else
			{
				type = declaration;
			}
			symbol->value.dec.p = declaration;
		}
		else
		{
			symbol->value.dec.p = &gDependentNested;
		}
	}
	void visit(cpp::simple_template_id* symbol)
	{
		UncheckedTemplateIdWalker walker(getState());
		TREEWALKER_WALK_NOHIT(walker, symbol);
		if(!isTemplate
			&& (allowDependent
				|| !isDependent(qualifying_p)))
		{
			Declaration* declaration = findDeclaration(*walker.id, isNestedName);
			if(declaration == &gUndeclared)
			{
				return reportIdentifierMismatch(symbol, *walker.id, declaration, "nested-name");
			}
			else
			{
				declaration = findTemplateSpecialization(declaration, walker.arguments);
				type.declaration = declaration;
				type.arguments.swap(walker.arguments);
			}
		}
		hit(walker);
	}
};

struct NestedNameSpecifierPrefixWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Type type;
	bool allowDependent;
	NestedNameSpecifierPrefixWalker(const WalkerState& state, bool allowDependent = false)
		: WalkerBase(state), type(0, context), allowDependent(allowDependent)
	{
	}
	void visit(cpp::namespace_name* symbol)
	{
		NamespaceNameWalker walker(getState(), isNestedName);
		TREEWALKER_WALK(walker, symbol);
		type = walker.declaration;
	}
	void visit(cpp::type_name* symbol)
	{
		TypeNameWalker walker(getState(), isNestedName, true);
		TREEWALKER_WALK(walker, symbol);
		if(allowDependent
			|| !isDependent(qualifying_p))
		{
			type.swap(walker.type);
		}
	}
};

struct NestedNameSpecifierWalker : public WalkerQualified
{
	TREEWALKER_DEFAULT;

	bool allowDependent;
	NestedNameSpecifierWalker(const WalkerState& state, bool allowDependent = false)
		: WalkerQualified(state), allowDependent(allowDependent)
	{
	}
	void visit(cpp::nested_name_specifier_prefix* symbol)
	{
		NestedNameSpecifierPrefixWalker walker(getState(), allowDependent);
		TREEWALKER_WALK(walker, symbol);
		if(walker.type.declaration != 0)
		{
			swapQualifying(walker.type);
		}
	}
	void visit(cpp::nested_name_specifier_suffix_template* symbol)
	{
		NestedNameSpecifierSuffixWalker walker(getState(), allowDependent);
		TREEWALKER_WALK(walker, symbol);
		if(walker.type.declaration != 0)
		{
			walker.type.qualifying.swap(qualifying);
			swapQualifying(walker.type);
		}
	}
	void visit(cpp::nested_name_specifier_suffix_default* symbol)
	{
		NestedNameSpecifierSuffixWalker walker(getState(), allowDependent);
		TREEWALKER_WALK(walker, symbol);
		if(walker.type.declaration != 0)
		{
			walker.type.qualifying.swap(qualifying);
			swapQualifying(walker.type);
		}
	}
};

struct TypeSpecifierWalker : public WalkerQualified
{
	TREEWALKER_DEFAULT;

	Type type;
	unsigned fundamental;
	TypeSpecifierWalker(const WalkerState& state)
		: WalkerQualified(state), type(0, context), fundamental(0)
	{
	}

	void visit(cpp::simple_type_specifier_name* symbol)
	{
		TypeSpecifierWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		type.swap(walker.type);
		fundamental = walker.fundamental;
	}
	void visit(cpp::simple_type_specifier_template* symbol)
	{
		TypeSpecifierWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		type.swap(walker.type);
		fundamental = walker.fundamental;
	}
	void visit(cpp::type_name* symbol)
	{
		TypeNameWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		SEMANTIC_ASSERT(walker.type.declaration != 0);
		type.swap(walker.type);
		type.qualifying.swap(qualifying);
	}
	void visit(cpp::terminal<boost::wave::T_COLON_COLON> symbol)
	{
		setQualifyingGlobal();
	}
	void visit(cpp::nested_name_specifier* symbol)
	{
		NestedNameSpecifierWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		swapQualifying(walker.qualifying);
	}
	void visit(cpp::simple_template_id* symbol) 
	{
		TemplateIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		SEMANTIC_ASSERT(walker.type.declaration != 0);
		type.swap(walker.type);
		type.qualifying.swap(qualifying);
	}
	void visit(cpp::simple_type_specifier_builtin* symbol)
	{
		TREEWALKER_LEAF(symbol);
		fundamental = symbol->id;
	}
};

struct UnqualifiedDeclaratorIdWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Identifier* id;
	UnqualifiedDeclaratorIdWalker(const WalkerState& state)
		: WalkerBase(state), id(&gAnonymousId)
	{
	}
	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF(symbol);
		id = &symbol->value;
	}
	void visit(cpp::simple_template_id* symbol) 
	{
		UncheckedTemplateIdWalker walker(getState());
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
		TypeIdWalker walker(getState());
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

struct QualifiedDeclaratorIdWalker : public WalkerQualified
{
	TREEWALKER_DEFAULT;

	Identifier* id;
	QualifiedDeclaratorIdWalker(const WalkerState& state)
		: WalkerQualified(state), id(&gAnonymousId)
	{
	}
	void visit(cpp::terminal<boost::wave::T_COLON_COLON> symbol)
	{
		setQualifyingGlobal();
	}
	void visit(cpp::nested_name_specifier* symbol)
	{
		NestedNameSpecifierWalker walker(getState(), true);
		TREEWALKER_WALK(walker, symbol);
		swapQualifying(walker.qualifying);
	}
	void visit(cpp::unqualified_id* symbol)
	{
		UnqualifiedDeclaratorIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		id = walker.id;
	}
};

struct DeclaratorIdWalker : public WalkerQualified
{
	TREEWALKER_DEFAULT;

	Identifier* id;
	DeclaratorIdWalker(const WalkerState& state)
		: WalkerQualified(state), id(&gAnonymousId)
	{
	}
	void visit(cpp::qualified_id_default* symbol)
	{
		QualifiedDeclaratorIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		id = walker.id;
		swapQualifying(walker.qualifying);
	}
	void visit(cpp::qualified_id_global* symbol)
	{
		QualifiedDeclaratorIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		id = walker.id;
		swapQualifying(walker.qualifying);
	}
	void visit(cpp::unqualified_id* symbol)
	{
		UnqualifiedDeclaratorIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		id = walker.id;
	}
};

struct ParameterDeclarationClauseWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	ParameterDeclarationClauseWalker(const WalkerState& state)
		: WalkerBase(state)
	{
		pushScope(newScope(makeIdentifier("$declarator"), SCOPETYPE_PROTOTYPE));
		if(templateParams != 0)
		{
			enclosing->declarations = templateParams->declarations;
			for(Scope::Declarations::iterator i = enclosing->declarations.begin(); i != enclosing->declarations.end(); ++i)
			{
				(*i).second.scope = enclosing;
			}
		}
		templateParams = 0;
	}

	void visit(cpp::parameter_declaration* symbol)
	{
		SimpleDeclarationWalker walker(getState(), true);
		TREEWALKER_WALK(walker, symbol);
	}
};

struct ExceptionSpecificationWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	ExceptionSpecificationWalker(const WalkerState& state)
		: WalkerBase(state)
	{
	}
	void visit(cpp::type_id* symbol)
	{
		TypeIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
};

struct PtrOperatorWalker : public WalkerQualified
{
	TREEWALKER_DEFAULT;

	PtrOperatorWalker(const WalkerState& state)
		: WalkerQualified(state)
	{
	}
	void visit(cpp::nested_name_specifier* symbol)
	{
		NestedNameSpecifierWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		swapQualifying(walker.qualifying);
	}
};

struct DeclaratorSuffixWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Scope* paramScope;
	Dependent valueDependent;
	DeclaratorSuffixWalker(const WalkerState& state)
		: WalkerBase(state), paramScope(0), valueDependent(context)
	{
	}

	void visit(cpp::parameter_declaration_clause* symbol)
	{
		ParameterDeclarationClauseWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		paramScope = walker.enclosing; // store reference for later resumption
	}
	void visit(cpp::exception_specification* symbol)
	{
		ExceptionSpecificationWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::constant_expression* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		valueDependent.splice(walker.valueDependent);
	}
};

struct DeclaratorWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Identifier* id;
	Scope* paramScope;
	Dependent valueDependent;
	DeclaratorWalker(const WalkerState& state)
		: WalkerBase(state), id(&gAnonymousId), paramScope(0), valueDependent(context)
	{
	}

	void visit(cpp::ptr_operator* symbol)
	{
		PtrOperatorWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::declarator_id* symbol)
	{
		DeclaratorIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		id = walker.id;

		if(walker.getQualifyingScope() != 0
			&& enclosing->type != SCOPETYPE_CLASS) // in 'class C { friend Q::N(X); };' X should be looked up in the scope of C rather than Q
		{
			enclosing = walker.getQualifyingScope(); // names in declarator suffix (array-size, parameter-declaration) are looked up in declarator-id's qualifying scope
		}
	}
	void visit(cpp::declarator_suffix_array* symbol)
	{
		DeclaratorSuffixWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		valueDependent.splice(walker.valueDependent);
	}
	void visit(cpp::declarator_suffix_function* symbol)
	{
		DeclaratorSuffixWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		paramScope = walker.paramScope;
		valueDependent.splice(walker.valueDependent);
	}
};

struct BaseSpecifierWalker : public WalkerQualified
{
	TREEWALKER_DEFAULT;

	Type type;
	BaseSpecifierWalker(const WalkerState& state)
		: WalkerQualified(state), type(0, context)
	{
	}

	void visit(cpp::nested_name_specifier* symbol)
	{
		NestedNameSpecifierWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		swapQualifying(walker.qualifying);
	}
	void visit(cpp::class_name* symbol)
	{
		/* 10-2
		The class-name in a base-specifier shall not be an incompletely defined class (Clause class); this class is
		called a direct base class for the class being defined. During the lookup for a base class name, non-type
		names are ignored (3.3.10)
		*/
		TypeNameWalker walker(getState(), isTypeName, true);
		TREEWALKER_WALK(walker, symbol);
		type.swap(walker.type);
		type.qualifying.swap(qualifying);
	}
};

struct ClassHeadWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Declaration* declaration;
	Identifier* id;
	TemplateArguments arguments;
	bool isUnion;
	ClassHeadWalker(const WalkerState& state)
		: WalkerBase(state), declaration(0), id(0), arguments(context), isUnion(false)
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
		NestedNameSpecifierWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);

		if(walker.getQualifyingScope() != 0)
		{
			if(enclosing == templateEnclosing)
			{
				templateEnclosing = walker.getQualifyingScope();
			}
			enclosing = walker.getQualifyingScope(); // names in declaration of nested-class are looked up in scope of enclosing class
		}
	}
	void visit(cpp::simple_template_id* symbol) 
	{
		UncheckedTemplateIdWalker walker(getState());
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
		BaseSpecifierWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		if(walker.type.declaration != 0) // declaration == 0 if base-class is dependent
		{
			SEMANTIC_ASSERT(declaration->enclosed != 0);
			addBase(declaration, walker.type);
		}
	}
};

struct UsingDeclarationWalker : public WalkerQualified
{
	TREEWALKER_DEFAULT;

	bool isTypename;
	UsingDeclarationWalker(const WalkerState& state)
		: WalkerQualified(state), isTypename(false)
	{
	}
	void visit(cpp::terminal<boost::wave::T_COLON_COLON> symbol)
	{
		setQualifyingGlobal();
	}
	void visit(cpp::nested_name_specifier* symbol)
	{
		NestedNameSpecifierWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		swapQualifying(walker.qualifying);
	}
	void visit(cpp::unqualified_id* symbol)
	{
		UnqualifiedIdWalker walker(getState());
		TREEWALKER_WALK_NOHIT(walker, symbol);
		if(!isTypename
			&& !isDependent(qualifying_p))
		{
			Declaration* declaration = walker.declaration;
			if(declaration == &gUndeclared
				|| !(isObject(*declaration) || isTypeName(*declaration)))
			{
				return reportIdentifierMismatch(symbol, *walker.id, declaration, "object-name or type-name");
			}
			else
			{
				// TODO: check for conflicts with earlier declarations
				enclosing->declarations.insert(Scope::Declarations::value_type(declaration->name.value, *declaration));
			}
			walker.id->dec.p = declaration;
		}
		else
		{
			// TODO: introduce typename into enclosing namespace
			walker.id->dec.p = &gDependentType;
		}
		hit(walker);
	}
	void visit(cpp::terminal<boost::wave::T_TYPENAME> symbol)
	{
		isTypename = true;
	}
};

struct UsingDirectiveWalker : public WalkerQualified
{
	TREEWALKER_DEFAULT;

	UsingDirectiveWalker(const WalkerState& state)
		: WalkerQualified(state)
	{
	}
	void visit(cpp::terminal<boost::wave::T_COLON_COLON> symbol)
	{
		setQualifyingGlobal();
	}
	void visit(cpp::nested_name_specifier* symbol)
	{
		NestedNameSpecifierWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		swapQualifying(walker.qualifying);
	}
	void visit(cpp::namespace_name* symbol)
	{
		/* basic.lookup.udir
		When looking up a namespace-name in a using-directive or namespace-alias-definition, only namespace
		names are considered.
		*/
		NamespaceNameWalker walker(getState(), isNamespaceName);
		TREEWALKER_WALK(walker, symbol);
		if(!findScope(enclosing, walker.declaration->enclosed))
		{
			enclosing->usingDirectives.push_back(walker.declaration->enclosed);
		}
	}
};

struct NamespaceAliasDefinitionWalker : public WalkerQualified
{
	TREEWALKER_DEFAULT;

	Identifier* id;
	NamespaceAliasDefinitionWalker(const WalkerState& state)
		: WalkerQualified(state), id(0)
	{
	}
	void visit(cpp::terminal<boost::wave::T_COLON_COLON> symbol)
	{
		setQualifyingGlobal();
	}
	void visit(cpp::nested_name_specifier* symbol)
	{
		NestedNameSpecifierWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		swapQualifying(walker.qualifying);
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
				return reportIdentifierMismatch(symbol, symbol->value, declaration, "namespace-name");
			}
			else
			{
				// TODO: check for conflicts with earlier declarations
				declaration = pointOfDeclaration(context, enclosing, *id, TYPE_NAMESPACE, declaration->enclosed);
			}
			id->dec.p = declaration;
		}
	}
};

struct MemberDeclarationWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Declaration* declaration;
	MemberDeclarationWalker(const WalkerState& state)
		: WalkerBase(state), declaration(0)
	{
	}
	void visit(cpp::member_template_declaration* symbol)
	{
		TemplateDeclarationWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
	}
	void visit(cpp::member_declaration_implicit* symbol)
	{
		SimpleDeclarationWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
	}
	void visit(cpp::member_declaration_default* symbol)
	{
		SimpleDeclarationWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		SEMANTIC_ASSERT(walker.type.declaration != 0);
		declaration = walker.declaration;
	}
	void visit(cpp::member_declaration_nested* symbol)
	{
		QualifiedIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::using_declaration* symbol)
	{
		UsingDeclarationWalker walker(getState());
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
	ClassSpecifierWalker(const WalkerState& state)
		: WalkerBase(state), declaration(0), id(0), arguments(context), isUnion(false)
	{
	}

	void visit(cpp::class_head* symbol)
	{
		ClassHeadWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
		id = walker.id;
		isUnion = walker.isUnion;
		arguments.swap(walker.arguments);
		enclosing = walker.enclosing;
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
		MemberDeclarationWalker walker(getState());
		if(WalkerState::deferred == 0)
		{
			walker.deferred = &deferred;
		}
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::terminal<boost::wave::T_RIGHTBRACE> symbol)
	{
		parseDeferred(deferred, *this);
	}
};

struct EnumeratorDefinitionWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Declaration* declaration;
	EnumeratorDefinitionWalker(const WalkerState& state)
		: WalkerBase(state), declaration(0)
	{
	}

	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF(symbol);
		/* 3.1-4
		The point of declaration for an enumerator is immediately after its enumerator-definition.
		*/
		// TODO: give enumerators a type
		declaration = pointOfDeclaration(context, enclosing, symbol->value, TYPE_ENUMERATOR, 0, DeclSpecifiers());
		symbol->value.dec.p = declaration;
	}
	void visit(cpp::constant_expression* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		declaration->valueDependent.splice(walker.valueDependent);
	}
};

struct EnumSpecifierWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Declaration* declaration;
	Identifier* id;
	EnumSpecifierWalker(const WalkerState& state)
		: WalkerBase(state), declaration(0), id(0)
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
			declaration = pointOfDeclaration(context, enclosing, *id, TYPE_ENUM, 0);
			id->dec.p = declaration;
		}
	}

	void visit(cpp::enumerator_definition* symbol)
	{
		if(declaration == 0)
		{
			// unnamed enum
			declaration = pointOfDeclaration(context, enclosing, makeIdentifier(enclosing->getUniqueName()), TYPE_ENUM, 0);
		}
		EnumeratorDefinitionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
};

struct ElaboratedTypeSpecifierWalker : public WalkerQualified
{
	TREEWALKER_DEFAULT;

	Declaration* key;
	Type type;
	Identifier* id;
	ElaboratedTypeSpecifierWalker(const WalkerState& state)
		: WalkerQualified(state), key(0), type(0, context), id(0)
	{
	}
	void visit(cpp::terminal<boost::wave::T_COLON_COLON> symbol)
	{
		setQualifyingGlobal();
	}
	void visit(cpp::elaborated_type_specifier_default* symbol)
	{
		ElaboratedTypeSpecifierWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		type.swap(walker.type);
		id = walker.id;

		if(!isUnqualified(symbol)
			|| !isClassKey(*type.declaration))
		{
			id = 0;
		}
	}
	void visit(cpp::elaborated_type_specifier_template* symbol)
	{
		ElaboratedTypeSpecifierWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		type.swap(walker.type);
		id = walker.id;
	}
	void visit(cpp::nested_name_specifier* symbol)
	{
		NestedNameSpecifierWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		swapQualifying(walker.qualifying);
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
		TemplateIdWalker walker(getState(), isType);
		TREEWALKER_WALK(walker, symbol);
		type.swap(walker.type);
		type.qualifying.swap(qualifying);
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

struct TypenameSpecifierWalker : public WalkerQualified
{
	TREEWALKER_DEFAULT;

	Type type;
	TypenameSpecifierWalker(const WalkerState& state)
		: WalkerQualified(state), type(0, context)
	{
	}

	void visit(cpp::terminal<boost::wave::T_COLON_COLON> symbol)
	{
		setQualifyingGlobal();
	}
	void visit(cpp::terminal<boost::wave::T_TEMPLATE> symbol)
	{
		// TODO
	}
	void visit(cpp::nested_name_specifier* symbol)
	{
		NestedNameSpecifierWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		swapQualifying(walker.qualifying);
	}
	void visit(cpp::type_name* symbol)
	{
		TypeNameWalker walker(getState(), isAny, true);
		TREEWALKER_WALK(walker, symbol);
		type.swap(walker.type);
		type.qualifying.swap(qualifying);
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
	DeclSpecifierSeqWalker(const WalkerState& state, bool isTemplateParameter = false)
		: WalkerBase(state), type(0, context), fundamental(0), forward(0), isUnion(false), isTemplateParameter(isTemplateParameter)
	{
	}

	void visit(cpp::simple_type_specifier* symbol)
	{
		TypeSpecifierWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		type.swap(walker.type);
		if(type.declaration == 0)
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
	void visit(cpp::elaborated_type_specifier_template* symbol)
	{
		ElaboratedTypeSpecifierWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		forward = walker.id;
		type.swap(walker.type);
	}
	void visit(cpp::elaborated_type_specifier* symbol)
	{
		ElaboratedTypeSpecifierWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		forward = walker.id;
		type.swap(walker.type);
	}
	void visit(cpp::typename_specifier* symbol)
	{
		TypenameSpecifierWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		type.swap(walker.type);
	}
	void visit(cpp::class_specifier* symbol)
	{
		ClassSpecifierWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		type = walker.declaration;
		isUnion = walker.isUnion;
	}
	void visit(cpp::enum_specifier* symbol)
	{
		EnumSpecifierWalker walker(getState());
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

struct TryBlockWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	TryBlockWalker(const WalkerState& state)
		: WalkerBase(state)
	{
	}

	void visit(cpp::compound_statement* symbol)
	{
		CompoundStatementWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::handler_seq* symbol)
	{
		HandlerSeqWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
};

struct StatementWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	StatementWalker(const WalkerState& state)
		: WalkerBase(state)
	{
	}
	void visit(cpp::simple_declaration* symbol)
	{
		SimpleDeclarationWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		SEMANTIC_ASSERT(walker.type.declaration != 0);
	}
	void visit(cpp::try_block* symbol)
	{
		TryBlockWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::namespace_alias_definition* symbol)
	{
		NamespaceAliasDefinitionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::selection_statement* symbol)
	{
		ControlStatementWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::iteration_statement* symbol)
	{
		ControlStatementWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::compound_statement* symbol)
	{
		CompoundStatementWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::expression_statement* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::jump_statement_return* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::jump_statement_goto* symbol)
	{
		TREEWALKER_LEAF(symbol);
		// TODO
	}
	void visit(cpp::using_directive* symbol)
	{
		UsingDirectiveWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
};

struct ControlStatementWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	ControlStatementWalker(const WalkerState& state)
		: WalkerBase(state)
	{
	}
	void visit(cpp::terminal<boost::wave::T_LEFTPAREN> symbol)
	{
		pushScope(newScope(makeIdentifier(enclosing->getUniqueName()), SCOPETYPE_LOCAL));
	}
	void visit(cpp::condition_init* symbol)
	{
		SimpleDeclarationWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		SEMANTIC_ASSERT(walker.type.declaration != 0);
	}
	void visit(cpp::simple_declaration* symbol)
	{
		SimpleDeclarationWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::statement* symbol)
	{
		StatementWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::expression* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
};

struct CompoundStatementWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	CompoundStatementWalker(const WalkerState& state)
		: WalkerBase(state)
	{
	}

	void visit(cpp::terminal<boost::wave::T_LEFTBRACE> symbol)
	{
		pushScope(newScope(makeIdentifier(enclosing->getUniqueName()), SCOPETYPE_LOCAL));
	}
	void visit(cpp::statement* symbol)
	{
		StatementWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
};

struct HandlerWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	HandlerWalker(const WalkerState& state)
		: WalkerBase(state)
	{
	}
	void visit(cpp::terminal<boost::wave::T_CATCH> symbol)
	{
		pushScope(newScope(makeIdentifier(enclosing->getUniqueName()), SCOPETYPE_LOCAL));
	}
	void visit(cpp::exception_declaration_default* symbol)
	{
		SimpleDeclarationWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		walker.commit();
	}
	void visit(cpp::compound_statement* symbol)
	{
		CompoundStatementWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
};

struct HandlerSeqWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	HandlerSeqWalker(const WalkerState& state)
		: WalkerBase(state)
	{
	}
	void visit(cpp::handler* symbol)
	{
		HandlerWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
};

struct QualifiedTypeNameWalker : public WalkerQualified
{
	TREEWALKER_DEFAULT;

	QualifiedTypeNameWalker(const WalkerState& state)
		: WalkerQualified(state)
	{
	}
	void visit(cpp::terminal<boost::wave::T_COLON_COLON> symbol)
	{
		setQualifyingGlobal();
	}
	void visit(cpp::nested_name_specifier* symbol)
	{
		NestedNameSpecifierWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		swapQualifying(walker.qualifying);
	}
	void visit(cpp::class_name* symbol)
	{
		TypeNameWalker walker(getState(), isAny, true);
		TREEWALKER_WALK(walker, symbol);
	}
};

struct MemInitializerWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	MemInitializerWalker(const WalkerState& state)
		: WalkerBase(state)
	{
	}
	void visit(cpp::mem_initializer_id_base* symbol)
	{
		QualifiedTypeNameWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF(symbol);
		Declaration* declaration = findDeclaration(symbol->value);
		if(declaration == &gUndeclared
			|| !isObject(*declaration))
		{
			return reportIdentifierMismatch(symbol, symbol->value, declaration, "object-name");
		}
		symbol->value.dec.p = declaration;
	}
	void visit(cpp::expression_list* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
};

struct MemberDeclaratorBitfieldWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Identifier* id;
	MemberDeclaratorBitfieldWalker(const WalkerState& state)
		: WalkerBase(state), id(0)
	{
	}
	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF(symbol);
		id = &symbol->value;
	}
	void visit(cpp::constant_expression* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
};

struct TypeIdWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Type type;
	Dependent valueDependent;
	TypeIdWalker(const WalkerState& state)
		: WalkerBase(state), type(0, context), valueDependent(context)
	{
	}
	void visit(cpp::type_specifier_seq* symbol)
	{
		DeclSpecifierSeqWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		type.swap(walker.type);
		declareEts(type, walker.forward);
	}
	void visit(cpp::abstract_declarator* symbol)
	{
		DeclaratorWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		valueDependent.splice(walker.valueDependent);
	}
};

struct IsTemplateName
{
	WalkerState& context;
	IsTemplateName(WalkerState& context) : context(context)
	{
	}
	bool operator()(Identifier& id) const
	{
		Declaration* declaration = context.findDeclaration(id);
		return declaration != &gUndeclared && isTemplateName(*declaration);
	}
};

struct InitializerWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	InitializerWalker(const WalkerState& state) : WalkerBase(state)
	{
	}
	void visit(cpp::assignment_expression* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
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
	DeferredSymbols deferred2;
#endif
	Dependent valueDependent;
	size_t templateParameter;
	bool isParameter;
	bool isUnion;

	SimpleDeclarationWalker(const WalkerState& state, bool isParameter = false, size_t templateParameter = INDEX_INVALID) : WalkerBase(state),
		declaration(0),
		parent(0),
		id(0),
		type(&gCtor, context),
		enclosed(0),
		forward(0),
		valueDependent(context),
		templateParameter(templateParameter),
		isParameter(isParameter),
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
			declaration = declareObject(parent, id, type, enclosed, specifiers, templateParameter, valueDependent);

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
		if(WalkerState::deferred != 0
			&& !deferred.empty())
		{
			WalkerState::deferred->splice(WalkerState::deferred->end(), deferred);
		}
#endif
	}

	// when parsing parameter-declaration, ensure that state of walker does not persist after failing parameter-declaration-default
	void visit(cpp::parameter_declaration_default* symbol)
	{
		SimpleDeclarationWalker walker(getState(), isParameter, templateParameter);
		TREEWALKER_WALK(walker, symbol);
		walker.commit();
	}
	// when parsing parameter-declaration, ensure that state of walker does not persist after failing parameter-declaration-abstract
	void visit(cpp::parameter_declaration_abstract* symbol)
	{
		SimpleDeclarationWalker walker(getState(), isParameter, templateParameter);
		TREEWALKER_WALK(walker, symbol);
		walker.commit();
	}
	void visit(cpp::decl_specifier_seq* symbol)
	{
		DeclSpecifierSeqWalker walker(getState(), templateParameter != INDEX_INVALID);
		TREEWALKER_WALK(walker, symbol);
		type.swap(walker.type);
		declaration = type.declaration; // if no declarator is specified later, this is probably a class-declaration
		specifiers = walker.specifiers;
		forward = walker.forward;
		isUnion = walker.isUnion;
	}
	void visit(cpp::type_specifier_seq* symbol)
	{
		DeclSpecifierSeqWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		type.swap(walker.type);
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
		DeclaratorWalker walker(getState());
		if(isParameter)
		{
			walker.declareEts(type, forward);
		}
#ifdef MINGLE
		if(WalkerState::deferred != 0)
		{
			walker.deferred = &deferred2;
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
		valueDependent.splice(walker.valueDependent);
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
		DeclaratorWalker walker(getState());
		if(isParameter)
		{
			walker.declareEts(type, forward);
		}
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::member_declarator_bitfield* symbol)
	{
		MemberDeclaratorBitfieldWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		if(walker.id != 0)
		{
			declaration = pointOfDeclaration(context, enclosing, *walker.id, type, 0, specifiers); // 3.3.1.1
			walker.id->dec.p = declaration;
		}
	}
	void visit(cpp::terminal<boost::wave::T_ASSIGN> symbol)
	{
		commit();
	}
	void visit(cpp::terminal<boost::wave::T_TRY> symbol)
	{
		commit();
	}
	void visit(cpp::terminal<boost::wave::T_LEFTBRACE> symbol)
	{
		commit();
	}
	void visit(cpp::terminal<boost::wave::T_COMMA> symbol)
	{
		commit();
	}
	void visit(cpp::terminal<boost::wave::T_SEMICOLON> symbol)
	{
		commit();
	}
	void visit(cpp::terminal<boost::wave::T_COLON> symbol)
	{
		commit();
	}

	// handle assignment-expression(s) in initializer
	void visit(cpp::default_parameter* symbol)
	{
#ifdef MINGLE
		// todo: we cannot skip a default-argument if it contains a template-name that is declared later in the class.
		// Comeau fails in this case too..
		if(WalkerState::deferred != 0
			&& templateParameter == INDEX_INVALID) // don't defer parse of default for non-type template-argument
		{
			result = defer(*WalkerState::deferred, *this, makeSkipDefaultArgument(IsTemplateName(*this)), symbol);
		}
		else
#endif
		{
			TREEWALKER_WALK(*this, symbol);
		}
	}
	// handle assignment-expression(s) in initializer
	void visit(cpp::assignment_expression* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	// handle initializer in separate context to avoid ',' confusing recognition of declaration
	void visit(cpp::initializer_clause* symbol)
	{
		InitializerWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	// handle initializer in separate context to avoid ',' confusing recognition of declaration
	void visit(cpp::initializer_parenthesis* symbol)
	{
		InitializerWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::constant_expression* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		SEMANTIC_ASSERT(declaration != 0);
		declaration->valueDependent.splice(walker.valueDependent);
	}

	void visit(cpp::statement_seq_wrapper* symbol)
	{
		ScopeGuard guard(*this);
		pushScope(newScope(makeIdentifier(enclosing->getUniqueName()), SCOPETYPE_LOCAL));
#ifdef MINGLE
		if(WalkerState::deferred != 0)
		{
			result = defer(*WalkerState::deferred, *this, skipBraced, symbol);
			if(result == 0)
			{
				return;
			}
		}
		else
#endif
		{
			TREEWALKER_WALK(*this, symbol);
		}
		guard.hit();
	}
	void visit(cpp::statement* symbol)
	{
		StatementWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::mem_initializer* symbol)
	{
		MemInitializerWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::handler_seq* symbol)
	{
		HandlerSeqWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
#ifdef MINGLE
	void visit(cpp::mem_initializer_clause* symbol)
	{
		result = defer(*WalkerState::deferred, *this, skipMemInitializerClause, symbol);
	}
#endif

	void visit(cpp::simple_declaration_named* symbol)
	{
		ScopeGuard guard(*this);
		declareEts(type, forward);
		TREEWALKER_WALK(*this, symbol);
		guard.hit();
	}
	void visit(cpp::member_declaration_named* symbol)
	{
		ScopeGuard guard(*this);
		declareEts(type, forward);
		TREEWALKER_WALK(*this, symbol);
		guard.hit();
#ifdef MINGLE
		// symbols may be deferred during attempt to parse void f(int i = j) {}
		// first parsed as member_declaration_named, fails on reaching '{'
		if(WalkerState::deferred != 0
			&& !deferred2.empty())
		{
			deferred.splice(deferred.end(), deferred2);
		}
#endif
	}
	void visit(cpp::function_definition* symbol)
	{
		ScopeGuard guard(*this);
		declareEts(type, forward);
		TREEWALKER_WALK(*this, symbol);
		guard.hit();
#ifdef MINGLE
		if(WalkerState::deferred != 0
			&& !deferred2.empty())
		{
			deferred.splice(deferred.end(), deferred2);
		}
#endif
	}
	void visit(cpp::type_declaration_suffix* symbol)
	{
		TREEWALKER_LEAF(symbol);
		if(forward != 0)
		{
			declaration = pointOfDeclaration(context, enclosing, *forward, TYPE_CLASS, 0, DeclSpecifiers(), enclosing == templateEnclosing);
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
					Declaration* declaration = ::findDeclaration(enclosing->declarations, member.name);
					if(declaration != 0)
					{
						printPosition(member.name.position);
						std::cout << "'" << member.name.value << "': anonymous union member already declared" << std::endl;
						printPosition(declaration->name.position);
						throw SemanticError();
					}
				}
				member.scope = enclosing;
				enclosing->declarations.insert(*i);
			}

		}
	}
};

struct TypeParameterWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Type param; // the default argument for this param
	Types params; // the default arguments for this param's template-params (if template-template-param)
	size_t templateParameter;
	TypeParameterWalker(const WalkerState& state, size_t templateParameter)
		: WalkerBase(state), param(0, context), templateParameter(templateParameter), params(context)
	{
	}
	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF(symbol);
		Declaration* declaration = pointOfDeclaration(context, enclosing, symbol->value, TYPE_PARAM, 0, DECLSPEC_TYPEDEF, !params.empty(), TEMPLATEARGUMENTS_NULL, templateParameter);
		symbol->value.dec.p = declaration;
		declaration->templateParamDefaults.swap(params);
	}
	void visit(cpp::type_id* symbol)
	{
		SEMANTIC_ASSERT(params.empty());
		TypeIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		param.swap(walker.type);
	}
	void visit(cpp::template_parameter_list* symbol)
	{
		TemplateParameterListWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		params.swap(walker.params);
	}
	void visit(cpp::id_expression* symbol)
	{
		IdExpressionWalker walker(getState());
		TREEWALKER_WALK_NOHIT(walker, symbol);
		Declaration* declaration = walker.declaration;
		if(declaration != 0)
		{
			if(declaration == &gUndeclared
				|| !isTemplateName(*declaration))
			{
				return reportIdentifierMismatch(symbol, *walker.id, declaration, "template-name");
			}
			walker.id->dec.p = declaration;
		}
		hit(walker);
	}
};

struct TemplateParameterListWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Types params;
	TemplateParameterListWalker(const WalkerState& state)
		: WalkerBase(state), params(context)
	{
		// collect template-params into a new scope
		Scope* scope = templateParams != 0 ? templateParams : newScope(makeIdentifier("$template"), SCOPETYPE_TEMPLATE);
		templateParams = 0;
		pushScope(scope);
	}
	void visit(cpp::type_parameter_default* symbol)
	{
		TypeParameterWalker walker(getState(), params.size());
		TREEWALKER_WALK(walker, symbol);
		params.push_back(Type(0, context));
		params.back().swap(walker.param);
	}
	void visit(cpp::type_parameter_template* symbol)
	{
		TypeParameterWalker walker(getState(), params.size());
		TREEWALKER_WALK(walker, symbol);
		params.push_back(Type(0, context));
		params.back().swap(walker.param);
	}
	void visit(cpp::parameter_declaration* symbol)
	{
		SimpleDeclarationWalker walker(getState(), false, params.size());
		TREEWALKER_WALK(walker, symbol);
		// push a dummy param so that we have the same number of template-params as template-arguments
		params.push_back(Type(0, context)); // TODO: default value for non-type template-param
	}
};

struct TemplateDeclarationWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Declaration* declaration;
	Types params;
	TemplateDeclarationWalker(const WalkerState& state)
		: WalkerBase(state), declaration(0), params(context)
	{
		templateEnclosing = enclosing;
	}
	void visit(cpp::template_parameter_list* symbol)
	{
		TemplateParameterListWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		templateParams = walker.enclosing;
		enclosing = walker.enclosing->parent;
		params.swap(walker.params);
	}
	void visit(cpp::declaration* symbol)
	{
		DeclarationWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
		declaration->templateParamDefaults.swap(params);
		SEMANTIC_ASSERT(declaration != 0);
	}
	void visit(cpp::member_declaration* symbol)
	{
		MemberDeclarationWalker walker(getState());
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
	DeclarationWalker(const WalkerState& state)
		: WalkerBase(state), declaration(0)
	{
	}
	void visit(cpp::namespace_definition* symbol)
	{
		NamespaceWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
	}
	void visit(cpp::namespace_alias_definition* symbol)
	{
		NamespaceAliasDefinitionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::general_declaration* symbol)
	{
		SimpleDeclarationWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		SEMANTIC_ASSERT(walker.type.declaration != 0);
		declaration = walker.declaration;
	}
	// occurs in for-init-statement
	void visit(cpp::simple_declaration* symbol)
	{
		SimpleDeclarationWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		SEMANTIC_ASSERT(walker.type.declaration != 0);
		declaration = walker.declaration;
	}
	void visit(cpp::constructor_definition* symbol)
	{
		SimpleDeclarationWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		SEMANTIC_ASSERT(walker.type.declaration != 0);
		declaration = walker.declaration;
	}
	void visit(cpp::template_declaration* symbol)
	{
		TemplateDeclarationWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
	}
	void visit(cpp::explicit_specialization* symbol)
	{
		TemplateDeclarationWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
	}
	void visit(cpp::using_declaration* symbol)
	{
		UsingDeclarationWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::using_directive* symbol)
	{
		UsingDirectiveWalker walker(getState());
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

	NamespaceWalker(const WalkerState& state)
		: WalkerBase(state), declaration(0), id(0)
	{
	}

	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF(symbol);
		id = &symbol->value;
	}
	void visit(cpp::terminal<boost::wave::T_LEFTBRACE> symbol)
	{
		if(id != 0)
		{
			declaration = pointOfDeclaration(context, enclosing, *id, TYPE_NAMESPACE, 0);
			id->dec.p = declaration;
			if(declaration->enclosed == 0)
			{
				declaration->enclosed = newScope(*id, SCOPETYPE_NAMESPACE);
			}
			pushScope(declaration->enclosed);
		}
	}
	void visit(cpp::declaration* symbol)
	{
		DeclarationWalker walker(getState());
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
		SymbolPrinter walker(getState());
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
#endif

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
#ifndef MINGLE
		WalkerContext context;
		Walker::NamespaceWalker walker(context);
		walker.visit(makeSymbol(p));
#endif

		SymbolPrinter printer(path);
		printer.visit(makeSymbol(p));
	}
	catch(SemanticError&)
	{
	}

}


TreeAllocator<int> getAllocator(ParserContext& lexer)
{
#ifdef TREEALLOCATOR_LINEAR
	return lexer.allocator;
#else
	return DebugAllocator<int>();
#endif
}

#ifdef MINGLE
cpp::declaration_seq* parseFile(ParserContext& lexer)
{
	WalkerContext& context = *new WalkerContext(getAllocator(lexer));
	Walker::NamespaceWalker& walker = *new Walker::NamespaceWalker(context);
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
	dumpProfile(gProfileIo);
	dumpProfile(gProfileWave);
	dumpProfile(gProfileParser);
	dumpProfile(gProfileLookup);
	dumpProfile(gProfileDiagnose);
	dumpProfile(gProfileAllocator);
	dumpProfile(gProfileIdentifier);
	dumpProfile(gProfileTemplateId);

	return result;
}

cpp::statement_seq* parseFunction(ParserContext& lexer)
{
	WalkerContext& context = *new WalkerContext(getAllocator(lexer));
	Walker::CompoundStatementWalker& walker = *new Walker::CompoundStatementWalker(context);
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

