
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
#define TreeAllocator ParserAllocatorWrapper
#define TREEALLOCATOR_NULL TreeAllocator<int>(NullParserAllocator())
#else
#define TreeAllocator DebugAllocator
#define TREEALLOCATOR_NULL TreeAllocator<int>()
#endif

// ----------------------------------------------------------------------------
// sequence

template<typename Visitor>
struct SequenceNode
{
	Reference<SequenceNode> next;

	virtual ~SequenceNode()
	{
	}
	virtual void accept(Visitor& visitor) const = 0;
	virtual bool operator==(const SequenceNode& other) const = 0;

	const SequenceNode* get() const
	{
		return next.get();
	}
};

template<typename Visitor>
struct SequenceNodeEmpty : SequenceNode<Visitor>
{
	virtual void accept(Visitor& visitor) const
	{
		throw SymbolsError();
	}
	virtual bool operator==(const SequenceNode<Visitor>& other) const
	{
		throw SymbolsError();
	}
	virtual bool operator<(const SequenceNode<Visitor>& other) const
	{
		throw SymbolsError();
	}
};

template<typename T, typename Visitor>
struct SequenceNodeGeneric : Reference< SequenceNode<Visitor> >::Value
{
	T value;
	SequenceNodeGeneric(const T& value)
		: value(value)
	{
	}
	void accept(Visitor& visitor) const
	{
		visitor.visit(value);
	}
	bool operator==(const SequenceNode<Visitor>& other) const
	{
		return typeid(*this) == typeid(other)
			&& value == static_cast<const SequenceNodeGeneric*>(&other)->value;
	}
	bool operator<(const SequenceNode<Visitor>& other) const
	{
		return (typeid(*this).before(typeid(other)) ||
			!(typeid(other).before(typeid(*this))) && value < static_cast<const SequenceNodeGeneric*>(&other)->value);
	}
};

template<typename A, typename Visitor>
struct Sequence : A
{
	typedef SequenceNode<Visitor> Node;
	typedef Reference<Node> Pointer;
	SequenceNodeEmpty<Visitor> head;

	A& getAllocator()
	{
		return *this;
	}
	const A& getAllocator() const
	{
		return *this;
	}

	Sequence()
	{
		construct();
	}
	Sequence(const A& allocator)
		:  A(allocator)
	{
		construct();
	}
	Sequence& operator=(Sequence other)
	{
		head = other.head;
		return *this;
	}
	void construct()
	{
		head.next = 0;
	}

	bool empty() const
	{
		return head.next == Pointer(0);
	}
	void clear()
	{
		construct();
	}

	template<typename T>
	void push_front(const T& value)
	{
		Pointer node = allocatorNew(getAllocator(), SequenceNodeGeneric<T, Visitor>(value));
		node->next = head.next;
		head.next = node;
	}
	template<typename T>
	void push_back(const T& value)
	{
		Pointer node = allocatorNew(getAllocator(), SequenceNodeGeneric<T, Visitor>(value));
		if(empty())
		{
			node->next = head.next;
			head.next = node;
		}
		else
		{
			Pointer last = head.next;
			for(Pointer next = last->next; next != 0; next = next->next)
			{
				last = next;
			}
			node->next = 0;
			last->next = node;
		}
	}
	void pop_front()
	{
#ifdef ALLOCATOR_DEBUG
		SYMBOLS_ASSERT(head.next.p->count == 1);
#endif
		SYMBOLS_ASSERT(!empty());
		Pointer node = head.next;
		head.next = node->next;
		allocatorDelete(getAllocator(), node.get());
	}

	void swap(Sequence& other)
	{
		head.next.swap(other.head.next);
	}
	void reverse()
	{
#ifdef ALLOCATOR_DEBUG
		SYMBOLS_ASSERT(head.next.p->count == 1);
#endif
		Pointer root = head.next;
		head.next = 0;
		while(root != 0)
		{
			Pointer next = root->next;
			root->next = head.next;
			head.next = root;
			root = next;
		}
	}

	const Node* get() const
	{
		return head.next.get();
	}

	void accept(Visitor& visitor) const
	{
		for(const Node* node = get(); node != 0; node = node->get())
		{
			node->accept(visitor);
		}
	}
};

template<typename Visitor>
const SequenceNode<Visitor>* findLast(const SequenceNode<Visitor>* node)
{
	SYMBOLS_ASSERT(node != 0);
	const SequenceNode<Visitor>* next = node->get();
	if(next == 0)
	{
		return node;
	}
	return findLast(next);
}



// ----------------------------------------------------------------------------
// type sequence


struct TypeElementVisitor
{
	virtual void visit(const struct DeclaratorObject&) = 0;
	virtual void visit(const struct DeclaratorPointer&) = 0;
	virtual void visit(const struct DeclaratorReference&) = 0;
	virtual void visit(const struct DeclaratorArray&) = 0;
	virtual void visit(const struct DeclaratorMemberPointer&) = 0;
	virtual void visit(const struct DeclaratorFunction&) = 0;
};

typedef Sequence<TreeAllocator<int>, TypeElementVisitor> TypeSequence;

// ----------------------------------------------------------------------------
// type


typedef ListReference<struct TemplateArgument, TreeAllocator<struct TemplateArgument> > TemplateArguments2;

// wrapper to disable default-constructor
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




typedef ListReference<struct Type, TreeAllocator<int> > Types2;

// wrapper to disable default-constructor
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

typedef ListReference<struct TypeId, TreeAllocator<int> > TypeIds2;

// wrapper to disable default-constructor
struct TypeIds : public TypeIds2
{
	TypeIds(const TreeAllocator<int>& allocator)
		: TypeIds2(allocator)
	{
	}
private:
	TypeIds()
	{
	}
};


struct Type;

typedef Reference<const Type> TypePtr;

typedef CopiedReference<const Type, TreeAllocator<int> > TypeRef;

typedef TypeRef Qualifying;


class Declaration;
typedef SafePtr<Declaration> DeclarationPtr;
struct Scope;
typedef SafePtr<Scope> ScopePtr;

const size_t INDEX_INVALID = size_t(-1);

struct Type
{
	DeclarationPtr declaration;
	TemplateArguments templateArguments; // may be non-empty if this is a template
	Qualifying qualifying;
	DeclarationPtr dependent;
	ScopePtr enclosingTemplate;
	size_t specialization;
	bool isImplicitTemplateId; // true if this is a template but the template-argument-clause has not been specified
	mutable bool visited; // use while iterating a set of types, to avoid visiting the same type twice (an optimisation, and a mechanism for handling cyclic dependencies)
	Type(Declaration* declaration, const TreeAllocator<int>& allocator)
		: declaration(declaration), templateArguments(allocator), qualifying(allocator), dependent(0), enclosingTemplate(0), specialization(INDEX_INVALID), isImplicitTemplateId(false), visited(false)
	{
	}
	void swap(Type& other)
	{
		std::swap(declaration, other.declaration);
		templateArguments.swap(other.templateArguments);
		qualifying.swap(other.qualifying);
		std::swap(dependent, other.dependent);
		std::swap(enclosingTemplate, other.enclosingTemplate);
		std::swap(specialization, other.specialization);
		std::swap(isImplicitTemplateId, other.isImplicitTemplateId);
	}
	Type& operator=(Declaration* declaration)
	{
		SYMBOLS_ASSERT(templateArguments.empty());
		SYMBOLS_ASSERT(qualifying.empty());
		SYMBOLS_ASSERT(specialization == INDEX_INVALID);
		this->declaration = declaration;
		return *this;
	}
private:
	Type();
};

struct TypeId : Type
{
	TypeSequence typeSequence;

	TypeId(Declaration* declaration, const TreeAllocator<int>& allocator)
		: Type(declaration, allocator), typeSequence(allocator)
	{
	}
	TypeId& operator=(Declaration* declaration)
	{
		SYMBOLS_ASSERT(typeSequence.empty());
		Type::operator=(declaration);
		return *this;
	}
	void swap(TypeId& other)
	{
		Type::swap(other);
		typeSequence.swap(other.typeSequence);
	}
	void swap(Type& other)
	{
		SYMBOLS_ASSERT(typeSequence.empty());
		Type::swap(other);
	}
};

#define TYPE_NULL TypeId(0, TREEALLOCATOR_NULL)

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

struct ReferenceCallbacksOpaque
{
	typedef void (*Function0)(void*);
	Function0 increment;
	Function0 decrement;
};

struct DependencyCallbacksOpaque : ReferenceCallbacksOpaque
{
	typedef bool (*IsDependent)(void*, const DependentContext&);
	IsDependent isDependent;
};

template<typename T>
struct DependencyCallbacks : DependencyCallbacksOpaque
{
};

template<typename T>
struct ReferenceCallbacksDefault
{
	static void increment(T*)
	{
	}
	static void decrement(T*)
	{
	}
};

template<typename T>
struct ReferenceCallbacks
{
	static void increment(typename Reference<T>::Value* p)
	{
#ifdef ALLOCATOR_DEBUG
		++p->count;
#endif
	}
	static void decrement(typename Reference<T>::Value* p)
	{
#ifdef ALLOCATOR_DEBUG
		--p->count;
#endif
	}
};

template<typename T>
DependencyCallbacks<T> makeDependencyCallbacks(
	bool (*isDependent)(T*, const DependentContext&),
	void (*increment)(T*) = ReferenceCallbacksDefault<T>::increment,
	void (*decrement)(T*) = ReferenceCallbacksDefault<T>::decrement
)
{
	DependencyCallbacks<T> result;
	result.isDependent = DependencyCallbacksOpaque::IsDependent(isDependent);
	result.increment = DependencyCallbacksOpaque::Function0(increment);
	result.decrement = DependencyCallbacksOpaque::Function0(decrement);
	return result;
}

struct DependencyCallback
{
	void* context;
	DependencyCallbacksOpaque* callbacks;

	DependencyCallback(void* context, DependencyCallbacksOpaque* callbacks)
		: context(context), callbacks(callbacks)
	{
		increment();
	}
	DependencyCallback(const DependencyCallback& other)
		: context(other.context), callbacks(other.callbacks)
	{
		increment();
	}
	~DependencyCallback()
	{
		decrement();
	}
	void swap(DependencyCallback& other)
	{
		std::swap(context, other.context);
		std::swap(callbacks, other.callbacks);
	}

	bool isDependent(const DependentContext& args) const
	{
		return callbacks->isDependent(context, args);
	}
	void increment()
	{
#ifdef ALLOCATOR_DEBUG
		callbacks->increment(context);
#endif
	}
	void decrement()
	{
#ifdef ALLOCATOR_DEBUG
		callbacks->decrement(context);
#endif
	}
};

