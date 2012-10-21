
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
	//virtual bool operator==(const SequenceNode& other) const = 0;

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
	virtual void visit(const struct DeclaratorDependent&) = 0;
	virtual void visit(const struct DeclaratorObject&) = 0;
	virtual void visit(const struct DeclaratorPointer&) = 0;
	virtual void visit(const struct DeclaratorReference&) = 0;
	virtual void visit(const struct DeclaratorArray&) = 0;
	virtual void visit(const struct DeclaratorMemberPointer&) = 0;
	virtual void visit(const struct DeclaratorFunction&) = 0;
};

typedef Sequence<TreeAllocator<int>, TypeElementVisitor> TypeSequence;

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
	IdentifierPtr id;
	DeclarationPtr declaration;
	TemplateArguments templateArguments; // may be non-empty if this is a template
	Qualifying qualifying;
	DeclarationPtr dependent;
	ScopePtr enclosingTemplate;
	size_t specialization;
	bool isImplicitTemplateId; // true if this is a template but the template-argument-clause has not been specified
	mutable bool visited; // use while iterating a set of types, to avoid visiting the same type twice (an optimisation, and a mechanism for handling cyclic dependencies)
	Type(Declaration* declaration, const TreeAllocator<int>& allocator)
		: id(0), declaration(declaration), templateArguments(allocator), qualifying(allocator), dependent(0), enclosingTemplate(0), specialization(INDEX_INVALID), isImplicitTemplateId(false), visited(false)
	{
	}
	void swap(Type& other)
	{
		std::swap(id, other.id);
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
	CvQualifiers qualifiers;
 
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
		std::swap(qualifiers, other.qualifiers);
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
	DeclarationPtr enclosingTemplate;
	Dependent(const TreeAllocator<int>& allocator) : Dependent2(allocator), enclosingTemplate(0)
	{
	}
	void swap(Dependent& other)
	{
		Dependent2::swap(other);
		std::swap(enclosingTemplate, other.enclosingTemplate);
	}
private:
	void splice(Dependent& other)
	{
		Dependent2::splice(begin(), other);
	}
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

const TemplateArguments TEMPLATEARGUMENTS_NULL = TemplateArguments(TREEALLOCATOR_NULL);


struct TemplateParameter : Type
{
	TypeId argument;
	TemplateParameter(const TreeAllocator<int>& allocator)
		: Type(0, allocator), argument(0, allocator)
	{
	}
	void swap(TemplateParameter& other)
	{
		Type::swap(other);
		argument.swap(other.argument);
	}
	Type& operator=(Declaration* declaration)
	{
		return Type::operator=(declaration);
	}
};

struct TemplateParameters : Types
{
	TypeIds defaults;
	TemplateParameters(const TreeAllocator<int>& allocator)
		: Types(allocator), defaults(allocator)
	{
	}
	void swap(TemplateParameters& other)
	{
		Types::swap(other);
		defaults.swap(other.defaults);
	}
	void push_front(const TemplateParameter& other)
	{
		Types::push_front(other);
		defaults.push_front(other.argument);
	}
};

const TemplateParameters TEMPLATEPARAMETERS_NULL = TemplateParameters(TREEALLOCATOR_NULL);


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
	TemplateParameters templateParams;
	TemplateArguments templateArguments; // non-empty if this is an explicit (or partial) specialization
	bool isTemplate;
	bool isSpecialization;

	Declaration(
		const TreeAllocator<int>& allocator,
		Scope* scope,
		Identifier& name,
		const TypeId& type,
		Scope* enclosed,
		DeclSpecifiers specifiers = DeclSpecifiers(),
		bool isTemplate = false,
		const TemplateParameters& templateParams = TEMPLATEPARAMETERS_NULL,
		bool isSpecialization = false,
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
		templateParams(templateParams),
		templateArguments(templateArguments),
		isTemplate(isTemplate),
		isSpecialization(isSpecialization)
	{
	}
	Declaration() :
		type(0, TREEALLOCATOR_NULL),
		valueDependent(TREEALLOCATOR_NULL),
		templateParams(TREEALLOCATOR_NULL),
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
		templateArguments.swap(other.templateArguments);
		std::swap(isTemplate, other.isTemplate);
		std::swap(isSpecialization, other.isSpecialization);
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

struct DeclarationInstance : DeclarationPtr
{
	Identifier* name;
	const DeclarationInstance* overloaded;
	const DeclarationInstance* redeclared;
	DeclarationInstance()
		: DeclarationPtr(0), name(0), overloaded(0), redeclared(0)
	{
	}
	explicit DeclarationInstance(Declaration* declaration)
		: DeclarationPtr(declaration), name(declaration != 0 ? &declaration->getName() : 0), overloaded(0), redeclared(0)
	{
		SYMBOLS_ASSERT(name != 0);
	}
	explicit DeclarationInstance(DeclarationPtr declaration)
		: DeclarationPtr(declaration), name(declaration != 0 ? &declaration->getName() : 0), overloaded(0), redeclared(0)
	{
		SYMBOLS_ASSERT(name != 0);
	}
};

inline cpp::terminal_identifier& getDeclarationId(const DeclarationInstance* declaration)
{
	return (*declaration)->getName();
}

inline bool isDecorated(const Identifier& id)
{
	return id.dec.p != 0;
}

inline Declaration& getDeclaration(const Identifier& id)
{
	SYMBOLS_ASSERT(isDecorated(id));
	return *(*id.dec.p);
}


struct Scope : public ScopeCounter
{
	ScopePtr parent;
	Identifier name;
	size_t enclosedScopeCount; // number of scopes directly enclosed by this scope
	typedef std::less<TokenValue> IdentifierLess;

	typedef std::multimap<TokenValue, DeclarationInstance, IdentifierLess, TreeAllocator<int> > Declarations2;

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

		const DeclarationInstance& insert(const DeclarationInstance& declaration)
		{
			SYMBOLS_ASSERT(declaration.name != 0);
			Declarations2::iterator result = Declarations2::insert(Declarations2::value_type(declaration.name->value, declaration));
			return (*result).second;
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
	size_t templateDepth;

	Scope(const TreeAllocator<int>& allocator, const Identifier& name, ScopeType type = SCOPETYPE_UNKNOWN)
		: parent(0), name(name), enclosedScopeCount(0), declarations(allocator), type(type), bases(allocator), usingDirectives(allocator), declarationList(allocator), deferred(allocator), deferredCount(0), templateDepth(0)

	{
	}
	~Scope()
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
		if((*i).second == declaration)
		{
			return i;
		}
	}

	return declarations.end();
}

inline void undeclare(const DeclarationInstance* p, LexerAllocator& allocator)
{
	Declaration* declaration = *p;
	SYMBOLS_ASSERT(declaration->getName().dec.p == 0 || declaration->getName().dec.p == p);
	declaration->getName().dec.p = 0;

	SYMBOLS_ASSERT(!declaration->scope->declarations.empty());
	SYMBOLS_ASSERT(!declaration->scope->declarationList.empty());

	SYMBOLS_ASSERT(declaration == declaration->scope->declarationList.back());
	declaration->scope->declarationList.pop_back(); // TODO: optimise

	Scope::Declarations::iterator i = findDeclaration(declaration->scope->declarations, declaration);
	SYMBOLS_ASSERT(i != declaration->scope->declarations.end());
	declaration->scope->declarations.erase(i);

}

inline BacktrackCallback makeUndeclareCallback(const DeclarationInstance* p)
{
	BacktrackCallback result = { BacktrackCallbackThunk<const DeclarationInstance, undeclare>::thunk, const_cast<DeclarationInstance*>(p) };
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

inline Scope* getEnclosingClass(Scope* scope)
{
	for(; scope != 0; scope = scope->parent)
	{
		if(scope->type == SCOPETYPE_CLASS
			&& *scope->name.value.c_str() != '$') // ignore anonymous union
		{
			return scope;
		}
	}
	return 0;
}


// ----------------------------------------------------------------------------
// template instantiation
inline const TypeId& getTemplateArgument(const Type& type, size_t index)
{
	TemplateArguments::const_iterator a = type.templateArguments.begin();
	TypeIds::const_iterator p = type.declaration->templateParams.defaults.begin();
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
			SYMBOLS_ASSERT(p != type.declaration->templateParams.defaults.end());
			if(index == 0)
			{
				SYMBOLS_ASSERT((*p).declaration != 0);
				return *p;
			}
			++p;
		}
	}
}

inline const Type& getInstantiatedSimpleType(const Type& type);

inline bool isTypedef(const Type& type)
{
	return type.declaration->specifiers.isTypedef // if this is a typedef..
		&& type.declaration->templateParameter == INDEX_INVALID; // .. and not a template-parameter
}

// resolve Template<T>::Type -> T
inline const Type& getInstantiatedTypeInternal(const Type& original, const Qualifying& qualifying)
{
	size_t index = original.declaration->templateParameter;
	if(index != INDEX_INVALID) // if the original type is a template-parameter.
	{
		// Find the template-specialisation it belongs to:
		for(const Type* i = qualifying.get(); i != 0; i = (*i).qualifying.get())
		{
			const Type& instantiated = getInstantiatedSimpleType(*i); // qualifying types should always be simple
			if(instantiated.declaration->enclosed == original.declaration->scope)
			{
				return getTemplateArgument(instantiated, index); // TODO: instantiate the argument?
			}
		}
	}

	return original;
}

template<typename Inner>
inline const Type& getInstantiatedTypeGeneric(const Type& type, Inner inner)
{
	if(type.declaration->specifiers.isTypedef)
	{ 
		// template<typename T> struct Template { typedef T Type; }; Template<int>::Type -> int
		const Type& instantiated = type.declaration->templateParameter == INDEX_INVALID // if type is not a template-parameter
			? getInstantiatedTypeInternal(inner(type.declaration->type), type.qualifying)
			: type;
		return instantiated;
	}
	return type;
}

inline const Type& getInstantiatedType(const Type& type)
{
	return getInstantiatedTypeGeneric(type, getInstantiatedType);
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
inline const Type& getInstantiatedSimpleType(const Type& type)
{
	SYMBOLS_ASSERT(isSimple(type));
	return getInstantiatedTypeGeneric(type, getInstantiatedSimpleType);
}

// ----------------------------------------------------------------------------

// meta types
extern Declaration gArithmetic;
extern Declaration gSpecial;
extern Declaration gClass;
extern Declaration gEnum;

extern Declaration gNamespace;
extern Declaration gCtor;
extern Declaration gEnumerator;
extern Declaration gUnknown;


inline bool isTemplate(const Scope& scope)
{
	return scope.templateDepth != 0;
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
	return declaration.scope != 0 && declaration.scope->type == SCOPETYPE_CLASS;
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

inline bool isSpecialization(const Declaration& declaration)
{
	return declaration.isSpecialization;
}

// ----------------------------------------------------------------------------



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
	const DeclarationInstance* filtered; // the declaration found by the name-lookup, using the filter

	LookupResult()
		: filtered(0)
	{
	}
	operator const DeclarationInstance*() const
	{
		return filtered;
	}

	// Combines the result of a subsequent lookup, returns true if lookup succeeded
	bool append(const LookupResult& other)
	{
		return append(other.filtered);
	}
	bool append(const DeclarationInstance* other)
	{
		filtered = other;
		return filtered != 0;
	}
};

struct DeclarationInstanceRef
{
	const DeclarationInstance* p;
	DeclarationInstanceRef()
		: p(0)
	{
	}
	DeclarationInstanceRef(const DeclarationInstance& p)
		: p(&p)
	{
		checkAllocated(this->p);
	}
	DeclarationInstanceRef(const LookupResult& result)
		: p(result.filtered)
	{
	}
	Declaration& operator*() const
	{
		checkAllocated(p);
		return p->operator*();
	}
	Declaration* operator->() const
	{
		checkAllocated(p);
		return p->operator->();
	}
	operator const DeclarationInstance&() const
	{
		checkAllocated(p);
		return *p;
	}
	operator Declaration*() const
	{
		checkAllocated(p);
		return p == 0 ? 0 : static_cast<Declaration*>(*p);
	}
};



inline bool isTypeName(const Declaration& declaration)
{
	return isType(declaration);
}

typedef LookupFilterDefault<isTypeName> IsTypeName;

inline bool isNamespaceName(const Declaration& declaration)
{
	return isNamespace(declaration);
}

typedef LookupFilterDefault<isNamespaceName> IsNamespaceName;


inline bool isTemplateName(const Declaration& declaration)
{
	// returns true if \p declaration is a template class, function or template-parameter
	return declaration.isTemplate && (isClass(declaration) || isFunction(declaration) || isTypedef(declaration));
}

inline bool isNestedName(const Declaration& declaration)
{
	return isTypeName(declaration)
		|| isNamespaceName(declaration);
}

typedef LookupFilterDefault<isNestedName> IsNestedName;


inline bool isNonMemberName(const Declaration& declaration)
{
	return isNonMember(declaration);
}

typedef LookupFilterDefault<isNonMemberName> IsNonMemberName;


// ----------------------------------------------------------------------------
// unique types
// Representation of a declarator, with type-elements linked in 'normal' order.
// e.g. int(*)[] == pointer to array of == DeclaratorPointer -> DeclaratorArray
// Note that this is the reverse of the order that the declarator is parsed in.
// This means a given unique type sub-sequence need only be stored once.
// This allows fast comparison of types and simplifies printing of declarators.

struct TypeElement
{
	UniqueType next;

	TypeElement()
	{
	}
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
	SYMBOLS_ASSERT(type.getBits() != 0);
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
	bool isDependent() const
	{
		return typeid(*value) == typeid(TypeElementGeneric<DeclaratorDependent>);
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
	bool isFunctionPointer() const
	{
		return isPointer()
			&& UniqueTypeWrapper(value->next).isFunction();
	}
};

inline bool operator==(UniqueTypeWrapper l, UniqueTypeWrapper r)
{
	return l.value == r.value;
}

inline bool operator!=(UniqueTypeWrapper l, UniqueTypeWrapper r)
{
	return !operator==(l, r);
}

inline bool operator<(UniqueTypeWrapper l, UniqueTypeWrapper r)
{
	return l.value < r.value;
}

// ----------------------------------------------------------------------------
struct Parameter
{
	DeclarationPtr declaration;
	cpp::default_argument* argument;
	Parameter(Declaration* declaration, cpp::default_argument* argument)
		: declaration(declaration), argument(argument)
	{
	}
};

typedef std::vector<Parameter> Parameters;

// ----------------------------------------------------------------------------

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
	bool evaluated;
	TypeInstance(Declaration* declaration, const TypeInstance* enclosing)
		: declaration(declaration), enclosing(enclosing), evaluated(false)
	{
		SYMBOLS_ASSERT(enclosing == 0 || isClass(*enclosing->declaration));
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
	return static_cast<const TypeElementGeneric<DeclaratorObject>*>(type.getPointer())->value.type;
}

// TODO: consider template-template-parameter
struct DeclaratorDependent
{
	DeclarationPtr type; // the declaration of the template parameter
	DeclaratorDependent(Declaration* type)
		: type(type)
	{
	}
};

inline bool operator==(const DeclaratorDependent& left, const DeclaratorDependent& right)
{
	return left.type->scope->templateDepth == right.type->scope->templateDepth
		&& left.type->templateParameter == right.type->templateParameter;
}

inline bool operator<(const DeclaratorDependent& left, const DeclaratorDependent& right)
{
	return left.type->scope->templateDepth != right.type->scope->templateDepth
		? left.type->scope->templateDepth < right.type->scope->templateDepth
		: left.type->templateParameter < right.type->templateParameter;
}


struct DeclaratorPointer
{
	CvQualifiers qualifiers;
	DeclaratorPointer()
	{
	}
	explicit DeclaratorPointer(CvQualifiers qualifiers)
		: qualifiers(qualifiers)
	{
	}
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
	Type type;
	CvQualifiers qualifiers;
	const TypeInstance* instance;
	DeclaratorMemberPointer(const Type& type, CvQualifiers qualifiers)
		: type(type), qualifiers(qualifiers), instance(0)
	{
	}
};

inline const TypeInstance& getMemberPointerClass(UniqueType type)
{
	SYMBOLS_ASSERT(typeid(*type) == typeid(TypeElementGeneric<DeclaratorMemberPointer>));
	return *(static_cast<const TypeElementGeneric<DeclaratorMemberPointer>*>(type.getPointer())->value.instance);
}

inline bool operator==(const DeclaratorMemberPointer& left, const DeclaratorMemberPointer& right)
{
	return left.instance == right.instance;
}

inline bool operator<(const DeclaratorMemberPointer& left, const DeclaratorMemberPointer& right)
{
	return left.instance < right.instance;
}

struct DeclaratorArray
{
	std::size_t size;
	DeclaratorArray(std::size_t size)
		: size(size)
	{
	}
};

inline bool operator==(const DeclaratorArray& left, const DeclaratorArray& right)
{
	return left.size == right.size;
}

inline bool operator<(const DeclaratorArray& left, const DeclaratorArray& right)
{
	return left.size < right.size;
}

typedef std::vector<UniqueTypeWrapper> ParameterTypes;
typedef std::vector<UniqueTypeWrapper> ArgumentTypes;

struct DeclaratorFunction
{
	Parameters parameters;
	CvQualifiers qualifiers;
	ParameterTypes parameterTypes;
	DeclaratorFunction(const Parameters& parameters, CvQualifiers qualifiers)
		: parameters(parameters), qualifiers(qualifiers)
	{
	}
};

inline bool operator==(const DeclaratorFunction& left, const DeclaratorFunction& right)
{
	return left.parameterTypes == right.parameterTypes;
}

inline bool operator<(const DeclaratorFunction& left, const DeclaratorFunction& right)
{
	return left.parameterTypes < right.parameterTypes;
}

inline const Parameters& getParameters(UniqueType type)
{
	SYMBOLS_ASSERT(typeid(*type) == typeid(TypeElementGeneric<DeclaratorFunction>));
	return static_cast<const TypeElementGeneric<DeclaratorFunction>*>(type.getPointer())->value.parameters;
}

inline const ParameterTypes& getParameterTypes(UniqueType type)
{
	SYMBOLS_ASSERT(typeid(*type) == typeid(TypeElementGeneric<DeclaratorFunction>));
	return static_cast<const TypeElementGeneric<DeclaratorFunction>*>(type.getPointer())->value.parameterTypes;
}



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



typedef std::vector<const Declaration*> DeclarationHistory;

struct GetTypeHistory
{
	DeclarationHistory& history;
	GetTypeHistory(DeclarationHistory& history)
		: history(history)
	{
	}
	const Type& operator()(const Type& type) const
	{
		return apply(type, history);
	}
	static const Type& apply(const Type& type, DeclarationHistory& history)
	{
		if(!isSimple(type))
		{
			history.push_back(type.declaration);
		}
		return getInstantiatedTypeGeneric(type, GetTypeHistory(history));
	}
	static const Type& apply(const Declaration& declaration, DeclarationHistory& history)
	{
		if(!isSimple(declaration))
		{
			history.push_back(&declaration);
		}
		return apply(declaration.type, history);
	}
};

inline Declaration* findPrimaryTemplate(Declaration* declaration)
{
	SYMBOLS_ASSERT(declaration->isTemplate);
	for(;declaration != 0; declaration = declaration->overloaded)
	{
		if(!isSpecialization(*declaration))
		{
			SYMBOLS_ASSERT(declaration->isTemplate);
			return declaration;
		}
	}
	SYMBOLS_ASSERT(false); // primary template not declared!
	return 0;
}

inline const DeclarationInstance& findLastDeclaration(const DeclarationInstance& instance, Declaration* declaration)
{
	for(const DeclarationInstance* p = &instance; p != 0; p = p->overloaded)
	{
		if(*p == declaration)
		{
			return *p;
		}
	}
	throw SymbolsError();
}


inline const DeclarationInstance& findPrimaryTemplateLastDeclaration(const DeclarationInstance& instance)
{
	return findLastDeclaration(instance, findPrimaryTemplate(instance));
}

inline UniqueTypeWrapper makeUniqueType(const TypeId& type);
inline UniqueTypeWrapper makeUniqueType(const TypeId& type, const TypeInstance* enclosing, bool allowDependent, std::size_t depth);
inline UniqueTypeWrapper makeUniqueType(const Type& type, const TypeInstance* enclosing, bool allowDependent, std::size_t depth);

struct TypeError
{
	TypeError()
	{
	}
};

inline UniqueTypeWrapper makeUniqueType(const TypeId& type, const TypeInstance* enclosing, bool allowDependent = false)
{
	try
	{
		return makeUniqueType(type, enclosing, allowDependent, 0);
	}
	catch(TypeError)
	{
		std::cout << "makeUniqueType failed!" << std::endl;
		extern ObjectTypeId gBaseClass;
		return gBaseClass;
	}
}

inline UniqueTypeWrapper makeUniqueType(const Type& type, const TypeInstance* enclosing, bool allowDependent = false)
{
	try
	{
		return makeUniqueType(type, enclosing, allowDependent, 0);
	}
	catch(TypeError)
	{
		std::cout << "makeUniqueType failed!" << std::endl;
		extern ObjectTypeId gBaseClass;
		return gBaseClass;
	}
}


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
#if 0 // TODO: fix
	SYMBOLS_ASSERT(enclosingTemplate != 0);
	SYMBOLS_ASSERT(type->enclosingTemplate == 0);
	enclosingTemplate->deferred.push_back(makeDeferredLookupCallback(type));
	type->specialization = enclosingTemplate->deferredCount++;
	type->enclosingTemplate = enclosingTemplate;
#endif
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
inline bool isDependentOld(const Type& type, Scope* enclosing)
{
	return type.dependent != DeclarationPtr(0)
		&& findScope(enclosing, type.dependent->scope);
}

inline const TypeInstance* makeUniqueEnclosing(const Qualifying& qualifying, const TypeInstance* enclosing, bool allowDependent, std::size_t depth)
{
	if(!qualifying.empty())
	{
		UniqueTypeWrapper tmp = makeUniqueType(*qualifying.get(), enclosing, allowDependent, depth);
		return allowDependent && tmp.isDependent() ? 0 : &getObjectType(tmp.value);
	}
	return enclosing;
}

inline const TypeInstance* makeUniqueEnclosing(const Qualifying& qualifying, const TypeInstance* enclosing, bool allowDependent = false)
{
	try
	{
		return makeUniqueEnclosing(qualifying, enclosing, allowDependent, 0);
	}
	catch(TypeError)
	{
		std::cout << "makeUniqueEnclosing failed!" << std::endl;
		return 0;
	}
}

inline bool matchTemplateSpecialization(const Declaration& declaration, const TemplateArgumentsInstance& arguments, const TypeInstance* enclosing)
{
	// TODO: check that all non-defaulted arguments are specified!
	TemplateArgumentsInstance::const_iterator a = arguments.begin();
	for(TemplateArguments::const_iterator i = declaration.templateArguments.begin(); i != declaration.templateArguments.end(); ++i)
	{
		SYMBOLS_ASSERT(a != arguments.end()); // a template-specialization must have no more arguments than the template parameters
		UniqueTypeWrapper type;
		SYMBOLS_ASSERT((*i).type.declaration != 0);
		extern Declaration gNonType;
		if((*i).type.declaration != &gNonType) // ignore non-type arguments
		{
			type = makeUniqueType((*i).type, enclosing, true); // a partial-specialization may have dependent template-arguments: template<class T> class C<T*>
		}
		if(type != *a)
		{
			return false;
		}
	}
	return true;
}

inline bool isDependentOld(const TemplateArguments& arguments, const DependentContext& context);

inline Declaration* findTemplateSpecialization(Declaration* declaration, const TemplateArgumentsInstance& arguments, const TypeInstance* enclosing)
{
	for(; declaration != 0; declaration = declaration->overloaded)
	{
		if(!isSpecialization(*declaration))
		{
			continue;
		}

		if(matchTemplateSpecialization(*declaration, arguments, enclosing))
		{
			return declaration;
		}
	}
	return 0;
}

inline void evaluateBases(const TypeInstance& enclosing)
{
	if(!enclosing.evaluated)
	{
		TypeInstance& instance = const_cast<TypeInstance&>(enclosing);
		instance.evaluated = true; // prevent recursion
		Types& bases = enclosing.declaration->enclosed->bases;
		UniqueBases uniqueBases;
		uniqueBases.reserve(std::distance(bases.begin(), bases.end()));
		for(Types::const_iterator i = bases.begin(); i != bases.end(); ++i)
		{
			uniqueBases.push_back(&getObjectType(makeUniqueType(*i, &enclosing).value));
		}
		instance.bases.swap(uniqueBases); // prevent searching a partially evaluated set of base classes inside 'makeUniqueType'
	}
}

inline const TypeInstance* findEnclosingType(const TypeInstance& enclosing, Scope* enclosingType)
{
	SYMBOLS_ASSERT(enclosingType != 0);
	if(enclosing.declaration->enclosed == enclosingType)
	{
		return &enclosing;
	}

	evaluateBases(enclosing);

	for(UniqueBases::const_iterator i = enclosing.bases.begin(); i != enclosing.bases.end(); ++i)
	{
		const TypeInstance* result = findEnclosingType(*(*i), enclosingType);
		if(result != 0)
		{
			return result;
		}
	}
	return 0;
}

inline const TypeInstance* findEnclosingType(const TypeInstance* enclosing, Scope* enclosingType)
{
	SYMBOLS_ASSERT(enclosingType != 0);
	for(const TypeInstance* i = enclosing; i != 0; i = (*i).enclosing)
	{
		const TypeInstance* result = findEnclosingType(*i, enclosingType);
		if(result != 0)
		{
			return result;
		}
	}
	return 0;
}

inline const DeclarationInstance* findDeclaration(Scope::Declarations& declarations, const Identifier& id, LookupFilter filter, bool isBase);
inline LookupResult findDeclaration(const TypeInstance& instance, const Identifier& id, LookupFilter filter, bool isBase);

inline LookupResult findDeclaration(const UniqueBases& bases, const Identifier& id, LookupFilter filter)
{
	LookupResult result;
	for(UniqueBases::const_iterator i = bases.begin(); i != bases.end(); ++i)
	{
		const TypeInstance& base = *(*i);
		SYMBOLS_ASSERT(base.declaration->enclosed != 0); // TODO: non-fatal error: incomplete type
		SYMBOLS_ASSERT(base.declaration->enclosed->usingDirectives.empty()); // namespace.udir: A using-directive shall not appear in class scope, but may appear in namespace scope or in block scope.
		if(result.append(findDeclaration(base, id, filter, true)))
		{
			return result;
		}
	}
	return result;
}

inline LookupResult findDeclaration(const TypeInstance& instance, const Identifier& id, LookupFilter filter, bool isBase = false)
{
	SYMBOLS_ASSERT(instance.declaration->enclosed != 0);
	LookupResult result;
	if(result.append(findDeclaration(instance.declaration->enclosed->declarations, id, filter, isBase)))
	{
		return result;
	}
	evaluateBases(instance);
	if(result.append(findDeclaration(instance.bases, id, filter)))
	{
		return result;
	}
	return result;
}

inline UniqueTypeWrapper makeUniqueObjectType(const TypeInstance& type)
{
	return UniqueTypeWrapper(pushUniqueType(gUniqueTypes, UNIQUETYPE_NULL, DeclaratorObject(type)));
}

// unqualified object name: int, Object,
// qualified object name: Qualifying::Object
// unqualified typedef: Typedef, TemplateParam
// qualified typedef: Qualifying::Type
// /p type
// /p enclosing The enclosing template, required when uniquing a template-argument: e.g. Enclosing<int>::Type
inline UniqueTypeWrapper makeUniqueType(const Type& type, const TypeInstance* enclosingType, bool allowDependent, std::size_t depth)
{
	if(depth++ == 256)
	{
		std::cout << "makeUniqueType reached maximum recursion depth!" << std::endl;
		throw TypeError();
	}
	// the type in which template-arguments are looked up: returns qualifying type if specified, else returns enclosingType
	const TypeInstance* enclosing = makeUniqueEnclosing(type.qualifying, enclosingType, allowDependent, depth);
	Declaration* declaration = type.declaration;
	extern Declaration gDependentType;
	extern Declaration gDependentTemplate;
	extern Declaration gDependentNested;
	if(declaration == &gDependentType
		|| declaration == &gDependentTemplate
		|| declaration == &gDependentNested) // this is a type-name with a dependent nested-name-specifier
	{
		SYMBOLS_ASSERT(!type.qualifying.empty()); // the type-name must be qualified
		SYMBOLS_ASSERT(type.id != IdentifierPtr(0));
		const DeclarationInstance* instance = 0;
		if(!allowDependent // the qualifying/enclosing type is not dependent
			&& enclosing->declaration->enclosed != 0) // the qualifying/enclosing type is complete
		{
			instance = findDeclaration(*enclosing, *type.id, declaration == &gDependentNested ? LookupFilter(IsNestedName()) : LookupFilter(IsAny()));
		}
		// SYMBOLS_ASSERT(declaration != 0); // TODO: assert
		if(instance == 0)
		{
			if(!allowDependent)
			{
				std::cout << "lookup failed!" << std::endl;
			}
			extern ObjectTypeId gBaseClass;
			return gBaseClass;
		}
		declaration = *instance;
	}
	if(type.specialization != INDEX_INVALID)
	{
		SYMBOLS_ASSERT(enclosing != 0);
		const TypeInstance* enclosingType = findEnclosingType(enclosing, type.enclosingTemplate);
		if(enclosingType != 0)
		{
			// lazily evaluate the specializations for this type
			{
				TypeInstance& instance = *const_cast<TypeInstance*>(enclosingType);
				SYMBOLS_ASSERT(enclosingType->declaration->isTemplate) // TODO: isImplicitTemplateId
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
			SYMBOLS_ASSERT(type.specialization < (*enclosingType).specializations.size());
			UniqueTypeWrapper result = (*enclosingType).specializations[type.specialization];
			// TODO: SYMBOLS_ASSERT(result.value != UNIQUETYPE_NULL);
			return result;
		}
		throw SymbolsError();
		return gUniqueTypeNull; // error?
	}
	size_t index = declaration->templateParameter; // TODO: template-template-parameter
	if(index != INDEX_INVALID)
	{
		SYMBOLS_ASSERT(type.qualifying.empty());
		// Find the template-specialisation it belongs to:
		const TypeInstance* enclosingType = findEnclosingType(enclosing, declaration->scope);
		if(enclosingType != 0)
		{
			SYMBOLS_ASSERT(index < enclosingType->templateArguments.size());
			UniqueTypeWrapper result = enclosingType->templateArguments[index];
			// TODO: SYMBOLS_ASSERT(result.value != UNIQUETYPE_NULL); // fails for non-type template-argument
			return result;
		}

		if(allowDependent
			/*&& enclosing == 0*/)
		{
			return UniqueTypeWrapper(pushUniqueType(gUniqueTypes, UNIQUETYPE_NULL, DeclaratorDependent(declaration)));
		}

		throw SymbolsError();
		return gUniqueTypeNull; // error: can't find template specialisation for this template parameter
	}
	if(declaration->specifiers.isTypedef)
	{
		return makeUniqueType(declaration->type, enclosing, allowDependent, depth);
	}
	TypeInstance tmp(declaration, declaration->scope == 0 || declaration->scope->type != SCOPETYPE_CLASS ? 0 : findEnclosingType(enclosing, declaration->scope));
	SYMBOLS_ASSERT(declaration->type.declaration != &gArithmetic || tmp.enclosing == 0); // arithmetic types should not have an enclosing template!
	if(declaration->isTemplate)
	{
		tmp.declaration = findPrimaryTemplate(declaration); // TODO: look up explicit specialization

		// 14.6.1: when the name of a template is used without arguments, substitute the parameters (in case of an explicit/partial-specialization, substitute the arguments
		const TypeIds& defaults = tmp.declaration->templateParams.defaults;
		SYMBOLS_ASSERT(!defaults.empty());
		if(type.isImplicitTemplateId // TODO: check that 'declaration' refers to the enclosing template
			&& !isSpecialization(*type.declaration))
		{
			for(Types::const_iterator i = declaration->templateParams.begin(); i != declaration->templateParams.end(); ++i)
			{
				const Type& argument = (*i);
				UniqueTypeWrapper result;
				extern Declaration gParam;
				if(argument.declaration->type.declaration == &gParam) // ignore non-type arguments
				{
					result = makeUniqueType(argument, enclosingType, allowDependent, depth);
					SYMBOLS_ASSERT(result.value != UNIQUETYPE_NULL);
				}
				tmp.templateArguments.push_back(result);
			}
		}
		else
		{
			const TemplateArguments& arguments = type.isImplicitTemplateId
				? type.declaration->templateArguments
				: type.templateArguments;
			TemplateArguments::const_iterator a = arguments.begin();
			for(TypeIds::const_iterator i = defaults.begin(); i != defaults.end(); ++i)
			{
				bool isTemplateParamDefault = a == arguments.end();
				const TypeId& argument = isTemplateParamDefault ? (*i) : (*a++).type;
				SYMBOLS_ASSERT(argument.declaration != 0); // TODO: non-fatal error: not enough template arguments!
				UniqueTypeWrapper result;
				extern Declaration gNonType;
				if(argument.declaration != &gNonType) // ignore non-type arguments
				{
					result = makeUniqueType(argument, isTemplateParamDefault ? &tmp : enclosingType, allowDependent, depth); // resolve dependent template-parameter-defaults in context of template class
					SYMBOLS_ASSERT(result.value != UNIQUETYPE_NULL);
				}
				tmp.templateArguments.push_back(result);
			}
		}
		SYMBOLS_ASSERT(!tmp.templateArguments.empty());

		Declaration* specialization = findTemplateSpecialization(declaration, tmp.templateArguments, enclosing);
		if(specialization != 0)
		{
			tmp.declaration = specialization;
		}
	}
	SYMBOLS_ASSERT(tmp.bases.empty());
	SYMBOLS_ASSERT(tmp.specializations.empty());
	return makeUniqueObjectType(tmp);
}

inline UniqueTypeWrapper makeUniqueType(const Type& type)
{
	return makeUniqueType(type, 0);
}


struct TypeSequenceMakeUnique : TypeElementVisitor
{
	UniqueType& type;
	const TypeInstance* enclosing;
	bool allowDependent;
	TypeSequenceMakeUnique(UniqueType& type, const TypeInstance* enclosing, bool allowDependent)
		: type(type), enclosing(enclosing), allowDependent(allowDependent)
	{
	}
	void visit(const DeclaratorDependent& element)
	{
		throw SymbolsError(); // error!
	}
	void visit(const DeclaratorObject& element)
	{
		pushUniqueType(type, element);
	}
	void visit(const DeclaratorPointer& element)
	{
		pushUniqueType(type, element);
		type.setQualifiers(element.qualifiers);
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
		DeclaratorMemberPointer result(element);
		UniqueTypeWrapper tmp = makeUniqueType(element.type, enclosing, allowDependent);
		result.instance = allowDependent && tmp.isDependent() ? 0 : &getObjectType(tmp.value); // TODO: should be non-null even if dependent
		pushUniqueType(type, result);
		type.setQualifiers(element.qualifiers);
	}
	void visit(const DeclaratorFunction& element)
	{
		DeclaratorFunction result(element);
		for(Parameters::const_iterator i = element.parameters.begin(); i != element.parameters.end(); ++i)
		{
			result.parameterTypes.push_back(makeUniqueType((*i).declaration->type, enclosing, allowDependent));
		}
		pushUniqueType(type, result);
		type.setQualifiers(element.qualifiers);
	}
};

inline UniqueTypeWrapper makeUniqueType(const TypeId& type, const TypeInstance* enclosing, bool allowDependent, std::size_t depth)
{
	UniqueTypeWrapper result = makeUniqueType(*static_cast<const Type*>(&type), enclosing, allowDependent, depth);
	result.value.setQualifiers(type.qualifiers);
	TypeSequenceMakeUnique visitor(result.value, enclosing, allowDependent);
	type.typeSequence.accept(visitor);
	return result;
}

inline UniqueTypeWrapper makeUniqueType(const TypeId& type)
{
	return makeUniqueType(type, 0);
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
extern const DeclarationInstance gUndeclaredInstance;

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


#define TYPE_NAMESPACE TypeId(&gNamespace, TREEALLOCATOR_NULL)
#define TYPE_CTOR TypeId(&gCtor, TREEALLOCATOR_NULL)
#define TYPE_ENUMERATOR TypeId(&gEnumerator, TREEALLOCATOR_NULL)
#define TYPE_UNKNOWN TypeId(&gUnknown, TREEALLOCATOR_NULL)


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

inline bool isVoidParameter(const TypeId& type)
{
	return type.declaration == &gVoidDeclaration
		&& type.typeSequence.empty();
}


struct StringLiteralTypeId : ObjectTypeId
{
	StringLiteralTypeId(Declaration* declaration, const TreeAllocator<int>& allocator)
		: ObjectTypeId(declaration, allocator)
	{
		value = pushBuiltInType(value, DeclaratorArray(0));
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
extern const DeclarationInstance gDependentTypeInstance;
extern Declaration gDependentObject;
extern const DeclarationInstance gDependentObjectInstance;
extern Declaration gDependentTemplate;
extern const DeclarationInstance gDependentTemplateInstance;
extern Declaration gDependentNested;
extern const DeclarationInstance gDependentNestedInstance;

extern Declaration gParam;
extern Declaration gNonType;

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

inline const TypeId& getUnderlyingType(const TypeId& type)
{
	if(type.declaration->specifiers.isTypedef)
	{
		return getUnderlyingType(type.declaration->type);
	}
	return type;
}

inline bool isEqual(const TypeId& l, const TypeId& r)
{
#if 1
	UniqueTypeWrapper left = makeUniqueType(l, 0, true);
	UniqueTypeWrapper right = makeUniqueType(r, 0, true);
	return left == right;
#endif
	// TODO: compare typeSequence
	if(getUnderlyingType(l).declaration == getUnderlyingType(r).declaration)
	{
		return true;
	}
	if(l.declaration->templateParameter != INDEX_INVALID
		&& l.declaration->templateParameter != INDEX_INVALID
		&& l.declaration->templateParameter == r.declaration->templateParameter)
	{
		return true;
	}
	return false;
}

inline bool isEqual(const TemplateArgument& l, const TemplateArgument& r)
{
	return isEqual(l.type, r.type);
}

inline bool matchTemplateSpecialization(const Declaration& declaration, const TemplateArguments& arguments)
{
	// TODO: check that all arguments are specified!
	TemplateArguments::const_iterator a = arguments.begin();
	for(TemplateArguments::const_iterator i = declaration.templateArguments.begin(); i != declaration.templateArguments.end(); ++i)
	{
		SYMBOLS_ASSERT(a != arguments.end()); // a template-specialization must have no more arguments than the template parameters

		if(!isEqual(*i, *a))
		{
			return false;
		}
		++a;
	}
	SYMBOLS_ASSERT(a == arguments.end());
	return true;
}

inline Declaration* findTemplateSpecialization(Declaration* declaration, const TemplateArguments& arguments)
{
	for(; declaration != 0; declaration = declaration->overloaded)
	{
		if(!isSpecialization(*declaration))
		{
			continue;
		}

		if(matchTemplateSpecialization(*declaration, arguments))
		{
			return declaration;
		}
	}
	return 0;
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

// [over.ics.scs]
enum ScsRank
{
	SCSRANK_IDENTITY, // no conversion
	SCSRANK_EXACT,
	SCSRANK_PROMOTION,
	SCSRANK_CONVERSION,
	SCSRANK_INVALID,
};

struct StandardConversionSequence
{
	ScsRank rank;
	CvQualifiers adjustment; // TODO: cv-qualification signature for multi-level pointer type
	StandardConversionSequence(ScsRank rank, CvQualifiers adjustment)
		: rank(rank), adjustment(adjustment)
	{
	}
};

const StandardConversionSequence STANDARDCONVERSIONSEQUENCE_INVALID = StandardConversionSequence(SCSRANK_INVALID, CvQualifiers());

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

inline UniqueTypeWrapper applyLvalueTransformation(UniqueTypeWrapper to, UniqueTypeWrapper from)
{
	if(to.isPointer()
		&& from.isArray())
	{
		return UniqueTypeWrapper(pushUniqueType(gUniqueTypes, getInner(from.value), DeclaratorPointer())); // T[] -> T*
	}
	if(to.isFunctionPointer()
		&& from.isFunction())
	{
		return UniqueTypeWrapper(pushUniqueType(gUniqueTypes, from.value, DeclaratorPointer())); // T() -> T(*)()
	}
	return from;
}

inline CvQualifiers makeQualificationAdjustment(UniqueTypeId to, UniqueTypeId from)
{
	return CvQualifiers(to.value.getQualifiers().isConst > from.value.getQualifiers().isConst,
		to.value.getQualifiers().isVolatile > from.value.getQualifiers().isVolatile);
}

inline bool isGreaterCvQualification(UniqueTypeId to, UniqueTypeId from)
{
	return to.value.getQualifiers().isConst > from.value.getQualifiers().isConst
		|| to.value.getQualifiers().isVolatile > from.value.getQualifiers().isVolatile;
}

inline bool isEqualCvQualification(UniqueTypeId to, UniqueTypeId from)
{
	return to.value.getQualifiers() == from.value.getQualifiers();
}



inline StandardConversionSequence makeScsPromotion(UniqueTypeId to, UniqueTypeId from)
{
	if(isArithmetic(from) && from.isSimple()
		&& isArithmetic(to) && to.isSimple()
		&& (isEqual(from, gFloat) && isEqual(to, gDouble))
			|| (isEqual(promoteToIntegralType(from), to)))
	{
		return StandardConversionSequence(SCSRANK_PROMOTION, CvQualifiers());
	}
	return STANDARDCONVERSIONSEQUENCE_INVALID;
}

inline StandardConversionSequence makeScsConversion(UniqueTypeId to, UniqueTypeId from, bool isNullPointerConstant = false) // TODO: detect null pointer constant
{
	SYMBOLS_ASSERT(to.value.getQualifiers() == CvQualifiers());
	SYMBOLS_ASSERT(from.value.getQualifiers() == CvQualifiers());
	if((isArithmetic(from) || isEnumeration(from)) && from.isSimple()
		&& isArithmetic(to) && to.isSimple())
	{
		// can convert from enumeration to integer/floating/bool, but not in reverse
		return StandardConversionSequence(SCSRANK_CONVERSION, CvQualifiers());
	}
	if((to.isPointer() || to.isMemberPointer())
		&& isIntegral(from)
		&& isNullPointerConstant)
	{
		return StandardConversionSequence(SCSRANK_CONVERSION, CvQualifiers()); // 0 -> T*
	}
	if(to.isSimplePointer()
		&& from.isSimplePointer()
		&& getInner(to.value) == gVoid.value)
	{
		to = UniqueTypeWrapper(getInner(to.value));
		from = UniqueTypeWrapper(getInner(from.value));
		return isEqualCvQualification(to, from) || isGreaterCvQualification(to, from)
			? StandardConversionSequence(SCSRANK_CONVERSION, makeQualificationAdjustment(to, from))
			: STANDARDCONVERSIONSEQUENCE_INVALID; // T* -> void*
	}
	if(to.isSimplePointer()
		&& from.isSimplePointer()
		&& isBaseOf(getObjectType(getInner(to.value)), getObjectType(getInner(from.value))))
	{
		to = UniqueTypeWrapper(getInner(to.value));
		from = UniqueTypeWrapper(getInner(from.value));
		return isEqualCvQualification(to, from) || isGreaterCvQualification(to, from)
			? StandardConversionSequence(SCSRANK_CONVERSION, makeQualificationAdjustment(to, from))
			: STANDARDCONVERSIONSEQUENCE_INVALID; // D* -> B*
	}
	if(to.isMemberPointer()
		&& from.isMemberPointer()
		&& isBaseOf(getMemberPointerClass(to.value), getMemberPointerClass(from.value)))
	{
		return StandardConversionSequence(SCSRANK_CONVERSION, CvQualifiers()); // D::* -> B::*
	}
	if(isEqual(to, gBool)
		&& (from.isPointer() || from.isMemberPointer()))
	{
		return StandardConversionSequence(SCSRANK_CONVERSION, CvQualifiers()); // T* -> bool, T::* -> bool
	}
	if(to.isSimple()
		&& from.isSimple()
		&& isBaseOf(getObjectType(to.value), getObjectType(from.value)))
	{
		return StandardConversionSequence(SCSRANK_CONVERSION, CvQualifiers()); // D -> B
	}
	return STANDARDCONVERSIONSEQUENCE_INVALID;
}

// exact
// T& <- T
// T& <- T&
// const T& <- T
// const T& <- T&
// derived to base conversion
// B& <- D
// B& <- D&
// const B& <- D
// const B& <- D&
// invalid
// T& <- const T
// T& <- const T&
// B& <- const D
// B& <- const D&

// [conv]
// exact
// T <- T
// T* <- T[]
// T(*)() <- T()
// const T* <- T*
// T C::const*  <- T C::*
// multi-level pointer / member-pointer
// derived to base conversion
// B <- D

inline StandardConversionSequence makeScsExactMatch(UniqueTypeWrapper to, UniqueTypeWrapper from)
{
	for(;;)
	{
		if(to.value.getPointer() == from.value.getPointer())
		{
			return isEqualCvQualification(to, from) || isGreaterCvQualification(to, from)
				? StandardConversionSequence(SCSRANK_EXACT, makeQualificationAdjustment(to, from))
				: STANDARDCONVERSIONSEQUENCE_INVALID;
		}
		if(to.isPointer()
			&& from.isPointer())
		{
		}
		else if(to.isMemberPointer()
			&& from.isMemberPointer()
			&& &getMemberPointerClass(to.value) == &getMemberPointerClass(from.value))
		{
		}
		else
		{
			break;
		}
		to = UniqueTypeWrapper(getInner(to.value));
		from = UniqueTypeWrapper(getInner(from.value));
	}
	return STANDARDCONVERSIONSEQUENCE_INVALID;
}

// 13.3.3.1 [over.best.ics]
inline StandardConversionSequence makeStandardConversionSequence(UniqueTypeWrapper to, UniqueTypeWrapper from, bool isNullPointerConstant = false, bool isLvalue = false)
{
	// TODO: user-defined conversion
	if(from.value == UNIQUETYPE_NULL)
	{
		return STANDARDCONVERSIONSEQUENCE_INVALID; // TODO: assert
	}
	// 13.3.3.1.4 [over.ics.ref]: reference binding
	if(to.isReference()) 
	{
		to = UniqueTypeWrapper(getInner(to.value));
		if(from.isReference())
		{
			isLvalue = true;
			from = UniqueTypeWrapper(getInner(from.value)); // TODO: removal of reference won't be detected later
		}
		// 8.5.3 [dcl.init.ref]
		// does it directly bind?
		if(isLvalue
			&& (isEqualCvQualification(to, from)
				|| isGreaterCvQualification(to, from))) // TODO: track 'added qualification' if qualification is greater
		{
			if(to.value.getPointer() == from.value.getPointer())
			{
				return StandardConversionSequence(SCSRANK_EXACT, makeQualificationAdjustment(to, from));
			}
			if(to.isSimple()
				&& from.isSimple()
				&& isBaseOf(getObjectType(to.value), getObjectType(from.value)))
			{
				return StandardConversionSequence(SCSRANK_CONVERSION, makeQualificationAdjustment(to, from));
			}
		}
		// if not bound directly, a standard conversion is required (which produces an rvalue)
		if(!to.value.getQualifiers().isConst
			|| to.value.getQualifiers().isVolatile) // 8.5.3-5: otherwise, the reference shall be to a non-volatile const type
		{
			// can't bind rvalue to a non-const reference
			return STANDARDCONVERSIONSEQUENCE_INVALID;
		}
	}

	if(!to.isReference()
		&& from.isReference())
	{
		from = UniqueTypeWrapper(getInner(from.value)); // T& -> T
	}

	// ignore top level cv-qualifiers
	to.value.setQualifiers(CvQualifiers());
	from.value.setQualifiers(CvQualifiers());

	UniqueTypeWrapper tmp = from;
	from = applyLvalueTransformation(to, from);

	if(tmp == from // no l-value transformation
		&& to == from) // no other conversions required
	{
		return StandardConversionSequence(SCSRANK_IDENTITY, CvQualifiers());
	}

	{
		StandardConversionSequence result = makeScsExactMatch(to, from);
		if(result.rank != SCSRANK_INVALID)
		{
			return result;
		}
	}
	{
		StandardConversionSequence result = makeScsPromotion(to, from);
		if(result.rank != SCSRANK_INVALID)
		{
			return result;
		}
	}
	{
		StandardConversionSequence result = makeScsConversion(to, from, isNullPointerConstant); // TODO: ordering of conversions by inheritance distance
		if(result.rank != SCSRANK_INVALID)
		{
			return result;
		}
	}

	// TODO: user-defined
	// TODO: ellipsis
	return STANDARDCONVERSIONSEQUENCE_INVALID;
}

inline IcsRank getIcsRank(UniqueTypeWrapper to, UniqueTypeWrapper from, bool isNullPointerConstant = false, bool isLvalue = false)
{
	StandardConversionSequence sequence = makeStandardConversionSequence(to, from, isNullPointerConstant, isLvalue);
	switch(sequence.rank)
	{
	case SCSRANK_IDENTITY:
	case SCSRANK_EXACT: return ICSRANK_STANDARDEXACT;
	case SCSRANK_PROMOTION: return ICSRANK_STANDARDPROMOTION;
	case SCSRANK_CONVERSION: return ICSRANK_STANDARDCONVERSION;
	}
	return ICSRANK_INVALID;
}

inline bool isProperSubsequence(CvQualifiers l, CvQualifiers r)
{
	return (!l.isConst && r.isConst)
		|| (!l.isVolatile && r.isVolatile);
}

inline bool isProperSubsequence(const StandardConversionSequence& l, const StandardConversionSequence& r)
{
	return (l.rank == SCSRANK_IDENTITY && r.rank != SCSRANK_IDENTITY)
		|| isProperSubsequence(l.adjustment, r.adjustment);
}

// [over.ics.rank]
inline bool isBetter(const StandardConversionSequence& l, const StandardConversionSequence& r)
{
	// TODO: assert rank not INVALID
	if(isProperSubsequence(l, r))
	{
		return true;
	}
	if(l.rank < r.rank) // TODO: ranking derived->base conversions by inheritance distance
	{
		return true;
	}
	// TODO: both sequences are similar references, but differ only in cv-qualification
	// TODO: user-defined conversion sequence ranking
	return false;
}

enum IcsType
{
	ICSTYPE_STANDARD,
	ICSTYPE_USERDEFINED,
	ICSTYPE_ELLIPSIS,
};

struct ImplicitConversion
{
	StandardConversionSequence sequence;
	IcsType type;
	ImplicitConversion(StandardConversionSequence sequence, IcsType type = ICSTYPE_STANDARD)
		: sequence(sequence), type(type)
	{
	}
};

const ImplicitConversion IMPLICITCONVERSION_USERDEFINED = ImplicitConversion(StandardConversionSequence(SCSRANK_IDENTITY, CvQualifiers()), ICSTYPE_USERDEFINED); // TODO

// [over.ics.rank]
inline bool isBetter(const ImplicitConversion& l, const ImplicitConversion& r)
{
	if(l.type < r.type)
	{
		return true;
	}
	return isBetter(l.sequence, r.sequence);
}

inline bool isValid(const ImplicitConversion& conversion)
{
	return conversion.sequence.rank != SCSRANK_INVALID;
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


inline bool isDependentOld(const Type& type, const DependentContext& context);

inline bool isDependentOld(const Type* qualifying, const DependentContext& context)
{
	if(qualifying == 0)
	{
		return false;
	}
	const Type& instantiated = getInstantiatedType(*qualifying);
	if(isDependentOld(instantiated.qualifying.get(), context))
	{
		return true;
	}
	return isDependentOld(instantiated, context);
}

inline bool isDependentOld(const Types& bases, const DependentContext& context)
{
	for(Types::const_iterator i = bases.begin(); i != bases.end(); ++i)
	{
		if((*i).visited)
		{
			continue;
		}
		if(isDependentOld(*i, context))
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

inline bool isDependentOld(const TemplateArguments& arguments, const DependentContext& context)
{
	for(TemplateArguments::const_iterator i = arguments.begin(); i != arguments.end(); ++i)
	{
		if((*i).type.visited)
		{
			continue;
		}
		if(evaluateDependent((*i).dependent, context) // array-size or constant-initializer
			|| ((*i).type.declaration != 0
				&& isDependentOld((*i).type, context)))
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
		return isDependentOld(original.qualifying.get(), context);
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
			if(isDependentOld(*i, context))
			{
				return true;
			}
		}
	}
	else
	{
		if(isDependentOld(original.templateArguments, context))
		{
			return true;
		}
	}
	Scope* enclosed = original.declaration->enclosed;
	if(enclosed != 0)
	{
		bool result = isDependentOld(enclosed->bases, context);
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
#if 1 // TEMPORARY HACK: workaround for issue with copying of template-params scope
	if(type.declaration->scope != 0
		&& type.declaration->scope->type == SCOPETYPE_TEMPLATE)
	{
		return true;
	}
#endif

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
#if 0
		if(original.declaration->templateParams.empty())
		{
			// we haven't finished parsing the class-declaration.
			// we can assume 'original' refers to the current-instantation.
			return true;
		}
#endif
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



inline bool isDependentOld(const Type& type, const DependentContext& context)
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
		|| isDependentOld(declaration->type, context)
		|| evaluateDependent(declaration->valueDependent, context);
}

inline bool isDependentType(const Type* type, const DependentContext& context)
{
	return isDependentOld(*type, context);
}

inline bool isDependentTypeRef(TypePtr::Value* type, const DependentContext& context)
{
	return isDependentType(type, context);
}

inline bool isDependentClass(Scope* scope, const DependentContext& context)
{
	return isDependentOld(scope->bases, context);
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
			return second; // redeclaration of typedef, or definition of type previously used in typedef
		}
		if(isTypedef(second))
		{
			return first; // typedef of type previously declared
		}
		if(isClass(first))
		{
			if(isSpecialization(second))
			{
				return second; // TODO: class template partial/explicit-specialization
			}
			if(isSpecialization(first))
			{
				return second; // TODO: class template partial/explicit-specialization
			}
			if(isIncomplete(second))
			{
				return first; // redeclaration of previously-declared class
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
	if(!first.templateParams.defaults.empty())
	{
		// if enclosing is a template
		return first;
	}
	throw DeclarationError("symbol already defined");
}




inline const DeclarationInstance* findDeclaration(Scope::Declarations& declarations, const Identifier& id, LookupFilter filter = IsAny(), bool isBase = false)
{
	Scope::Declarations::iterator i = declarations.upper_bound(id.value);

	for(; i != declarations.begin()
		&& (*--i).first == id.value;)
	{
		if(!isBase
			|| (*i).second->templateParameter == INDEX_INVALID) // template-params of base-class are not visible outside the class
		{
			if(filter(*(*i).second))
			{
				return &(*i).second;
			}
		}
	}

	return 0;
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
			&& base.id->value == id.value
			&& filter(*base.declaration))
		{
			result.filtered = base.id->dec.p;
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

inline LookupResult findDeclarationWithin(Scope& scope, const Identifier& id, LookupFilter filter = IsAny())
{
	LookupResult result;
	if(result.append(::findDeclaration(scope.declarations, scope.bases, id, filter)))
	{
		return result;
	}
	SYMBOLS_ASSERT(!(scope.type == SCOPETYPE_CLASS && !scope.usingDirectives.empty())); // TODO: non-fatal error: 7.3.4: a using-directive shall not appear in class scope
	if(result.append(::findDeclaration(scope.usingDirectives, id, filter)))
	{
		return result;
	}
	return result;
}

inline bool hasTemplateParamDefaults(const TemplateParameters& params)
{
	for(TypeIds::const_iterator i = params.defaults.begin(); i != params.defaults.end(); ++i)
	{
		if((*i).declaration != 0)
		{
			return true;
		}
	}
	return false;
}

// substitute references to template-parameters of 'otherParams' for template-parameters of 'params'
inline void fixTemplateParamDefault(TypeId& type, const TemplateParameters& params, const TemplateParameters& otherParams)
{
	if(type.declaration == 0)
	{
		return;
	}
	std::size_t index = type.declaration->templateParameter;
	if(index != INDEX_INVALID)
	{
		Types::const_iterator i = params.begin();
		std::advance(i, index);
		Types::const_iterator j = otherParams.begin();
		std::advance(j, index);
		if(type.declaration->scope == (*j).declaration->scope)
		{
			type.declaration = (*i).declaration;
		}
	}
	for(TemplateArguments::iterator i = type.templateArguments.begin(); i != type.templateArguments.end(); ++i)
	{
		SYMBOLS_ASSERT((*i).dependent.empty());
		fixTemplateParamDefault((*i).type, params, otherParams);
	}
}

inline void copyTemplateParamDefault(TypeId& type, const TypeId& otherType, const TemplateParameters& params, const TemplateParameters& otherParams)
{
	type = otherType;
	fixTemplateParamDefault(type, params, otherParams);
}

inline void copyTemplateParamDefaults(TemplateParameters& params, const TemplateParameters& otherParams)
{
	SYMBOLS_ASSERT(params.defaults.empty());
	for(TypeIds::const_iterator i = otherParams.defaults.begin(); i != otherParams.defaults.end(); ++i)
	{
		params.defaults.push_back(TYPE_NULL);
		copyTemplateParamDefault(params.defaults.back(), *i, params, otherParams);
	}
}

/// 14.1-10: the set of template param defaults is obtained by merging those from the definition and all declarations currently in scope (excluding explicit-specializations)
inline void mergeTemplateParamDefaults(TemplateParameters& params, const TemplateParameters& otherParams)
{
	if(params.defaults.empty())
	{
		copyTemplateParamDefaults(params, otherParams);
		return;
	}
	if(!hasTemplateParamDefaults(otherParams)) // ignore declarations with no default-arguments, e.g. explicit/partial-specializations
	{
		return;
	}
	SYMBOLS_ASSERT(!otherParams.defaults.empty());
	TypeIds::iterator d = params.defaults.begin();
	for(TypeIds::const_iterator i = otherParams.defaults.begin(); i != otherParams.defaults.end(); ++i)
	{
		SYMBOLS_ASSERT(d != params.defaults.end());
		SYMBOLS_ASSERT((*d).declaration == 0 || (*i).declaration == 0); // TODO: non-fatal error: default param defined more than once
		if((*d).declaration == 0)
		{
			copyTemplateParamDefault(*d, *i, params, otherParams);
		}
		++d;
	}
	SYMBOLS_ASSERT(d == params.defaults.end());
}

inline void mergeTemplateParamDefaults(Declaration& declaration, const TemplateParameters& templateParams)
{
	SYMBOLS_ASSERT(declaration.isTemplate);
	SYMBOLS_ASSERT(isClass(declaration));
	SYMBOLS_ASSERT(!isSpecialization(declaration)); // explicit/partial-specializations cannot have default-arguments
	mergeTemplateParamDefaults(declaration.templateParams, templateParams);
	SYMBOLS_ASSERT(!declaration.templateParams.defaults.empty());
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
	void visit(const DeclaratorDependent& element)
	{
		throw SymbolsError(); // error!
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

	void visit(const DeclaratorDependent& dependent)
	{
		printName(dependent.type);
		visitTypeElement();
	}
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
		if(pointer.qualifiers.isConst)
		{
			printer.out << "const ";
		}
		if(pointer.qualifiers.isVolatile)
		{
			printer.out << "volatile ";
		}
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
	void visit(const DeclaratorMemberPointer& pointer)
	{
		pushType(true);
		printer.out << "::*";
		if(pointer.qualifiers.isConst)
		{
			printer.out << "const ";
		}
		if(pointer.qualifiers.isVolatile)
		{
			printer.out << "volatile ";
		}
		visitTypeElement();
		popType();
	}
	void visit(const DeclaratorFunction& function)
	{
		pushType(false);
		visitTypeElement();
		printParameters(function.parameters);
		if(function.qualifiers.isConst)
		{
			printer.out << " const";
		}
		if(function.qualifiers.isVolatile)
		{
			printer.out << " volatile";
		}
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
		GetTypeHistory::apply(type, typeHistory);
		visitTypeHistory(typeHistory);
		visitTypeElement();
	}

	void printTypeSequence(const Declaration& declaration)
	{
		DeclarationHistory typeHistory;
		GetTypeHistory::apply(declaration, typeHistory);
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
	void printParameters(const Parameters& parameters)
	{
		printer.out << "(";
		bool separator = false;
		for(Parameters::const_iterator i = parameters.begin(); i != parameters.end(); ++i)
		{
			const Declaration* declaration = (*i).declaration;
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
		best.conversions.resize(count, ImplicitConversion(STANDARDCONVERSIONSEQUENCE_INVALID));
	}
	Declaration* get() const
	{
		return ambiguous != 0 ? 0 : best.declaration;
	}
	bool isViable(const CandidateFunction& candidate)
	{
		if(candidate.conversions.size() != best.conversions.size())
		{
			return false; // TODO: early-out for functions with not enough params
		}

		for(ArgumentConversions::const_iterator i = candidate.conversions.begin(); i != candidate.conversions.end();  ++i)
		{
			if(!isValid(*i))
			{
				return false;
			}
		}

		return true;
	}
	void add(const CandidateFunction& candidate)
	{
		if(best.declaration != 0
			&& candidate.declaration->enclosed == best.declaration->enclosed)
		{
			return; // TODO: don't add multiple declarations of same signature
		}

		if(!isViable(candidate))
		{
			return;
		}

		if(best.declaration == 0
			|| isBetter(candidate, best))
		{
			best = candidate;
			ambiguous = 0;
		}
		else if(!isBetter(best, candidate)) // the best candidate is an equally good match
		{
			ambiguous = candidate.declaration;
		}
	}
	void addSingle(Declaration* declaration)
	{
		CandidateFunction candidate(declaration);
		candidate.conversions.reserve(best.conversions.size());

#if 1
		UniqueTypeWrapper type = makeUniqueType(declaration->type, 0, true); // TODO: dependent types, template argument deduction
		if(!type.isFunction())
		{
			return; // TODO: invoke operator() on object of class-type
		}
		const ParameterTypes& parameters = getParameterTypes(type.value);
		UniqueTypeIds::const_iterator a = arguments.begin();
		Parameters::const_iterator p = getParameters(type.value).begin();
		// TODO: ellipsis
		for(ParameterTypes::const_iterator i = parameters.begin(); i != parameters.end(); ++i)
		{
			UniqueTypeId to = (*i);
			UniqueTypeId from;
			if(a != arguments.end())
			{
				candidate.conversions.push_back(makeStandardConversionSequence(to, *a)); // TODO: null-pointer-constant, l-value
				++a;
			}
			else if((*p).argument == 0) // TODO: catch this earlier
			{
				return; // [over.match.viable] no default-argument available, this candidate is not viable
			}
			else
			{
				SYMBOLS_ASSERT((*p).argument->expr != 0); // TODO: non-fatal error: trying to use a default-argument before it has been declared. 
			}
			++p;
		}
#else
		UniqueTypeIds::const_iterator a = arguments.begin();
		for(Scope::DeclarationList::iterator i = declaration->enclosed->declarationList.begin(); i != declaration->enclosed->declarationList.end(); ++i)
		{
			const Declaration& parameter = *(*i);
			UniqueTypeId type = makeUniqueType(parameter.type); // TODO: template argument deduction
			if(a != arguments.end())
			{
				candidate.conversions.push_back(makeStandardConversionSequence(type, *a)); // TODO: null-pointer-constant, l-value
				++a;
			}
			else
			{
				break; // TODO: default parameter values
			}
		}
#endif

		add(candidate);
	}
	void add(Declaration* declaration)
	{
		for(Declaration* p = declaration; p != 0; p = p->overloaded)
		{
			if(p->enclosed == 0)
			{
				continue;
			}

			if(p->isTemplate)
			{
				continue; // TODO: template argument deduction
			}

			addSingle(p);
		}
	}
};

inline Declaration* findBestMatch(Declaration* declaration, const UniqueTypeIds& arguments)
{
	OverloadResolver resolver(arguments);
	resolver.add(declaration);

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