template<typename T>
DependencyCallback makeDependencyCallback(T* context, DependencyCallbacks<T>* callbacks)
{
	return DependencyCallback(const_cast<typename TypeTraits<T>::Value*>(context), callbacks);
}

typedef ListReference<DependencyCallback, TreeAllocator<int> > Dependent2;

struct Dependent : public Dependent2
{
	Dependent(const TreeAllocator<int>& allocator) : Dependent2(allocator)
	{
	}
	void splice(Dependent& other)
	{
		Dependent2::splice(begin(), other);
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
// deferred lookup of dependent names

typedef ListReference<struct DeferredLookupCallback, TreeAllocator<int> > DeferredLookup2;

struct DeferredLookup : public DeferredLookup2
{
	DeferredLookup(const TreeAllocator<int>& allocator) : DeferredLookup2(allocator)
	{
	}
	void splice(DeferredLookup& other)
	{
		DeferredLookup2::splice(begin(), other);
	}
private:
	DeferredLookup();
};

// ----------------------------------------------------------------------------
// template-argument

struct TemplateArgument
{
	TypeId type;
	Dependent dependent;
#if 0
	TemplateArgument(const TypeId& type) : type(type)
	{
	}
#endif
	TemplateArgument(const TreeAllocator<int>& allocator)
		: type(0, allocator), dependent(allocator)
	{
	}
};

inline TemplateArguments& nullTemplateArguments()
{
	const TreeAllocator<int> allocator = TREEALLOCATOR_NULL;
	static TemplateArguments null(allocator);
	return null;
}
#define TEMPLATEARGUMENTS_NULL nullTemplateArguments()



// ----------------------------------------------------------------------------
// identifier

typedef cpp::terminal_identifier Identifier;

inline Identifier makeIdentifier(const char* value)
{
	Identifier result = { TokenValue(value) };
	return result;
}

const Identifier IDENTIFIER_NULL = Identifier();

inline const char* getValue(const Identifier& id)
{
	return id.value.empty() ? "$unnamed" : id.value.c_str();
}

typedef SafePtr<Identifier> IdentifierPtr;

// ----------------------------------------------------------------------------
// declaration


class Declaration
{
	Identifier* name;

#if 0
	Declaration(const Declaration&);
	Declaration& operator=(const Declaration&);
#endif
public:
	Scope* scope;
	TypeId type;
	Scope* enclosed;
	Declaration* overloaded;
	Dependent valueDependent; // the dependent-types/names that are referred to in the declarator-suffix (array size)
	DeclSpecifiers specifiers;
	size_t templateParameter;
	Types templateParams;
	TypeIds templateParamDefaults;
	TemplateArguments templateArguments;
	bool isTemplate;

	Declaration(
		const TreeAllocator<int>& allocator,
		Scope* scope,
		Identifier& name,
		const TypeId& type,
		Scope* enclosed,
		DeclSpecifiers specifiers = DeclSpecifiers(),
		bool isTemplate = false,
		const TemplateArguments& templateArguments = TEMPLATEARGUMENTS_NULL,
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
		templateArguments(templateArguments),
		isTemplate(isTemplate)
	{
	}
	Declaration() :
		type(0, TREEALLOCATOR_NULL),
		valueDependent(TREEALLOCATOR_NULL),
		templateParams(TREEALLOCATOR_NULL),
		templateParamDefaults(TREEALLOCATOR_NULL),
		templateArguments(TREEALLOCATOR_NULL)
	{
	}
	void swap(Declaration& other)
	{
		std::swap(name, other.name);
		std::swap(scope, other.scope);
		type.swap(other.type);
		std::swap(enclosed, other.enclosed);
		std::swap(overloaded, other.overloaded);
		valueDependent.swap(other.valueDependent);
		std::swap(specifiers, other.specifiers);
		std::swap(templateParameter, other.templateParameter);
		templateParams.swap(other.templateParams);
		templateParamDefaults.swap(other.templateParamDefaults);
		templateArguments.swap(other.templateArguments);
		std::swap(isTemplate, other.isTemplate);
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
		sprintf(buffer, "$%x", unsigned(index));
		Identifier::value = TokenValue(buffer);
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

#if 1
#define DeclarationHolder Declaration
#else
struct DeclarationHolder : Declaration
{
	DeclarationHolder()
	{
	}
	DeclarationHolder(const DeclarationHolder&)
	{
	}
};
#endif

struct Scope : public ScopeCounter
{
	ScopePtr parent;
	Identifier name;
	size_t enclosedScopeCount; // number of scopes directly enclosed by this scope
	typedef std::less<TokenValue> IdentifierLess;

	typedef std::multimap<TokenValue, DeclarationHolder, IdentifierLess, TreeAllocator<int> > Declarations2;

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

		Declaration* insert(Declaration& other)
		{
			Scope::Declarations::iterator result = Declarations2::insert(Scope::Declarations::value_type(other.getName().value, DeclarationHolder()));
			(*result).second.swap(other);
			return &(*result).second;
		}
	};


	Declarations declarations;
	ScopeType type;
	Types bases;
	typedef List<ScopePtr, TreeAllocator<int> > Scopes;
	Scopes usingDirectives;
	typedef List<DeclarationPtr, TreeAllocator<int> > DeclarationList;
	DeclarationList declarationList;

	DeferredLookup deferred;
	size_t deferredCount;
	bool isTemplate;

	Scope(const TreeAllocator<int>& allocator, const Identifier& name, ScopeType type = SCOPETYPE_UNKNOWN)
		: parent(0), name(name), enclosedScopeCount(0), declarations(allocator), type(type), bases(allocator), usingDirectives(allocator), declarationList(allocator), deferred(allocator), deferredCount(0), isTemplate(false)

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

private:
	//Scope(const Scope&);
	//Scope& operator=(const Scope&);
};

inline Scope::Declarations::iterator findDeclaration(Scope::Declarations& declarations, Declaration* declaration)
{
	const Identifier& id = declaration->getName();
	Scope::Declarations::iterator i = declarations.upper_bound(id.value);

	for(; i != declarations.begin()
		&& (*--i).first == id.value;)
	{
		if(&(*i).second == declaration)
		{
			return i;
		}
	}

	return declarations.end();
}

inline void undeclare(Declaration* declaration, LexerAllocator& allocator)
{
	SYMBOLS_ASSERT(declaration->getName().dec.p == 0 || declaration->getName().dec.p == declaration);
	declaration->getName().dec.p = 0;

	SYMBOLS_ASSERT(!declaration->scope->declarations.empty());
	SYMBOLS_ASSERT(!declaration->scope->declarationList.empty());

	SYMBOLS_ASSERT(declaration == declaration->scope->declarationList.back());
	declaration->scope->declarationList.pop_back(); // TODO: optimise

	Scope::Declarations::iterator i = findDeclaration(declaration->scope->declarations, declaration);
	SYMBOLS_ASSERT(i != declaration->scope->declarations.end());
	declaration->scope->declarations.erase(i);

}

inline BacktrackCallback makeUndeclareCallback(Declaration* p)
{
	BacktrackCallback result = { BacktrackCallbackThunk<Declaration, undeclare>::thunk, p };
	return result;
}


inline const Scope& nullScope()
{
	static Scope null(TREEALLOCATOR_NULL, IDENTIFIER_NULL);
	return null;
}
#define SCOPE_NULL nullScope()

inline bool enclosesEts(ScopeType type)
{
	return type == SCOPETYPE_NAMESPACE
		|| type == SCOPETYPE_LOCAL;
}


// ----------------------------------------------------------------------------
// template instantiation
inline const TypeId& getTemplateArgument(const Type& type, size_t index)
{
	TemplateArguments::const_iterator a = type.templateArguments.begin();
	TypeIds::const_iterator p = type.declaration->templateParamDefaults.begin();
	for(;; --index)
	{
		if(a != type.templateArguments.end())
		{
			if(index == 0)
			{
				SYMBOLS_ASSERT((*a).type.declaration != 0);
				return (*a).type;
			}
			++a;
		}
		else
		{
			SYMBOLS_ASSERT(p != type.declaration->templateParamDefaults.end());
			if(index == 0)
			{
				SYMBOLS_ASSERT((*p).declaration != 0);
				return *p;
			}
			++p;
		}
	}
}

inline const Type& getInstantiatedSimpleType(const Type& type, const Qualifying& context);

inline bool isTypedef(const Type& type)
{
	return type.declaration->specifiers.isTypedef // if this is a typedef..
		&& type.declaration->templateParameter == INDEX_INVALID; // .. and not a template-parameter
}

// resolve Template<T>::Type -> T
inline const Type& getInstantiatedTypeInternal(const Type& original, const Qualifying& qualifying, const Qualifying& context = Qualifying(TREEALLOCATOR_NULL))
{
	size_t index = original.declaration->templateParameter;
	if(index != INDEX_INVALID) // if the original type is a template-parameter.
	{
		// Find the template-specialisation it belongs to:
		for(const Type* i = qualifying.get(); i != 0; i = (*i).qualifying.get())
		{
			const Type& instantiated = getInstantiatedSimpleType(*i, context); // qualifying types should always be simple
			if(instantiated.declaration->enclosed == original.declaration->scope)
			{
				return getTemplateArgument(instantiated, index); // TODO: instantiate the argument?
			}
		}
	}

	return original;
}

template<typename Inner>
inline const Type& getInstantiatedTypeGeneric(const Type& type, const Qualifying& context, Inner inner)
{
	if(type.declaration->specifiers.isTypedef)
	{ 
		// template<typename T> struct Template { typedef T Type; }; Template<int>::Type -> int
		const Type& instantiated = type.declaration->templateParameter == INDEX_INVALID // if type is not a template-parameter
			? getInstantiatedTypeInternal(inner(type.declaration->type, context), type.qualifying, context)
			: type;
		// template<typename T> struct Template { T m; }; Template<int> m; m.m -> int
		// if type was a template param, look it up in provided context,
		// else look up instantiated type (which may be template param) in provided context
		return getInstantiatedTypeInternal(instantiated, context);
	}
	return type;
}

inline const Type& getInstantiatedType(const Type& type, const Qualifying& context = Qualifying(TREEALLOCATOR_NULL))
{
	return getInstantiatedTypeGeneric(type, context, getInstantiatedType);
}

inline bool isSimple(const TypeId& type)
{
	return type.typeSequence.empty();
}

inline bool isSimple(const Declaration& declaration)
{
	return isSimple(declaration.type);
}

inline bool isSimple(const Type& type)
{
	return isSimple(*type.declaration);
}

// asserts that the resulting type is not a complex type. e.g. type-id found in nested-name-specifier
inline const Type& getInstantiatedSimpleType(const Type& type, const Qualifying& context)
{
	SYMBOLS_ASSERT(isSimple(type));
	return getInstantiatedTypeGeneric(type, context, getInstantiatedSimpleType);
}


// ----------------------------------------------------------------------------
// unique types
// Representation of a declarator, with type-elements linked in 'normal' order.
// e.g. int(*)[] == pointer to array of == DeclaratorPointer -> DeclaratorArray
// Note that this is the reverse of the order that the declarator is parsed in.
// This means a given unique type sub-sequence need only be stored once.
// This allows fast comparison of types and simplifies printing of declarators.
struct TypeElement
{
	const TypeElement* next;

	virtual ~TypeElement()
	{
	}
	virtual void accept(TypeElementVisitor& visitor) const = 0;
	virtual bool operator<(const TypeElement& other) const = 0;
};

struct TypeElementEmpty : TypeElement
{
	TypeElementEmpty()
	{
		next = 0;
	}
	virtual void accept(TypeElementVisitor& visitor) const
	{
		throw SymbolsError();
	}
	virtual bool operator<(const TypeElement& other) const
	{
		throw SymbolsError();
	}
};

extern const TypeElementEmpty gTypeElementEmpty;

template<typename T>
struct TypeElementGeneric : TypeElement
{
	T value;
	TypeElementGeneric(const T& value)
		: value(value)
	{
	}
	void accept(TypeElementVisitor& visitor) const
	{
		visitor.visit(value);
	}
	bool operator<(const TypeElement& other) const
	{
		return (typeid(*this).before(typeid(other)) ||
			!(typeid(other).before(typeid(*this))) && value < static_cast<const TypeElementGeneric*>(&other)->value);
	}
};

typedef const TypeElement* UniqueType;

const UniqueType UNIQUETYPE_NULL = &gTypeElementEmpty;


struct UniqueTypeLess
{
	bool operator()(UniqueType left, UniqueType right) const
	{
		return (*left < *right ||
			!(*right < *left) && left->next < right->next);
	}
};

typedef std::set<UniqueType, UniqueTypeLess> UniqueTypes;

extern UniqueTypes gBuiltInTypes;

template<typename T>
inline UniqueType pushUniqueType(UniqueTypes& types, UniqueType type, const T& value)
{
	TypeElementGeneric<T> node(value);
	node.next = type;
	{
		UniqueTypes::iterator i = gBuiltInTypes.find(&node);
		if(i != gBuiltInTypes.end())
		{
			return *i;
		}
	}
	UniqueTypes::iterator i = types.lower_bound(&node); // first element not less than value
	if(i != types.end()
		&& !types.key_comp()(&node, *i)) // if value is not less than lower bound
	{
		// lower bound is equal to value
		return *i;
	}
	return *types.insert(i, new TypeElementGeneric<T>(node)); // leaked deliberately
}

extern UniqueTypes gUniqueTypes;

template<typename T>
inline void pushUniqueType(UniqueType& type, const T& value)
{
	type = pushUniqueType(gUniqueTypes, type, value);
}

inline void popUniqueType(UniqueType& type)
{
	SYMBOLS_ASSERT(type != 0);
	type = type->next;
}

struct UniqueTypeWrapper
{
	UniqueType value;

	UniqueTypeWrapper()
		: value(&gTypeElementEmpty)
	{
	}
	explicit UniqueTypeWrapper(UniqueType value)
		: value(value)
	{
	}
	template<typename T>
	void push_front(const T& t)
	{
		pushUniqueType(value, t);
	}
	void pop_front()
	{
		popUniqueType(value);
	}
	void swap(UniqueTypeWrapper& other)
	{
		std::swap(value, other.value);
	}
	bool empty() const
	{
		return value == UNIQUETYPE_NULL;
	}
	bool isSimple() const
	{
		return typeid(*value) == typeid(TypeElementGeneric<DeclaratorObject>);
	}
	bool isPointer() const
	{
		return typeid(*value) == typeid(TypeElementGeneric<DeclaratorPointer>);
	}
	bool isReference() const
	{
		return typeid(*value) == typeid(TypeElementGeneric<DeclaratorReference>);
	}
	bool isArray() const
	{
		return typeid(*value) == typeid(TypeElementGeneric<DeclaratorArray>);
	}
	bool isMemberPointer() const
	{
		return typeid(*value) == typeid(TypeElementGeneric<DeclaratorMemberPointer>);
	}
	bool isFunction() const
	{
		return typeid(*value) == typeid(TypeElementGeneric<DeclaratorFunction>);
	}
	bool isSimplePointer() const
	{
		return isPointer()
			&& UniqueTypeWrapper(value->next).isSimple();
	}
	bool isSimpleReference() const
	{
		return isReference()
			&& UniqueTypeWrapper(value->next).isSimple();
	}
	bool isSimpleArray() const
	{
		return isArray()
			&& UniqueTypeWrapper(value->next).isSimple();
	}
};

inline bool operator==(UniqueTypeWrapper l, UniqueTypeWrapper r)
{
	return l.value == r.value;
}

inline bool operator<(UniqueTypeWrapper l, UniqueTypeWrapper r)
{
	return l.value < r.value;
}

#if 1
typedef UniqueTypeWrapper UniqueTypeId;

#else
struct UniqueTypeId : Type, UniqueTypeWrapper
{
	UniqueTypeId(Declaration* declaration, const TreeAllocator<int>& allocator)
		: Type(declaration, allocator)
	{
	}
	UniqueTypeId& operator=(Declaration* declaration)
	{
		SYMBOLS_ASSERT(UniqueTypeWrapper::empty());
		Type::operator=(declaration);
		return *this;
	}
	void swap(UniqueTypeId& other)
	{
		Type::swap(other);
		UniqueTypeWrapper::swap(other);
	}
	void swap(Type& other)
	{
		SYMBOLS_ASSERT(UniqueTypeWrapper::empty());
		Type::swap(other);
	}
};
#endif

const UniqueTypeId gUniqueTypeNull = UniqueTypeId(UNIQUETYPE_NULL);

inline bool isEqual(const UniqueTypeId& l, const UniqueTypeId& r)
{
	return l.value == r.value;
}

inline UniqueType getInner(UniqueType type)
{
	SYMBOLS_ASSERT(typeid(*type) != typeid(TypeElementGeneric<struct DeclaratorObject>));
	return type->next;
}

inline bool isEqualInner(const UniqueTypeId& l, const UniqueTypeId& r)
{
	return getInner(l.value) == getInner(r.value);
}

typedef std::vector<UniqueTypeWrapper> TemplateArgumentsInstance;
typedef std::vector<UniqueTypeWrapper> SpecializationTypes;
typedef std::vector<const struct TypeInstance*> UniqueBases;

struct TypeInstance
{
	DeclarationPtr declaration;
	TemplateArgumentsInstance templateArguments;
	const TypeInstance* enclosing; // the enclosing template
	UniqueBases bases;
	SpecializationTypes specializations; // the types of the dependent-names in the specialization
	TypeInstance(Declaration* declaration, const TypeInstance* enclosing)
		: declaration(declaration), enclosing(enclosing)
	{
		SYMBOLS_ASSERT(enclosing == 0 || enclosing->declaration->isTemplate);
	}
};

inline bool operator==(const TypeInstance& left, const TypeInstance& right)
{
	return left.declaration.p == right.declaration.p
		&& left.enclosing == right.enclosing
		&& left.templateArguments == right.templateArguments;
}

inline bool operator<(const TypeInstance& left, const TypeInstance& right)
{
	return left.declaration.p != right.declaration.p ? left.declaration.p < right.declaration.p
		: left.enclosing != right.enclosing ? left.enclosing < right.enclosing
		: left.templateArguments != right.templateArguments ? left.templateArguments < right.templateArguments
		: false;
}


struct DeclaratorObject
{
	TypeInstance type;
	DeclaratorObject(const TypeInstance& type)
		: type(type)
	{
	}
};

inline bool operator==(const DeclaratorObject& left, const DeclaratorObject& right)
{
	return left.type == right.type;
}

inline bool operator<(const DeclaratorObject& left, const DeclaratorObject& right)
{
	return left.type < right.type;
}

inline const TypeInstance& getObjectType(UniqueType type)
{
	SYMBOLS_ASSERT(typeid(*type) == typeid(TypeElementGeneric<DeclaratorObject>));
	return static_cast<const TypeElementGeneric<DeclaratorObject>*>(type)->value.type;
}




struct DeclaratorPointer
{
};

inline bool operator==(const DeclaratorPointer& left, const DeclaratorPointer& right)
{
	return true;
}

inline bool operator<(const DeclaratorPointer& left, const DeclaratorPointer& right)
{
	return false;
}

struct DeclaratorReference
{
};

inline bool operator==(const DeclaratorReference& left, const DeclaratorReference& right)
{
	return true;
}

inline bool operator<(const DeclaratorReference& left, const DeclaratorReference& right)
{
	return false;
}

struct DeclaratorMemberPointer
{
	const TypeInstance* type;
	DeclaratorMemberPointer()
		: type(0)
	{
	}
};

inline const TypeInstance& getMemberPointerClass(UniqueType type)
{
	SYMBOLS_ASSERT(typeid(*type) == typeid(TypeElementGeneric<DeclaratorMemberPointer>));
	return *(static_cast<const TypeElementGeneric<DeclaratorMemberPointer>*>(type)->value.type);
}

inline bool operator==(const DeclaratorMemberPointer& left, const DeclaratorMemberPointer& right)
{
	return true; // TODO
}

inline bool operator<(const DeclaratorMemberPointer& left, const DeclaratorMemberPointer& right)
{
	return false; // TODO
}

struct DeclaratorArray
{
	// TODO: size
};

inline bool operator==(const DeclaratorArray& left, const DeclaratorArray& right)
{
	return true;
}

inline bool operator<(const DeclaratorArray& left, const DeclaratorArray& right)
{
	return false;
}

struct DeclaratorFunction
{
	ScopePtr paramScope;
	DeclaratorFunction(Scope* scope)
		: paramScope(scope)
	{
	}
};

inline bool operator==(const DeclaratorFunction& left, const DeclaratorFunction& right)
{
	return left.paramScope == right.paramScope; // TODO
}

inline bool operator<(const DeclaratorFunction& left, const DeclaratorFunction& right)
{
	return left.paramScope < right.paramScope; // TODO
}




typedef std::vector<const Declaration*> DeclarationHistory;

struct GetTypeHistory
{
	DeclarationHistory& history;
	GetTypeHistory(DeclarationHistory& history)
		: history(history)
	{
	}
	const Type& operator()(const Type& type, const Qualifying& context) const
	{
		return apply(type, context, history);
	}
	static const Type& apply(const Type& type, const Qualifying& context, DeclarationHistory& history)
	{
		if(!isSimple(type))
		{
			history.push_back(type.declaration);
		}
		return getInstantiatedTypeGeneric(type, context, GetTypeHistory(history));
	}
	static const Type& apply(const Declaration& declaration, const Qualifying& context, DeclarationHistory& history)
	{
		if(!isSimple(declaration))
		{
			history.push_back(&declaration);
		}
		return apply(declaration.type, context, history);
	}
};

struct TypeSequenceReverseCopy : TypeElementVisitor
{
	UniqueType& type;
	TypeSequenceReverseCopy(UniqueType& type)
		: type(type)
	{
	}
	void visit(const DeclaratorObject& element)
	{
		pushUniqueType(type, element);
	}
	void visit(const DeclaratorPointer& element)
	{
		pushUniqueType(type, element);
	}
	void visit(const DeclaratorReference& element)
	{
		pushUniqueType(type, element);
	}
	void visit(const DeclaratorArray& element)
	{
		pushUniqueType(type, element);
	}
	void visit(const DeclaratorMemberPointer& element)
	{
		pushUniqueType(type, element);
	}
	void visit(const DeclaratorFunction& element)
	{
		SYMBOLS_ASSERT(element.paramScope != 0);
		pushUniqueType(type, element);
	}
};


inline Declaration* findPrimaryTemplate(Declaration* declaration)
{
	SYMBOLS_ASSERT(declaration->isTemplate);
	for(;declaration != 0; declaration = declaration->overloaded)
	{
		if(declaration->templateArguments.empty())
		{
			SYMBOLS_ASSERT(declaration->isTemplate);
			return declaration;
		}
	}
	SYMBOLS_ASSERT(false); // primary template not declared!
	return 0;
}


inline UniqueTypeWrapper makeUniqueType(const TypeId& type);
inline UniqueTypeWrapper makeUniqueType(const TypeId& type, const TypeInstance* enclosing);
inline UniqueTypeWrapper makeUniqueType(const Type& type, const TypeInstance* enclosing);

struct DeferredLookupCallback
{
	void* context;
	typedef UniqueTypeWrapper (*Callback)(void* context, const TypeInstance& enclosing);
	Callback callback;

	UniqueTypeWrapper evaluate(const TypeInstance& enclosing) const
	{
		return callback(context, enclosing);
	}
};

template<typename T>
DeferredLookupCallback makeDeferredLookupCallbackGeneric(UniqueTypeWrapper (*callback)(T*, const TypeInstance&), T* context)
{
	DeferredLookupCallback result = { context, DeferredLookupCallback::Callback(callback) };
	return result;
}

template<typename T>
inline UniqueTypeWrapper evaluateTypeGeneric(T* type, const TypeInstance& enclosing)
{
	size_t index = type->declaration->templateParameter; // TODO: template-template-parameter
	if(index != INDEX_INVALID)
	{
		// Find the template-specialisation it belongs to:
		for(const TypeInstance* i = &enclosing; i != 0; i = (*i).enclosing)
		{
			if((*i).declaration->enclosed == type->declaration->scope)
			{
				UniqueTypeWrapper result = (*i).templateArguments[index];
				// TODO: SYMBOLS_ASSERT(result.value != UNIQUETYPE_NULL);
				return result;
			}
		}
		throw SymbolsError();
		return gUniqueTypeNull; // error?
	}
	size_t tmp = type->specialization;
	type->specialization = INDEX_INVALID; // prevent makeUniqueType from accessing this type while it's being evaluated 
	UniqueTypeWrapper result = makeUniqueType(*type, &enclosing);
	// TODO: SYMBOLS_ASSERT(result.value != UNIQUETYPE_NULL);
	type->specialization = tmp;
	return result;
}

inline UniqueTypeWrapper evaluateTypeId(TypeId* type, const TypeInstance& enclosing)
{
	return evaluateTypeGeneric(type, enclosing);
}

inline UniqueTypeWrapper evaluateType(Type* type, const TypeInstance& enclosing)
{
	return evaluateTypeGeneric(type, enclosing);
}

inline bool isTemplate(const Scope& scope)
{
	return scope.isTemplate;
}

inline Scope* getEnclosingTemplate(Scope* enclosing)
{
	for(Scope* scope = enclosing; scope != 0; scope = scope->parent)
	{
		if(isTemplate(*scope))
		{
			return scope;
		}
	}
	return 0;
}

inline DeferredLookupCallback makeDeferredLookupCallback(Type* type)
{
	return makeDeferredLookupCallbackGeneric(evaluateType, type);
}

inline DeferredLookupCallback makeDeferredLookupCallback(TypeId* type)
{
	return makeDeferredLookupCallbackGeneric(evaluateTypeId, type);
}

template<typename T>
inline void addDeferredLookupType(T* type, Scope* enclosingTemplate)
{
	SYMBOLS_ASSERT(enclosingTemplate != 0);
	SYMBOLS_ASSERT(type->enclosingTemplate == 0);
	enclosingTemplate->deferred.push_back(makeDeferredLookupCallback(type));
	type->specialization = enclosingTemplate->deferredCount++;
	type->enclosingTemplate = enclosingTemplate;
}

inline void makeUniqueTemplateArguments(const TemplateArguments& arguments, const TypeIds& templateParamDefaults, TemplateArgumentsInstance& result, const TypeInstance* enclosing = 0)
{
	TemplateArguments::const_iterator a = arguments.begin();
	for(TypeIds::const_iterator i = templateParamDefaults.begin(); i != templateParamDefaults.end(); ++i)
	{
		const TypeId& argument = a != arguments.end() ? (*a++).type : (*i);
		UniqueTypeWrapper type;
		if(argument.declaration != 0) // ignore non-type arguments
		{
			type = makeUniqueType(argument, enclosing);
		}
		result.push_back(type);
	}
}

inline bool findScope(Scope* scope, Scope* other)
{
	if(scope == 0)
	{
		return false;
	}
	if(scope == other)
	{
		return true;
	}
	return findScope(scope->parent, other);
}

// Returns true if /p type is dependent.
inline bool isDependent(const Type& type, Scope* enclosing)
{
	return type.dependent != DeclarationPtr(0)
		&& findScope(enclosing, type.dependent->scope);
}

inline const TypeInstance* makeUniqueEnclosing(const Qualifying& qualifying, const TypeInstance* enclosing)
{
	if(!qualifying.empty())
	{
		return &getObjectType(makeUniqueType(*qualifying.get(), enclosing).value);
	}
	return enclosing;
}

inline const TypeInstance* makeUniqueEnclosing(const Qualifying& qualifying)
{
	return makeUniqueEnclosing(qualifying, 0);
}

inline const TypeInstance* getEnclosingTemplate(const TypeInstance* enclosing)
{
	for(; enclosing; enclosing = enclosing->enclosing)
	{
		if(enclosing->declaration->isTemplate)
		{
			break;
		}
	}
	return enclosing;
}

inline void evaluateBases(const TypeInstance& enclosing)
{
	TypeInstance& instance = const_cast<TypeInstance&>(enclosing);
	Types& bases = enclosing.declaration->enclosed->bases;
	size_t count = std::distance(bases.begin(), bases.end());
	if(instance.bases.size() != count)
	{
		instance.bases.reserve(std::distance(bases.begin(), bases.end()));
		for(Types::const_iterator i = bases.begin(); i != bases.end(); ++i)
		{
			instance.bases.push_back(&getObjectType(makeUniqueType(*i, &enclosing).value));
		}
	}
}

inline const TypeInstance* findEnclosingTemplate(const TypeInstance& enclosing, Scope* enclosingTemplate)
{
	if(enclosing.declaration->enclosed == enclosingTemplate)
	{
		return &enclosing;
	}

	evaluateBases(enclosing);

	for(UniqueBases::const_iterator i = enclosing.bases.begin(); i != enclosing.bases.end(); ++i)
	{
		const TypeInstance* result = findEnclosingTemplate(*(*i), enclosingTemplate);
		if(result != 0)
		{
			return result;
		}
	}
	return 0;
}

// unqualified object name: int, Object,
// qualified object name: Qualifying::Object
// unqualified typedef: Typedef, TemplateParam
// qualified typedef: Qualifying::Type
inline UniqueTypeWrapper makeUniqueType(const Type& type, const TypeInstance* enclosing)
{
	extern Declaration gDependentType;
	if(type.declaration == &gDependentType)
	{
		return gUniqueTypeNull; // TODO!
	}
	enclosing = makeUniqueEnclosing(type.qualifying, enclosing);
	if(type.specialization != INDEX_INVALID)
	{
		SYMBOLS_ASSERT(enclosing != 0);
		for(const TypeInstance* i = enclosing; i != 0; i = (*i).enclosing)
		{
			const TypeInstance* enclosingTemplate = findEnclosingTemplate(*i, type.enclosingTemplate);
			if(enclosingTemplate != 0)
			{
				{
					TypeInstance& instance = *const_cast<TypeInstance*>(enclosingTemplate);
					SYMBOLS_ASSERT(enclosingTemplate->declaration->isTemplate) // TODO: isImplicitTemplateId
					if(instance.declaration->enclosed->deferredCount != 0
						&& instance.specializations.empty())
					{
						instance.specializations.reserve(instance.declaration->enclosed->deferredCount);
						for(DeferredLookup::const_iterator i = instance.declaration->enclosed->deferred.begin(); i != instance.declaration->enclosed->deferred.end(); ++i)
						{
							instance.specializations.push_back((*i).evaluate(instance));
							// TODO: SYMBOLS_ASSERT(instance.specializations.back().value != UNIQUETYPE_NULL);
						}
					}
					//SYMBOLS_ASSERT(instance.declaration->enclosed->deferredCount == instance.specializations.size());
				}
				SYMBOLS_ASSERT(type.specialization < (*enclosingTemplate).specializations.size());
				UniqueTypeWrapper result = (*enclosingTemplate).specializations[type.specialization];
				// TODO: SYMBOLS_ASSERT(result.value != UNIQUETYPE_NULL);
				return result;
			}
		}
		throw SymbolsError();
		return gUniqueTypeNull; // error?
	}
	if(type.declaration->specifiers.isTypedef)
	{
		return makeUniqueType(type.declaration->type, enclosing);
	}
	TypeInstance tmp(type.declaration, getEnclosingTemplate(enclosing));
	if(type.declaration->isTemplate)
	{
		Declaration* primary = findPrimaryTemplate(type.declaration); // TODO: look up explicit specialization
		makeUniqueTemplateArguments(type.templateArguments, primary->templateParamDefaults, tmp.templateArguments, enclosing);
	}
	return UniqueTypeWrapper(pushUniqueType(gUniqueTypes, UNIQUETYPE_NULL, DeclaratorObject(tmp)));
}

inline UniqueTypeWrapper makeUniqueType(const Type& type)
{
	return makeUniqueType(type, 0);
}

inline UniqueTypeWrapper makeUniqueType(const TypeId& type, const TypeInstance* enclosing)
{
	UniqueTypeWrapper result = makeUniqueType(*static_cast<const Type*>(&type), enclosing);
	TypeSequenceReverseCopy copier(result.value);
	type.typeSequence.accept(copier);
	return result;
}

inline UniqueTypeWrapper makeUniqueType(const TypeId& type)
{
	return makeUniqueType(type, 0);
}


// ----------------------------------------------------------------------------

inline Declaration* findTemplateSpecialization(Declaration* declaration, const TemplateArguments& arguments)
{
	SYMBOLS_ASSERT(declaration->isTemplate);
	Declaration* primary = findPrimaryTemplate(declaration);
	TemplateArgumentsInstance instance;
	makeUniqueTemplateArguments(arguments, primary->templateParamDefaults, instance);
	for(;declaration != 0; declaration = declaration->overloaded)
	{
		if(declaration->templateArguments.empty())
		{
			continue;
		}
		TemplateArgumentsInstance other;
		makeUniqueTemplateArguments(declaration->templateArguments, primary->templateParamDefaults, other);
		if(instance == other)
		{
			return declaration;
		}
	}
	return primary;
}

// ----------------------------------------------------------------------------
// expression helper

template<typename T, bool isExpression = IsConvertible<T, cpp::expression>::RESULT>
struct ExpressionType;

template<typename T>
struct ExpressionType<T, false>
{
	static UniqueTypeId get(T* symbol)
	{
		return gUniqueTypeNull;
	}
	static void set(T* symbol, UniqueTypeId declaration)
	{
	}
};

template<typename T>
inline UniqueTypeId getExpressionType(T* symbol)
{
	return UniqueTypeId(symbol->dec.p);
}
template<typename T>
inline void setExpressionType(T* symbol, UniqueTypeId value)
{
	symbol->dec.p = value.value;
}

template<typename T>
struct ExpressionType<T, true>
{
	static UniqueTypeId get(T* symbol)
	{
		return getExpressionType(symbol);
	}
	static void set(T* symbol, UniqueTypeId declaration)
	{
		setExpressionType(symbol, declaration);
	}
};


// ----------------------------------------------------------------------------
// built-in symbols

// special-case
extern Declaration gUndeclared;


// meta types
extern Declaration gArithmetic;
extern Declaration gSpecial;
extern Declaration gClass;
extern Declaration gEnum;

#define TYPE_ARITHMETIC TypeId(&gArithmetic, TREEALLOCATOR_NULL)
#define TYPE_SPECIAL TypeId(&gSpecial, TREEALLOCATOR_NULL)
#define TYPE_CLASS TypeId(&gClass, TREEALLOCATOR_NULL)
#define TYPE_ENUM TypeId(&gEnum, TREEALLOCATOR_NULL)

// types
struct BuiltInTypeDeclaration : Declaration
{
	BuiltInTypeDeclaration(Identifier& name, const TypeId& type = TYPE_ARITHMETIC)
		: Declaration(TREEALLOCATOR_NULL, 0, name, type, 0)
	{
	}
};


extern Declaration gNamespace;

#define TYPE_NAMESPACE TypeId(&gNamespace, TREEALLOCATOR_NULL)

extern Declaration gCtor;
extern Declaration gEnumerator;
extern Declaration gUnknown;

#define TYPE_CTOR TypeId(&gCtor, TREEALLOCATOR_NULL)
#define TYPE_ENUMERATOR TypeId(&gEnumerator, TREEALLOCATOR_NULL)
#define TYPE_UNKNOWN TypeId(&gUnknown, TREEALLOCATOR_NULL)


template<typename T>
inline UniqueType pushBuiltInType(UniqueType type, const T& value)
{
	TypeElementGeneric<T> node(value);
	node.next = type;
	return *gBuiltInTypes.insert(new TypeElementGeneric<T>(node)).first; // leaked deliberately
}

struct ObjectTypeId : UniqueTypeId
{
	ObjectTypeId(Declaration* declaration, const TreeAllocator<int>& allocator)
	{
		value = pushBuiltInType(value, DeclaratorObject(TypeInstance(declaration, 0)));
	}
};

// fundamental types
extern BuiltInTypeDeclaration gCharDeclaration;
extern BuiltInTypeDeclaration gSignedCharDeclaration;
extern BuiltInTypeDeclaration gUnsignedCharDeclaration;
extern BuiltInTypeDeclaration gSignedShortIntDeclaration;
extern BuiltInTypeDeclaration gUnsignedShortIntDeclaration;
extern BuiltInTypeDeclaration gSignedIntDeclaration;
extern BuiltInTypeDeclaration gUnsignedIntDeclaration;
extern BuiltInTypeDeclaration gSignedLongIntDeclaration;
extern BuiltInTypeDeclaration gUnsignedLongIntDeclaration;
extern BuiltInTypeDeclaration gSignedLongLongIntDeclaration;
extern BuiltInTypeDeclaration gUnsignedLongLongIntDeclaration;
extern BuiltInTypeDeclaration gWCharTDeclaration;
extern BuiltInTypeDeclaration gBoolDeclaration;
extern BuiltInTypeDeclaration gFloatDeclaration;
extern BuiltInTypeDeclaration gDoubleDeclaration;
extern BuiltInTypeDeclaration gLongDoubleDeclaration;
extern BuiltInTypeDeclaration gVoidDeclaration;
extern ObjectTypeId gChar;
extern ObjectTypeId gSignedChar;
extern ObjectTypeId gUnsignedChar;
extern ObjectTypeId gSignedShortInt;
extern ObjectTypeId gUnsignedShortInt;
extern ObjectTypeId gSignedInt;
extern ObjectTypeId gUnsignedInt;
extern ObjectTypeId gSignedLongInt;
extern ObjectTypeId gUnsignedLongInt;
extern ObjectTypeId gSignedLongLongInt;
extern ObjectTypeId gUnsignedLongLongInt;
extern ObjectTypeId gWCharT;
extern ObjectTypeId gBool;
extern ObjectTypeId gFloat;
extern ObjectTypeId gDouble;
extern ObjectTypeId gLongDouble;
extern ObjectTypeId gVoid;

struct StringLiteralTypeId : ObjectTypeId
{
	StringLiteralTypeId(Declaration* declaration, const TreeAllocator<int>& allocator)
		: ObjectTypeId(declaration, allocator)
	{
		value = pushBuiltInType(value, DeclaratorArray());
	}
};

extern StringLiteralTypeId gStringLiteral;
extern StringLiteralTypeId gWideStringLiteral;

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
	case MAKE_FUNDAMENTAL(CHAR): return &gCharDeclaration;
	case MAKE_FUNDAMENTAL(SIGNED) | MAKE_FUNDAMENTAL(CHAR): return &gSignedCharDeclaration;
	case MAKE_FUNDAMENTAL(UNSIGNED) | MAKE_FUNDAMENTAL(CHAR): return &gUnsignedCharDeclaration;
	case MAKE_FUNDAMENTAL(SHORT):
	case MAKE_FUNDAMENTAL(SHORT) | MAKE_FUNDAMENTAL(INT):
	case MAKE_FUNDAMENTAL(SIGNED) | MAKE_FUNDAMENTAL(SHORT):
	case MAKE_FUNDAMENTAL(SIGNED) | MAKE_FUNDAMENTAL(SHORT) | MAKE_FUNDAMENTAL(INT): return &gSignedShortIntDeclaration;
	case MAKE_FUNDAMENTAL(UNSIGNED) | MAKE_FUNDAMENTAL(SHORT):
	case MAKE_FUNDAMENTAL(UNSIGNED) | MAKE_FUNDAMENTAL(SHORT) | MAKE_FUNDAMENTAL(INT): return &gUnsignedShortIntDeclaration;
	case MAKE_FUNDAMENTAL(INT):
	case MAKE_FUNDAMENTAL(SIGNED):
	case MAKE_FUNDAMENTAL(SIGNED) | MAKE_FUNDAMENTAL(INT): return &gSignedIntDeclaration;
	case MAKE_FUNDAMENTAL(UNSIGNED):
	case MAKE_FUNDAMENTAL(UNSIGNED) | MAKE_FUNDAMENTAL(INT): return &gUnsignedIntDeclaration;
	case MAKE_FUNDAMENTAL(LONG):
	case MAKE_FUNDAMENTAL(LONG) | MAKE_FUNDAMENTAL(INT):
	case MAKE_FUNDAMENTAL(SIGNED) | MAKE_FUNDAMENTAL(LONG):
	case MAKE_FUNDAMENTAL(SIGNED) | MAKE_FUNDAMENTAL(LONG) | MAKE_FUNDAMENTAL(INT): return &gSignedLongIntDeclaration;
	case MAKE_FUNDAMENTAL(UNSIGNED) | MAKE_FUNDAMENTAL(LONG):
	case MAKE_FUNDAMENTAL(UNSIGNED) | MAKE_FUNDAMENTAL(LONG) | MAKE_FUNDAMENTAL(INT): return &gUnsignedLongIntDeclaration;
	case MAKE_FUNDAMENTAL(LONG) | MAKE_FUNDAMENTAL2(LONG):
	case MAKE_FUNDAMENTAL(SIGNED) | MAKE_FUNDAMENTAL(LONG) | MAKE_FUNDAMENTAL2(LONG):
	case MAKE_FUNDAMENTAL(SIGNED) | MAKE_FUNDAMENTAL(LONG) | MAKE_FUNDAMENTAL2(LONG) | MAKE_FUNDAMENTAL(INT): return &gSignedLongLongIntDeclaration;
	case MAKE_FUNDAMENTAL(UNSIGNED) | MAKE_FUNDAMENTAL(LONG) | MAKE_FUNDAMENTAL2(LONG):
	case MAKE_FUNDAMENTAL(UNSIGNED) | MAKE_FUNDAMENTAL(LONG) | MAKE_FUNDAMENTAL2(LONG) | MAKE_FUNDAMENTAL(INT): return &gUnsignedLongLongIntDeclaration;
	case MAKE_FUNDAMENTAL(WCHAR_T): return &gWCharTDeclaration;
	case MAKE_FUNDAMENTAL(BOOL): return &gBoolDeclaration;
	case MAKE_FUNDAMENTAL(FLOAT): return &gFloatDeclaration;
	case MAKE_FUNDAMENTAL(DOUBLE): return &gDoubleDeclaration;
	case MAKE_FUNDAMENTAL(LONG) | MAKE_FUNDAMENTAL(DOUBLE): return &gLongDoubleDeclaration;
	case MAKE_FUNDAMENTAL(VOID): return &gVoidDeclaration;
	}
	SYMBOLS_ASSERT(false);
	return 0;
}

#define MAKE_INTEGERLITERALSUFFIX(token) (1 << cpp::simple_type_specifier_builtin::token)

inline bool isHexadecimalLiteral(const char* value)
{
	return *value++ == '0'
		&& (*value == 'x' || *value == 'X');
}

inline bool isFloatingLiteral(const char* value)
{
	if(!isHexadecimalLiteral(value))
	{
		const char* p = value;
		for(; *p != '\0'; ++p)
		{
			if(std::strchr(".eE", *p) != 0)
			{
				return true;
			}
		}
	}
	return false;
}

inline const char* getIntegerLiteralSuffix(const char* value)
{
	const char* p = value;
	for(; *p != '\0'; ++p)
	{
		if(std::strchr("ulUL", *p) != 0)
		{
			break;
		}
	}
	return p;
}

inline const UniqueTypeId& getIntegerLiteralType(const char* value)
{
	const char* suffix = getIntegerLiteralSuffix(value);
	if(*suffix == '\0')
	{
		return gSignedInt; // TODO: return long on overflow
	}
	if(*(suffix + 1) == '\0') // u U l L
	{
		return *suffix == 'u' || *suffix == 'U' ? gUnsignedInt : gSignedLongInt; // TODO: return long/unsigned on overflow
	}
	if(*(suffix + 2) == '\0') // ul lu uL Lu Ul lU UL LU
	{
		return gUnsignedLongInt; // TODO: long long
	}
	throw SymbolsError();
}

inline const char* getFloatingLiteralSuffix(const char* value)
{
	const char* p = value;
	for(; *p != '\0'; ++p)
	{
		if(std::strchr("flFL", *p) != 0)
		{
			break;
		}
	}
	return p;
}

inline const UniqueTypeId& getFloatingLiteralType(const char* value)
{
	const char* suffix = getFloatingLiteralSuffix(value);
	if(*suffix == '\0')
	{
		return gDouble;
	}
	if(*(suffix + 1) == '\0') // f F l L
	{
		return *suffix == 'f' || *suffix == 'F' ? gFloat : gLongDouble;
	}
	throw SymbolsError();
}

inline const UniqueTypeId& getCharacterLiteralType(const char* value)
{
	return *value == 'L' ? gWCharT : gSignedChar; // TODO: multicharacter literal
}

inline const UniqueTypeId& getNumericLiteralType(cpp::numeric_literal* symbol)
{
	const char* value = symbol->value.value.c_str();
	switch(symbol->id)
	{
	case cpp::numeric_literal::INTEGER: return getIntegerLiteralType(value);
	case cpp::numeric_literal::CHARACTER: return getCharacterLiteralType(value);
	case cpp::numeric_literal::FLOATING: return getFloatingLiteralType(value);
	case cpp::numeric_literal::BOOLEAN: return gBool;
	default: break;
	}
	throw SymbolsError();
}

inline const UniqueTypeId& getStringLiteralType(cpp::string_literal* symbol)
{
	const char* value = symbol->value.value.c_str();
	return *value == 'L' ? gWideStringLiteral : gStringLiteral;
}



extern Declaration gDependentType;
extern Declaration gDependentObject;
extern Declaration gDependentTemplate;
extern Declaration gDependentNested;

extern Declaration gParam;

#define TYPE_PARAM TypeId(&gParam, TREEALLOCATOR_NULL)


// objects
extern Name gOperatorNewId;
extern Name gOperatorDeleteId;
extern Name gOperatorNewArrayId;
extern Name gOperatorDeleteArrayId;
extern Name gOperatorPlusId;
extern Name gOperatorMinusId;
extern Name gOperatorStarId;
extern Name gOperatorDivideId;
extern Name gOperatorPercentId;
extern Name gOperatorXorId;
extern Name gOperatorAndId;
extern Name gOperatorOrId;
extern Name gOperatorComplId;
extern Name gOperatorNotId;
extern Name gOperatorAssignId;
extern Name gOperatorLessId;
extern Name gOperatorGreaterId;
extern Name gOperatorPlusAssignId;
extern Name gOperatorMinusAssignId;
extern Name gOperatorStarAssignId;
extern Name gOperatorDivideAssignId;
extern Name gOperatorPercentAssignId;
extern Name gOperatorXorAssignId;
extern Name gOperatorAndAssignId;
extern Name gOperatorOrAssignId;
extern Name gOperatorShiftLeftId;
extern Name gOperatorShiftRightId;
extern Name gOperatorShiftRightAssignId;
extern Name gOperatorShiftLeftAssignId;
extern Name gOperatorEqualId;
extern Name gOperatorNotEqualId;
extern Name gOperatorLessEqualId;
extern Name gOperatorGreaterEqualId;
extern Name gOperatorAndAndId;
extern Name gOperatorOrOrId;
extern Name gOperatorPlusPlusId;
extern Name gOperatorMinusMinusId;
extern Name gOperatorCommaId;
extern Name gOperatorArrowStarId;
extern Name gOperatorArrowId;
extern Name gOperatorFunctionId;
extern Name gOperatorArrayId;

inline Name getOverloadableOperatorId(cpp::overloadable_operator_default* symbol)
{
	switch(symbol->id)
	{
	case cpp::overloadable_operator_default::ASSIGN: return gOperatorAssignId;
	case cpp::overloadable_operator_default::STARASSIGN: return gOperatorStarAssignId;
	case cpp::overloadable_operator_default::DIVIDEASSIGN: return gOperatorDivideAssignId;
	case cpp::overloadable_operator_default::PERCENTASSIGN: return gOperatorPercentAssignId;
	case cpp::overloadable_operator_default::PLUSASSIGN: return gOperatorPlusAssignId;
	case cpp::overloadable_operator_default::MINUSASSIGN: return gOperatorMinusAssignId;
	case cpp::overloadable_operator_default::SHIFTRIGHTASSIGN: return gOperatorShiftRightAssignId;
	case cpp::overloadable_operator_default::SHIFTLEFTASSIGN: return gOperatorShiftLeftAssignId;
	case cpp::overloadable_operator_default::ANDASSIGN: return gOperatorAndAssignId;
	case cpp::overloadable_operator_default::XORASSIGN: return gOperatorXorAssignId;
	case cpp::overloadable_operator_default::ORASSIGN: return gOperatorOrAssignId;
	case cpp::overloadable_operator_default::EQUAL: return gOperatorEqualId;
	case cpp::overloadable_operator_default::NOTEQUAL: return gOperatorNotEqualId;
	case cpp::overloadable_operator_default::LESS: return gOperatorLessId;
	case cpp::overloadable_operator_default::GREATER: return gOperatorGreaterId;
	case cpp::overloadable_operator_default::LESSEQUAL: return gOperatorLessEqualId;
	case cpp::overloadable_operator_default::GREATEREQUAL: return gOperatorGreaterEqualId;
	case cpp::overloadable_operator_default::ANDAND: return gOperatorAndAndId;
	case cpp::overloadable_operator_default::OROR: return gOperatorOrOrId;
	case cpp::overloadable_operator_default::PLUSPLUS: return gOperatorPlusPlusId;
	case cpp::overloadable_operator_default::MINUSMINUS: return gOperatorMinusMinusId;
	case cpp::overloadable_operator_default::STAR: return gOperatorStarId;
	case cpp::overloadable_operator_default::DIVIDE: return gOperatorDivideId;
	case cpp::overloadable_operator_default::PERCENT: return gOperatorPercentId;
	case cpp::overloadable_operator_default::PLUS: return gOperatorPlusId;
	case cpp::overloadable_operator_default::MINUS: return gOperatorMinusId;
	case cpp::overloadable_operator_default::SHIFTLEFT: return gOperatorShiftLeftId;
	case cpp::overloadable_operator_default::SHIFTRIGHT: return gOperatorShiftRightId;
	case cpp::overloadable_operator_default::AND: return gOperatorAndId;
	case cpp::overloadable_operator_default::OR: return gOperatorOrId;
	case cpp::overloadable_operator_default::XOR: return gOperatorXorId;
	case cpp::overloadable_operator_default::NOT: return gOperatorNotId;
	case cpp::overloadable_operator_default::COMPL: return gOperatorComplId;
	case cpp::overloadable_operator_default::ARROW: return gOperatorArrowId;
	case cpp::overloadable_operator_default::ARROWSTAR: return gOperatorArrowStarId;
	case cpp::overloadable_operator_default::COMMA: return gOperatorCommaId;
	default: break;
	}
	throw SymbolsError();
}

inline Name getOverloadableOperatorId(cpp::new_operator* symbol)
{
	if(symbol->array.p != 0)
	{
		return gOperatorNewArrayId;
	}
	return gOperatorNewId;
}

inline Name getOverloadableOperatorId(cpp::delete_operator* symbol)
{
	if(symbol->array.p != 0)
	{
		return gOperatorDeleteArrayId;
	}
	return gOperatorDeleteId;
}

inline Name getOverloadableOperatorId(cpp::function_operator* symbol)
{
	return gOperatorFunctionId;
}

inline Name getOverloadableOperatorId(cpp::array_operator* symbol)
{
	return gOperatorArrayId;
}

inline Name getUnaryOperatorName(cpp::unary_operator* symbol)
{
	switch(symbol->id)
	{
	case cpp::unary_operator::PLUSPLUS: return gOperatorPlusPlusId;
	case cpp::unary_operator::MINUSMINUS: return gOperatorMinusMinusId;
	case cpp::unary_operator::STAR: return gOperatorStarId;
	case cpp::unary_operator::AND: return gOperatorAndId;
	case cpp::unary_operator::PLUS: return gOperatorPlusId;
	case cpp::unary_operator::MINUS: return gOperatorMinusId;
	case cpp::unary_operator::NOT: return gOperatorNotId;
	case cpp::unary_operator::COMPL: return gOperatorComplId;
	default: break;
	}
	throw SymbolsError();
}



extern Identifier gConversionFunctionId;
extern Identifier gOperatorFunctionTemplateId;
// TODO: don't declare if id is anonymous?
extern Identifier gAnonymousId;


inline const UniqueTypeId& binaryOperatorAssignment(const UniqueTypeId& left, const UniqueTypeId& right)
{
	return left;
}

inline const UniqueTypeId& binaryOperatorComma(const UniqueTypeId& left, const UniqueTypeId& right)
{
	return right;
}

inline const UniqueTypeId& binaryOperatorBoolean(const UniqueTypeId& left, const UniqueTypeId& right)
{
	return gBool;
}

inline const UniqueTypeId& binaryOperatorNull(const UniqueTypeId& left, const UniqueTypeId& right)
{
	return gUniqueTypeNull;
}



inline bool isType(const Declaration& type)
{
	return type.specifiers.isTypedef
		|| type.type.declaration == &gArithmetic
		|| type.type.declaration == &gSpecial
		|| type.type.declaration == &gEnum
		|| type.type.declaration == &gClass;
}

inline bool isFunction(const Declaration& declaration)
{
	return declaration.enclosed != 0 && declaration.enclosed->type == SCOPETYPE_PROTOTYPE;
}

inline bool isMember(const Declaration& declaration)
{
	return declaration.scope->type == SCOPETYPE_CLASS;
}

inline bool isMemberOfTemplate(const Declaration& declaration)
{
	return isMember(declaration) && isTemplate(*declaration.scope);
}

inline bool isNonMember(const Declaration& declaration)
{
	return !isMember(declaration);
}

inline bool isMemberObject(const Declaration& declaration)
{
	return isMember(declaration)
		&& !isFunction(declaration);
}

inline bool isMemberFunction(const Declaration& declaration)
{
	return isMember(declaration)
		&& isFunction(declaration);
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

inline bool isComplete(const Declaration& declaration)
{
	return declaration.enclosed != 0;
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

inline bool isClass(const UniqueTypeId& type)
{
	return type.isSimple() && getObjectType(type.value).declaration->type.declaration == &gClass;
}

inline bool isComplete(const UniqueTypeId& type)
{
	return type.isSimple() && isComplete(*getObjectType(type.value).declaration);
}

inline bool isArithmetic(const UniqueTypeId& type)
{
	return type.isSimple() && getObjectType(type.value).declaration->type.declaration == &gArithmetic;
}

inline bool isFloating(const UniqueTypeId& type)
{
	return isEqual(type, gFloat)
		|| isEqual(type, gDouble)
		|| isEqual(type, gLongDouble);
}

inline bool isIntegral(const UniqueTypeId& type)
{
	return isArithmetic(type) && !isFloating(type);
}

inline bool isEnumerator(const UniqueTypeId& type)
{
	return type.isSimple() && getObjectType(type.value).declaration == &gEnumerator;
}

inline bool isEnumeration(const UniqueTypeId& type)
{
	return (type.isSimple() && getObjectType(type.value).declaration->type.declaration == &gEnum)
		|| isEnumerator(type); // TODO: remove when enumerators are correctly typed
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



enum IcsRank
{
	ICSRANK_STANDARDEXACT,
	ICSRANK_STANDARDPROMOTION,
	ICSRANK_STANDARDCONVERSION,
	ICSRANK_USERDEFINED,
	ICSRANK_ELLIPSIS,
	ICSRANK_INVALID,
};

inline bool findBase(const TypeInstance& other, const TypeInstance& type)
{
	if(other.declaration == &gParam)
	{
		return false; // TODO: when type-evaluation fails, sometimes template-params are uniqued
	}
	SYMBOLS_ASSERT(other.declaration->enclosed != 0);
	SYMBOLS_ASSERT(isClass(*type.declaration));
	evaluateBases(other);
	for(UniqueBases::const_iterator i = other.bases.begin(); i != other.bases.end(); ++i)
	{
		const TypeInstance& base = *(*i);
		SYMBOLS_ASSERT(isClass(*base.declaration));
		if(&base == &type)
		{
			return true;
		}
		if(findBase(base, type))
		{
			return true;
		}
	}
	return false;
}

// Returns true if 'type' is a base of 'other'
inline bool isBaseOf(const TypeInstance& type, const TypeInstance& other)
{
	if(!isClass(*type.declaration)
		|| !isClass(*other.declaration))
	{
		return false;
	}
	if(isIncomplete(*type.declaration)
		|| isIncomplete(*other.declaration))
	{
		return false;
	}
	return findBase(other, type);
}

// 4.5 Integral Promotions
// TODO: handle bitfield types?
inline const UniqueTypeId& promoteToIntegralType(const UniqueTypeId& type)
{
	if(isEqual(type, gChar)
		|| isEqual(type, gSignedChar)
		|| isEqual(type, gUnsignedChar)
		|| isEqual(type, gSignedShortInt)
		|| isEqual(type, gUnsignedShortInt))
	{
		return gSignedInt;
	}
	if(isEqual(type, gWCharT)
		|| isEnumeration(type))
	{
		return gSignedInt;
	}
	if(isEqual(type, gBool))
	{
		return gSignedInt;
	}
	return type;
}

inline bool isPromotion(const UniqueTypeId& to, const UniqueTypeId& from)
{
	if(isArithmetic(from) && from.isSimple()
		&& isArithmetic(to) && to.isSimple())
	{
		return (isEqual(from, gFloat) && isEqual(to, gDouble))
			|| (isEqual(promoteToIntegralType(from), to));
	}
	return false;
}

inline bool isConversion(const UniqueTypeId& to, const UniqueTypeId& from, bool isNullPointerConstant = false) // TODO: detect null pointer constant
{
	if((isArithmetic(from) || isEnumeration(from)) && from.isSimple()
		&& isArithmetic(to) && to.isSimple())
	{
		// can convert from enumeration to integer/floating/bool, but not in reverse
		return true;
	}
	if((to.isPointer() || to.isMemberPointer())
		&& isIntegral(from)
		&& isNullPointerConstant)
	{
		return true; // 0 -> T*
	}
	if(to.isSimplePointer()
		&& from.isSimplePointer()
		&& getInner(to.value) == gVoid.value)
	{
		return true; // T* -> void*
	}
	if(to.isSimplePointer()
		&& from.isSimplePointer()
		&& isBaseOf(getObjectType(getInner(to.value)), getObjectType(getInner(from.value))))
	{
		return true; // D* -> B*
	}
	if(to.isMemberPointer()
		&& from.isMemberPointer()
		&& isBaseOf(getMemberPointerClass(to.value), getMemberPointerClass(from.value)))
	{
		return true; // D::* -> B::*
	}
	if(isEqual(to, gBool)
		&& (from.isPointer() || from.isMemberPointer()))
	{
		return true; // T* -> bool, T::* -> bool
	}
	if(to.isSimple()
		&& from.isSimple()
		&& isBaseOf(getObjectType(to.value), getObjectType(from.value)))
	{
		return true; // D -> B
	}
	if(to.isPointer()
		&& from.isArray()
		&& isEqualInner(to, from))
	{
		return true; // T[] -> T*
	}
	return false;
}

inline IcsRank getIcsRank(const UniqueTypeId& to, const UniqueTypeId& from, bool isNullPointerConstant = false)
{
	// TODO: user-defined conversion
	if(from.value == UNIQUETYPE_NULL)
	{
		return ICSRANK_INVALID; // TODO: assert
	}
	if(isEqual(to, from))
	{
		return ICSRANK_STANDARDEXACT;
	}
	if(isPromotion(to, from))
	{
		return ICSRANK_STANDARDPROMOTION;
	}
	if(isConversion(to, from, isNullPointerConstant))
	{
		return ICSRANK_STANDARDCONVERSION; // TODO: ordering of conversions by inheritance distance
	}
	// TODO: user-defined
	// TODO: ellipsis
	return ICSRANK_INVALID;
}

struct ImplicitConversion
{
	IcsRank rank;
	ImplicitConversion(IcsRank rank)
		: rank(rank)
	{
	}
};

inline bool isBetter(const ImplicitConversion& l, const ImplicitConversion& r)
{
	return l.rank < r.rank;
}



typedef std::vector<ImplicitConversion> ArgumentConversions;
struct CandidateFunction
{
	Declaration* declaration;
	ArgumentConversions conversions;
	bool isTemplate;
	CandidateFunction()
		: declaration(0)
	{
	}
	CandidateFunction(Declaration* declaration)
		: declaration(declaration), isTemplate(false)
	{
	}
};

inline bool isBetter(const CandidateFunction& l, const CandidateFunction& r)
{
	SYMBOLS_ASSERT(l.conversions.size() == r.conversions.size());
	for(size_t i = 0; i != l.conversions.size(); ++i)
	{
		if(isBetter(r.conversions[i], l.conversions[i]))
		{
			return false; // at least one argument is not a better conversion sequence
		}
	}
	for(size_t i = 0; i != l.conversions.size(); ++i)
	{
		if(isBetter(l.conversions[i], r.conversions[i]))
		{
			return true; // at least one argument is a better conversion sequence
		}
	}
	if(!l.isTemplate && r.isTemplate)
	{
		return true; // non-template better than template
	}
	// TODO: ordering of template specialisations
	// TODO: in context of initialisation by user defined conversion, consider return type
	return false;
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
		return isDependent(original.qualifying.get(), context);
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
		if(isDependent(original.templateArguments, context))
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
		findTypes(original.templateArguments, result);
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
		for(TemplateArguments::const_iterator i = original.templateArguments.begin(); i != original.templateArguments.end(); ++i)
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
	if(type.visited)
	{
		return false;
	}

	type.visited = true;
	bool result = isDependentFast(type, context);
#if 0
	bool alternative = isDependentInternal(type, context);
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

inline bool isDependentType(const Type* type, const DependentContext& context)
{
	return isDependent(*type, context);
}

inline bool isDependentTypeRef(TypePtr::Value* type, const DependentContext& context)
{
	return isDependentType(type, context);
}

inline bool isDependentClass(Scope* scope, const DependentContext& context)
{
	return isDependent(scope->bases, context);
}

inline bool isDependentListRef(Reference<Dependent>::Value* dependent, const DependentContext& context)
{
	return evaluateDependent(*dependent, context);
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
	return *declaration.getName().value.c_str() == '$';
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
			if(!second.templateArguments.empty())
			{
				return second; // TODO
			}
			if(!first.templateArguments.empty())
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
		const Type& base = getInstantiatedType(*i);
		// an identifier looked up in the context of a class may name a base class
		if(base.declaration->templateParameter == INDEX_INVALID // TODO: don't look in dependent base classes!
			&& base.declaration->getName().value == id.value
			&& filter(*base.declaration))
		{
			result.filtered = base.declaration;
			return result;
		}
		Scope* scope = base.declaration->enclosed;
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



typedef TokenPrinter<std::ostream> FileTokenPrinter;

struct TypeElementOpaque
{
	const void* p;
	typedef void (*VisitCallback)(TypeElementVisitor& visitor, const void* p);
	VisitCallback callback;
	void accept(TypeElementVisitor& visitor)
	{
		callback(visitor, p);
	}
};

template<typename ElementType>
struct TypeElementThunk
{
	static void thunk(TypeElementVisitor& visitor, const void* p)
	{
		visitor.visit(*static_cast<const ElementType*>(p));
	}
};

template<typename ElementType>
TypeElementOpaque makeTypeElementOpaque(const ElementType& element)
{
	TypeElementOpaque result = { &element, &TypeElementThunk<ElementType>::thunk };
	return result;
}

typedef std::list<TypeElementOpaque> TypeElements;

struct TypeElementsAppend : TypeElementVisitor
{
	TypeElements& typeElements;
	TypeElementsAppend(TypeElements& typeElements)
		: typeElements(typeElements)
	{
	}

	template<typename T>
	void visitGeneric(const T& element)
	{
		typeElements.push_back(makeTypeElementOpaque(element));
	}
	void visit(const DeclaratorObject& element)
	{
		visitGeneric(element);
	}
	void visit(const DeclaratorReference& element)
	{
		visitGeneric(element);
	}
	void visit(const DeclaratorPointer& element)
	{
		visitGeneric(element);
	}
	void visit(const DeclaratorArray& element)
	{
		visitGeneric(element);
	}
	void visit(const DeclaratorMemberPointer& element)
	{
		visitGeneric(element);
	}
	void visit(const DeclaratorFunction& element)
	{
		visitGeneric(element);
	}
};

struct SymbolPrinter : TypeElementVisitor
{
	FileTokenPrinter& printer;
	SymbolPrinter(FileTokenPrinter& printer)
		: printer(printer)
	{
		typeStack.push_back(false);
	}
#if 0
	template<typename T>
	void printSymbol(T* symbol)
	{
		SourcePrinter walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
#endif
	void printName(const Scope* scope)
	{
		if(scope != 0
			&& scope->parent != 0)
		{
			printName(scope->parent);
			printer.out << getValue(scope->name) << ".";
		}
	}

	std::vector<bool> typeStack;

	void pushType(bool isPointer)
	{
		bool wasPointer = typeStack.back();
		bool parenthesise = typeStack.size() != 1 && !wasPointer && isPointer;
		if(parenthesise)
		{
			printer.out << "(";
		}
		typeStack.back() = parenthesise;
		typeStack.push_back(isPointer);
	}
	void popType()
	{
		typeStack.pop_back();
		if(typeStack.back())
		{
			printer.out << ")";
		}
	}

	const TypeSequence::Node* nextElement;

	void visit(const DeclaratorObject& object)
	{
		printName(object.type.declaration);
		visitTypeElement();
	}
	void visit(const DeclaratorReference& pointer)
	{
		pushType(true);
		printer.out << "&";
		visitTypeElement();
		popType();
	}
	void visit(const DeclaratorPointer& pointer)
	{
		pushType(true);
		printer.out << "*";
		visitTypeElement();
		popType();
	}
	void visit(const DeclaratorArray&)
	{
		pushType(false);
		visitTypeElement();
		printer.out << "[]";
		popType();
	}
	void visit(const DeclaratorMemberPointer&)
	{
		pushType(true);
		printer.out << "::*";
		visitTypeElement();
		popType();
	}
	void visit(const DeclaratorFunction& function)
	{
		pushType(false);
		visitTypeElement();
		printParameters(function.paramScope->declarationList);
		popType();
	}

	TypeElements typeElements;

	void visitTypeElement()
	{
		if(!typeElements.empty())
		{
			TypeElementOpaque element = typeElements.front();
			typeElements.pop_front();
			element.accept(*this);
		}
	}

	void visitTypeHistory(const DeclarationHistory& typeHistory)
	{
		for(DeclarationHistory::const_reverse_iterator i = typeHistory.rbegin(); i != typeHistory.rend(); ++i)
		{
			const Declaration& declaration = *(*i);
			for(const TypeSequence::Node* node = declaration.type.typeSequence.get(); node != 0; node = node->get())
			{
				TypeElementsAppend visitor(typeElements);
				node->accept(visitor);
			}
		}
	}

	void printTypeSequence(const Type& type)
	{
		DeclarationHistory typeHistory;
		GetTypeHistory::apply(type, Qualifying(TREEALLOCATOR_NULL), typeHistory);
		visitTypeHistory(typeHistory);
		visitTypeElement();
	}

	void printTypeSequence(const Declaration& declaration)
	{
		DeclarationHistory typeHistory;
		GetTypeHistory::apply(declaration, Qualifying(TREEALLOCATOR_NULL), typeHistory);
		visitTypeHistory(typeHistory);
		visitTypeElement();
	}

	void printType(const Type& type)
	{
		printName(getInstantiatedType(type).declaration);
		printTypeSequence(type);
	}

	void printType(const Declaration& declaration)
	{
		printName(getInstantiatedType(declaration.type).declaration);
		printTypeSequence(declaration);
	}

	void printType(const UniqueTypeId& type)
	{
		for(UniqueType i = type.value; i != UNIQUETYPE_NULL; i = i->next)
		{
			TypeElementsAppend visitor(typeElements);
			i->accept(visitor);
		}
		typeElements.reverse();
		visitTypeElement();
	}

	void printParameters(const Scope::DeclarationList& parameters)
	{
		printer.out << "(";
		bool separator = false;
		for(Scope::DeclarationList::const_iterator i = parameters.begin(); i != parameters.end(); ++i)
		{
			const Declaration* declaration = *i;
			if(declaration->templateParameter == INDEX_INVALID)
			{
				if(separator)
				{
					printer.out << ",";
				}
				SymbolPrinter walker(printer);
				walker.printType(*declaration);
				separator = true;
			}
		}
		printer.out << ")";
	}

	void printName(const Declaration* name)
	{
		if(name == 0)
		{
			printer.out << "<unknown>";
		}
		else
		{
			printName(name->scope);
			printer.out << getValue(name->getName());
			if(name->enclosed != 0
				&& name->enclosed->type == SCOPETYPE_PROTOTYPE)
			{
				printParameters(name->enclosed->declarationList);
			}
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

inline void printName(const Declaration* name)
{
	FileTokenPrinter tokenPrinter(std::cout);
	SymbolPrinter printer(tokenPrinter);
	printer.printName(name);
}

typedef ListReference<UniqueTypeId, TreeAllocator<int> > UniqueTypeIds2;

// wrapper to disable default-constructor
struct UniqueTypeIds : public UniqueTypeIds2
{
	UniqueTypeIds(const TreeAllocator<int>& allocator)
		: UniqueTypeIds2(allocator)
	{
	}
private:
	UniqueTypeIds()
	{
	}
};

struct OverloadResolver
{
	const UniqueTypeIds& arguments;
	CandidateFunction best;
	Declaration* ambiguous;

	OverloadResolver(const UniqueTypeIds& arguments)
		: arguments(arguments), ambiguous(0)
	{
		size_t count = std::distance(arguments.begin(), arguments.end());
		best.conversions.resize(count, ImplicitConversion(ICSRANK_INVALID));
	}
	Declaration* get() const
	{
		return ambiguous != 0 ? 0 : best.declaration;
	}
	void add(const CandidateFunction& candidate)
	{
		if(best.declaration != 0
			&& candidate.declaration->enclosed == best.declaration->enclosed)
		{
			return; // TODO: don't add multiple declarations of same signature
		}

		if(candidate.conversions.size() != best.conversions.size())
		{
			return; // TODO: early-out for functions with not enough params
		}

		if(isBetter(candidate, best))
		{
			best = candidate;
			ambiguous = 0;
		}
		else if(!isBetter(best, candidate)) // the best candidate is an equally good match
		{
			ambiguous = candidate.declaration;
		}
	}
	void add(Declaration* declaration)
	{
		CandidateFunction candidate(declaration);
		candidate.conversions.reserve(best.conversions.size());

		UniqueTypeIds::const_iterator a = arguments.begin();
		for(Scope::DeclarationList::iterator i = declaration->enclosed->declarationList.begin(); i != declaration->enclosed->declarationList.end(); ++i)
		{
			const Declaration& parameter = *(*i);
			UniqueTypeId type = makeUniqueType(parameter.type); // TODO: template argument deduction
			if(a != arguments.end())
			{
				candidate.conversions.push_back(getIcsRank(type, *a));
				++a;
			}
			else
			{
				break; // TODO: default parameter values
			}
		}

		add(candidate);
	}

};

inline Declaration* findBestMatch(Declaration* declaration, const UniqueTypeIds& arguments)
{
	OverloadResolver resolver(arguments);
	for(Declaration* p = declaration; p != 0; p = p->overloaded) // note this list may contain multiple (forward) declarations of the same function
	{
		// TODO
		// Gather all the functions in the current scope that have the same name as the function called.
		// Exclude those that don't have the right number of parameters to match the arguments in the call. (be careful about parameters with default values; void f(int x, int y = 0) is a candidate for the call f(25);)
		// If no function matches, the compiler reports an error.
		// If there is more than one match, select the 'best match'.
		// If there is no clear winner of the best matches, the compiler reports an error - ambiguous function call.
		if(p->enclosed == 0)
		{
			continue;
		}

		if(p->isTemplate)
		{
			continue; // TODO: template argument deduction
		}

		resolver.add(p);
	}

	if(resolver.ambiguous != 0)
	{
#if 0
		std::cout << "overload resolution failed:" << std::endl;
		std::cout << "  ";
		printPosition(resolver.ambiguous->getName().position);
		printName(resolver.ambiguous);
		std::cout << std::endl;
		if(resolver.best.declaration != 0)
		{
			std::cout << "  ";
			printPosition(resolver.best.declaration->getName().position);
			printName(resolver.best.declaration);
			std::cout << std::endl;
		}
#endif
	}

	return resolver.get();
}


#endif


