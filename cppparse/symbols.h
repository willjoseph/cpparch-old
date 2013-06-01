
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
#if 0
	virtual bool operator==(const SequenceNode& other) const = 0;
#endif

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
#if 0
	virtual bool operator==(const SequenceNode<Visitor>& other) const
	{
		throw SymbolsError();
	}
#endif
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
#if 0
	bool operator==(const SequenceNode<Visitor>& other) const
	{
		return typeid(*this) == typeid(other)
			&& value == static_cast<const SequenceNodeGeneric*>(&other)->value;
	}
#endif
#if 0
	bool operator<(const SequenceNode<Visitor>& other) const
	{
		return (typeid(*this).before(typeid(other)) ||
			!(typeid(other).before(typeid(*this))) && value < static_cast<const SequenceNodeGeneric*>(&other)->value);
	}
#endif
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


struct TypeSequenceVisitor
{
	virtual void visit(const struct DeclaratorPointerType&) = 0;
	virtual void visit(const struct DeclaratorReferenceType&) = 0;
	virtual void visit(const struct DeclaratorArrayType&) = 0;
	virtual void visit(const struct DeclaratorMemberPointerType&) = 0;
	virtual void visit(const struct DeclaratorFunctionType&) = 0;
};

typedef Sequence<TreeAllocator<int>, TypeSequenceVisitor> TypeSequence;

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
	UniqueType unique;
	bool isDependent; // true if the type is dependent in the context in which it was parsed
	bool isImplicitTemplateId; // true if this is a template but the template-argument-clause has not been specified
	bool isEnclosingClass; // true if this is the type of an enclosing class
	mutable bool visited; // use while iterating a set of types, to avoid visiting the same type twice (an optimisation, and a mechanism for handling cyclic dependencies)
	Type(Declaration* declaration, const TreeAllocator<int>& allocator)
		: id(0), declaration(declaration), templateArguments(allocator), qualifying(allocator), dependent(0), enclosingTemplate(0), specialization(INDEX_INVALID), unique(0), isDependent(false), isImplicitTemplateId(false), isEnclosingClass(false), visited(false)
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
		std::swap(unique, other.unique);
		std::swap(isDependent, other.isDependent);
		std::swap(isImplicitTemplateId, other.isImplicitTemplateId);
		std::swap(isEnclosingClass, other.isEnclosingClass);
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

// refers to the innermost template scope that a name/type/expression depends on
struct Dependent : DeclarationPtr
{
	Dependent() : DeclarationPtr(0)
	{
	}
};

#if 0
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
#endif

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
		: type(0, allocator)
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
	size_t uniqueId;
	Scope* scope;
	TypeId type;
	Scope* enclosed;
	Scope* templateParamScope;
	Declaration* overloaded;
	Dependent valueDependent; // the dependent-types/names that are referred to in the declarator-suffix (array size)
	DeclSpecifiers specifiers;
	size_t templateParameter;
	TemplateParameters templateParams;
	TemplateArguments templateArguments; // non-empty if this is an explicit (or partial) specialization
	bool isTemplate;
	bool isSpecialization;
	bool isFunctionDefinition;

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
		const Dependent& valueDependent = Dependent()
	) : name(&name),
		uniqueId(0),
		scope(scope),
		type(type),
		enclosed(enclosed),
		templateParamScope(0),
		overloaded(0),
		valueDependent(valueDependent),
		specifiers(specifiers),
		templateParameter(templateParameter),
		templateParams(templateParams),
		templateArguments(templateArguments),
		isTemplate(isTemplate),
		isSpecialization(isSpecialization),
		isFunctionDefinition(false)
	{
	}
	Declaration() :
		type(0, TREEALLOCATOR_NULL),
		templateParams(TREEALLOCATOR_NULL),
		templateArguments(TREEALLOCATOR_NULL)
	{
	}
	void swap(Declaration& other)
	{
		std::swap(name, other.name);
		std::swap(uniqueId, other.uniqueId);
		std::swap(scope, other.scope);
		type.swap(other.type);
		std::swap(enclosed, other.enclosed);
		std::swap(templateParamScope, other.templateParamScope);
		std::swap(overloaded, other.overloaded);
		std::swap(valueDependent, other.valueDependent);
		std::swap(specifiers, other.specifiers);
		std::swap(templateParameter, other.templateParameter);
		templateParams.swap(other.templateParams);
		templateArguments.swap(other.templateArguments);
		std::swap(isTemplate, other.isTemplate);
		std::swap(isSpecialization, other.isSpecialization);
		std::swap(isFunctionDefinition, other.isFunctionDefinition);
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

// An instance of a declaration - multiple declarations may refer to the same entity.
// e.g. definition, forward declaration, redeclaration
struct DeclarationInstance : DeclarationPtr
{
	Identifier* name; // the identifier used in this declaration.
	const DeclarationInstance* overloaded; // the previously declared overload of this name (which may or may not refer to the same entity.)
	const DeclarationInstance* redeclared; // the previous declaration that refers to the same entity.
	DeclarationInstance()
		: DeclarationPtr(0), name(0), overloaded(0), redeclared(0)
	{
	}
	// used when cloning an existing declaration, in the process of copying declarations from one scope to another.
	explicit DeclarationInstance(Declaration* declaration)
		: DeclarationPtr(declaration), name(declaration != 0 ? &declaration->getName() : 0), overloaded(0), redeclared(0)
	{
		SYMBOLS_ASSERT(name != 0);
	}
#if 0
	explicit DeclarationInstance(DeclarationPtr declaration)
		: DeclarationPtr(declaration), name(declaration != 0 ? &declaration->getName() : 0), overloaded(0), redeclared(0)
	{
		SYMBOLS_ASSERT(name != 0);
	}
#endif
};

inline bool operator<(const DeclarationInstance& l, const DeclarationInstance& r)
{
	return l.name < r.name;
}

inline cpp::terminal_identifier& getDeclarationId(const DeclarationInstance* declaration)
{
	return (*declaration)->getName();
}

inline bool isDecorated(const Identifier& id)
{
	return id.dec.p != 0;
}

inline const DeclarationInstance& getDeclaration(const Identifier& id)
{
	SYMBOLS_ASSERT(isDecorated(id));
	return *id.dec.p;
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

#if 0
	DeferredLookup deferred;
	size_t deferredCount;
#endif
	size_t templateDepth;

	Scope(const TreeAllocator<int>& allocator, const Identifier& name, ScopeType type = SCOPETYPE_UNKNOWN)
		: parent(0), name(name), enclosedScopeCount(0), declarations(allocator), type(type), bases(allocator), usingDirectives(allocator), declarationList(allocator),
#if 0
		deferred(allocator), deferredCount(0),
#endif
		templateDepth(0)

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

inline Scope::Declarations::iterator findDeclaration(Scope::Declarations& declarations, const DeclarationInstance* declaration)
{
	const Identifier& id = *declaration->name;
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

inline void undeclare(const DeclarationInstance* p, LexerAllocator& allocator)
{
	SYMBOLS_ASSERT(p->name->dec.p == 0 || p->name->dec.p == p);
	p->name->dec.p = 0;

	Declaration* declaration = *p;
	SYMBOLS_ASSERT(!declaration->scope->declarations.empty());
	SYMBOLS_ASSERT(!declaration->scope->declarationList.empty());

	SYMBOLS_ASSERT(declaration == declaration->scope->declarationList.back());
	declaration->scope->declarationList.pop_back(); // TODO: optimise

	Scope::Declarations::iterator i = findDeclaration(declaration->scope->declarations, p);
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
// e.g. int(*)[] == pointer to array of == DeclaratorPointerType -> DeclaratorArrayType
// Note that this is the reverse of the order that the declarator is parsed in.
// This means a given unique type sub-sequence need only be stored once.
// This allows fast comparison of types and simplifies printing of declarators.

struct TypeElementVisitor
{
	virtual void visit(const struct DependentType&) = 0;
	virtual void visit(const struct DependentTypename&) = 0;
	virtual void visit(const struct ObjectType&) = 0;
	virtual void visit(const struct PointerType&) = 0;
	virtual void visit(const struct ReferenceType&) = 0;
	virtual void visit(const struct ArrayType&) = 0;
	virtual void visit(const struct MemberPointerType&) = 0;
	virtual void visit(const struct FunctionType&) = 0;
};

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
		SYMBOLS_ASSERT(value != 0);
		SYMBOLS_ASSERT(value != UNIQUETYPE_NULL);
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
		return typeid(*value) == typeid(TypeElementGeneric<ObjectType>);
	}
	bool isPointer() const
	{
		return typeid(*value) == typeid(TypeElementGeneric<PointerType>);
	}
	bool isReference() const
	{
		return typeid(*value) == typeid(TypeElementGeneric<ReferenceType>);
	}
	bool isArray() const
	{
		return typeid(*value) == typeid(TypeElementGeneric<ArrayType>);
	}
	bool isMemberPointer() const
	{
		return typeid(*value) == typeid(TypeElementGeneric<MemberPointerType>);
	}
	bool isFunction() const
	{
		return typeid(*value) == typeid(TypeElementGeneric<FunctionType>);
	}
	bool isDependent() const
	{
		return typeid(*value) == typeid(TypeElementGeneric<DependentType>)
			|| typeid(*value) == typeid(TypeElementGeneric<DependentTypename>);
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

typedef UniqueTypeWrapper UniqueTypeId;

const UniqueTypeId gUniqueTypeNull = UniqueTypeId(UNIQUETYPE_NULL);

inline bool isEqual(const UniqueTypeId& l, const UniqueTypeId& r)
{
	return l.value == r.value;
}

inline UniqueType getInner(UniqueType type)
{
	SYMBOLS_ASSERT(typeid(*type) != typeid(TypeElementGeneric<struct ObjectType>));
	return type->next;
}

inline bool isEqualInner(const UniqueTypeId& l, const UniqueTypeId& r)
{
	return getInner(l.value) == getInner(r.value);
}


typedef std::vector<UniqueTypeWrapper> TemplateArgumentsInstance;
typedef std::vector<UniqueTypeWrapper> SpecializationTypes;
typedef std::vector<const struct TypeInstance*> UniqueBases;

typedef Name Location;

struct TypeInstance
{
	size_t uniqueId;
	DeclarationPtr declaration;
	TemplateArgumentsInstance templateArguments;
	const TypeInstance* enclosing; // the enclosing template
	UniqueBases bases;
	SpecializationTypes specializations; // the types of the dependent-names in the specialization
	bool instantiated;
	mutable bool visited; // used during findDeclaration to prevent infinite recursion
	Location instantiation;
	TypeInstance(Declaration* declaration, const TypeInstance* enclosing)
		: uniqueId(0), declaration(declaration), enclosing(enclosing), instantiated(false), visited(false), instantiation(NAME_NULL)
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

struct ObjectType
{
	TypeInstance type;
	ObjectType(const TypeInstance& type)
		: type(type)
	{
	}
};

inline bool operator<(const ObjectType& left, const ObjectType& right)
{
	return left.type < right.type;
}

inline const TypeInstance& getObjectType(UniqueType type)
{
	SYMBOLS_ASSERT(typeid(*type) == typeid(TypeElementGeneric<ObjectType>));
	return static_cast<const TypeElementGeneric<ObjectType>*>(type.getPointer())->value.type;
}


struct DependentType
{
	DeclarationPtr type; // the declaration of the template parameter
	DependentType(Declaration* type)
		: type(type)
	{
	}
};

inline bool operator<(const DependentType& left, const DependentType& right)
{
	return left.type->scope->templateDepth != right.type->scope->templateDepth
		? left.type->scope->templateDepth < right.type->scope->templateDepth
		: left.type->templateParameter < right.type->templateParameter;
}


struct DependentTypename
{
	IdentifierPtr name; // the type name
	DependentTypename(Identifier* name)
		: name(name)
	{
	}
};

inline bool operator<(const DependentTypename& left, const DependentTypename& right)
{
	return left.name->value < right.name->value;
}


struct DeclaratorPointerType
{
	CvQualifiers qualifiers;
	DeclaratorPointerType()
	{
	}
	explicit DeclaratorPointerType(CvQualifiers qualifiers)
		: qualifiers(qualifiers)
	{
	}
};

struct PointerType
{
	PointerType()
	{
	}
};

inline bool operator<(const PointerType& left, const PointerType& right)
{
	return false;
}

struct DeclaratorReferenceType
{
};

struct ReferenceType
{
};

inline bool operator<(const ReferenceType& left, const ReferenceType& right)
{
	return false;
}

struct DeclaratorMemberPointerType
{
	Type type;
	CvQualifiers qualifiers;
	DeclaratorMemberPointerType(const Type& type, CvQualifiers qualifiers)
		: type(type), qualifiers(qualifiers)
	{
	}
};

struct MemberPointerType
{
	const TypeInstance* instance;
	MemberPointerType(const TypeInstance* instance)
		: instance(instance)
	{
	}
};

inline const TypeInstance& getMemberPointerClass(UniqueType type)
{
	SYMBOLS_ASSERT(typeid(*type) == typeid(TypeElementGeneric<MemberPointerType>));
	return *(static_cast<const TypeElementGeneric<MemberPointerType>*>(type.getPointer())->value.instance);
}

inline bool operator<(const MemberPointerType& left, const MemberPointerType& right)
{
	return left.instance < right.instance;
}

typedef std::vector<size_t> ArrayRank;

struct DeclaratorArrayType
{
	ArrayRank rank; // TODO: store expression to be evaluated when template-params are known
	DeclaratorArrayType(const ArrayRank& rank)
		: rank(rank)
	{
	}
};

struct ArrayType
{
	std::size_t size;
	ArrayType(std::size_t size)
		: size(size)
	{
	}
};

inline bool operator<(const ArrayType& left, const ArrayType& right)
{
	return left.size < right.size;
}

inline const ArrayType& getArrayType(UniqueType type)
{
	SYMBOLS_ASSERT(typeid(*type) == typeid(TypeElementGeneric<ArrayType>));
	return static_cast<const TypeElementGeneric<ArrayType>*>(type.getPointer())->value;
}

struct DeclaratorFunctionType
{
	Parameters parameters;
	CvQualifiers qualifiers;
	DeclaratorFunctionType(const Parameters& parameters, CvQualifiers qualifiers)
		: parameters(parameters), qualifiers(qualifiers)
	{
	}
};

typedef std::vector<UniqueTypeWrapper> ParameterTypes;

struct FunctionType
{
	ParameterTypes parameterTypes;
};

inline bool operator<(const FunctionType& left, const FunctionType& right)
{
	return left.parameterTypes < right.parameterTypes;
}

inline const TypeSequence::Node* getLastNode(const TypeSequence& typeSequence)
{
	const TypeSequence::Node* result = 0;
	for(const TypeSequence::Node* node = typeSequence.get(); node != 0; node = node->get())
	{
		result = node;
	}
	return result;
}
inline const Parameters& getParameters(const TypeId& type)
{
	const TypeSequence::Node* node = getLastNode(type.typeSequence);
	SYMBOLS_ASSERT(node != 0);
	SYMBOLS_ASSERT(typeid(*node) == typeid(SequenceNodeGeneric<DeclaratorFunctionType, TypeSequenceVisitor>));
	return static_cast<const SequenceNodeGeneric<DeclaratorFunctionType, TypeSequenceVisitor>*>(node)->value.parameters;
}

inline const ParameterTypes& getParameterTypes(UniqueType type)
{
	SYMBOLS_ASSERT(typeid(*type) == typeid(TypeElementGeneric<FunctionType>));
	return static_cast<const TypeElementGeneric<FunctionType>*>(type.getPointer())->value.parameterTypes;
}


// ----------------------------------------------------------------------------


inline UniqueTypeWrapper adjustFunctionParameter(UniqueTypeWrapper type)
{
	UniqueTypeWrapper result(type.value.getPointer());  // ignore cv-qualifiers
	if(type.isFunction()) // T() becomes T(*)()
	{
		pushUniqueType(result.value, PointerType());
	}
	else if(type.isArray()) // T[] becomes T*
	{
		popUniqueType(result.value);
		pushUniqueType(result.value, PointerType());
	}
	return result;
}

inline bool isFunctionParameterEquivalent(UniqueTypeWrapper left, UniqueTypeWrapper right)
{
	return adjustFunctionParameter(left) == adjustFunctionParameter(right);
}

inline bool isEquivalent(const ParameterTypes& left, const ParameterTypes& right)
{
	ParameterTypes::const_iterator l = left.begin();
	ParameterTypes::const_iterator r = right.begin();
	for(;; ++l, ++r)
	{
		if(l == left.end())
		{
			return r == right.end();
		}
		if(r == right.end())
		{
			return false;
		}
		if(!isFunctionParameterEquivalent(*l, *r))
		{
			return false;
		}
	}
	return true;
}


// ----------------------------------------------------------------------------

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
		value = pushBuiltInType(value, ObjectType(TypeInstance(declaration, 0)));
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

// ----------------------------------------------------------------------------
// template instantiation


inline UniqueTypeWrapper makeUniqueType(const TypeId& type, Location source);
inline UniqueTypeWrapper makeUniqueType(const TypeId& type, Location source, const TypeInstance* enclosing, bool allowDependent, std::size_t depth);
inline UniqueTypeWrapper makeUniqueType(const Type& type, Location source, const TypeInstance* enclosing, bool allowDependent, std::size_t depth);

struct TypeError
{
	TypeError()
	{
	}
};

inline UniqueTypeWrapper makeUniqueType(const TypeId& type, Location source, const TypeInstance* enclosing, bool allowDependent = false)
{
	try
	{
		return makeUniqueType(type, source, enclosing, allowDependent, 0);
	}
	catch(TypeError)
	{
		std::cout << "makeUniqueType failed!" << std::endl;
		extern ObjectTypeId gBaseClass;
		return gBaseClass;
	}
}

inline UniqueTypeWrapper makeUniqueType(const Type& type, Location source, const TypeInstance* enclosing, bool allowDependent = false)
{
	try
	{
		return makeUniqueType(type, source, enclosing, allowDependent, 0);
	}
	catch(TypeError)
	{
		std::cout << "makeUniqueType failed!" << std::endl;
		extern ObjectTypeId gBaseClass;
		return gBaseClass;
	}
}

#if 0
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
#endif
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

inline Scope* findEnclosingClassTemplate(Declaration* dependent)
{
	if(dependent != 0
		&& isClass(*dependent)
		&& dependent->scope != 0
		&& isDecorated(dependent->scope->name))
	{
		return isTemplate(*dependent->scope)
			? dependent->scope
			: findEnclosingClassTemplate(getDeclaration(dependent->scope->name));
	}
	return 0;
}

inline bool isDependent(Declaration* dependent, Scope* enclosing, Scope* templateParamScope)
{
	return dependent != 0
		&& (findScope(enclosing, dependent->scope) != 0
		|| findScope(templateParamScope, dependent->scope) != 0); // if we are within the candidate template-parameter's template-definition
}

inline void printType(const TypeInstance& type);

inline void instantiateClass(const TypeInstance& enclosing, Location source, bool allowDependent = false)
{
	SYMBOLS_ASSERT(isClass(*enclosing.declaration));
	if(!enclosing.instantiated)
	{
		TypeInstance& instance = const_cast<TypeInstance&>(enclosing);
		instance.instantiated = true; // prevent recursion
		instance.instantiation = source;
		if(enclosing.declaration->enclosed == 0)
		{
			std::cout << "instantiateClass failed: ";
			printType(enclosing);
			std::cout << std::endl;
			return; // TODO: this can occur when the primary template is incomplete, and a specialization was not chosen
		}
		SYMBOLS_ASSERT(enclosing.declaration->enclosed != 0);
		Types& bases = enclosing.declaration->enclosed->bases;
		UniqueBases uniqueBases;
		uniqueBases.reserve(std::distance(bases.begin(), bases.end()));
		for(Types::const_iterator i = bases.begin(); i != bases.end(); ++i)
		{
			UniqueTypeId base = makeUniqueType(*i, source, &enclosing, allowDependent);
			if((*i).unique != 0
				&& !(*i).isDependent)
			{
				SYMBOLS_ASSERT(base.value == (*i).unique);
			}
			if(allowDependent && (*i).isDependent)
			{
				continue;
			}
			const TypeInstance& objectType = getObjectType(base.value);
			if(!isClass(*objectType.declaration)) // TODO: this can occur when the primary template derives from its template parameter, and the specialization for a non-class argument was not chosen
			{
				continue;
			}
			if(objectType.declaration->enclosed == 0) // TODO: this can occur when the primary template is incomplete, and a specialization was not chosen
			{
				continue;
			}
			uniqueBases.push_back(&objectType);
			instantiateClass(objectType, source, allowDependent);
		}
		instance.bases.swap(uniqueBases); // prevent searching a partially evaluated set of base classes inside 'makeUniqueType'
	}
}

inline void requireCompleteObjectType(UniqueTypeWrapper type, Location source)
{
	while(type.isArray()
		&& getArrayType(type.value).size != 0)
	{
		type.pop_front(); // arrays of known size are complete object types
	}
	if(type.isSimple())
	{
		const TypeInstance& objectType = getObjectType(type.value);
		if(isClass(*objectType.declaration))
		{
			instantiateClass(objectType, source);
		}
	}
}

 // [expr] If an expression initially has the type "reference to T", the type is adjusted to "T" prior to any further analysis.
inline UniqueTypeWrapper removeReference(UniqueTypeWrapper type)
{
	if(type.isReference())
	{
		type.pop_front();
	}
	return type;
}

inline const TypeInstance* makeUniqueEnclosing(const Qualifying& qualifying, Location source, const TypeInstance* enclosing, bool allowDependent, std::size_t depth, UniqueTypeWrapper& tmp)
{
	if(!qualifying.empty())
	{
		if(isNamespace(*qualifying.back().declaration))
		{
			return 0; // name is qualified by a namespace, therefore cannot be enclosed by a class
		}
		tmp = makeUniqueType(qualifying.back(), source, enclosing, allowDependent, depth);
		if(allowDependent && tmp.isDependent())
		{
			return 0;
		}
		const TypeInstance& type = getObjectType(tmp.value);
		// [temp.inst] A class template is implicitly instantiated ... if the completeness of the class-type affects the semantics of the program.
		instantiateClass(type, source, allowDependent);
		return &type;
	}
	return enclosing;
}

inline const TypeInstance* makeUniqueEnclosing(const Qualifying& qualifying, Location source, const TypeInstance* enclosing, bool allowDependent, std::size_t depth)
{
	UniqueTypeWrapper tmp;
	return makeUniqueEnclosing(qualifying, source, enclosing, allowDependent, depth, tmp);
}

inline const TypeInstance* makeUniqueEnclosing(const Qualifying& qualifying, Location source, const TypeInstance* enclosing, bool allowDependent = false)
{
	try
	{
		return makeUniqueEnclosing(qualifying, source, enclosing, allowDependent, 0);
	}
	catch(TypeError)
	{
		std::cout << "makeUniqueEnclosing failed!" << std::endl;
		return 0;
	}
}

inline bool matchTemplateSpecialization(const Declaration& declaration, const TemplateArgumentsInstance& arguments, Location source, const TypeInstance* enclosing)
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
			type = makeUniqueType((*i).type, source, enclosing, true); // a partial-specialization may have dependent template-arguments: template<class T> class C<T*>
		}
		if(type != *a)
		{
			return false;
		}
	}
	return true;
}

inline Declaration* findTemplateSpecialization(Declaration* declaration, const TemplateArgumentsInstance& arguments, Location source, const TypeInstance* enclosing)
{
	for(; declaration != 0; declaration = declaration->overloaded)
	{
		if(!isSpecialization(*declaration))
		{
			continue;
		}

		if(matchTemplateSpecialization(*declaration, arguments, source, enclosing))
		{
			return declaration;
		}
	}
	return 0;
}


inline const TypeInstance* findEnclosingType(const TypeInstance& enclosing, Scope* scope)
{
	SYMBOLS_ASSERT(scope != 0);
	if(scope->type == SCOPETYPE_TEMPLATE)
	{
		return enclosing.declaration->templateParamScope == scope
			? &enclosing
			: 0; // don't search base classes for template-parameter
	}

	if(enclosing.declaration->enclosed == scope)
	{
		return &enclosing;
	}

	if(enclosing.declaration->enclosed != 0) // TODO: 'enclosing' may be incomplete if we're finding the enclosing type for a template default argument. 
	{
		SYMBOLS_ASSERT(enclosing.instantiated); // the enclosing type should have been instantiated by this point
	}

	for(UniqueBases::const_iterator i = enclosing.bases.begin(); i != enclosing.bases.end(); ++i)
	{
		const TypeInstance* result = findEnclosingType(*(*i), scope);
		if(result != 0)
		{
			return result;
		}
	}
	return 0;
}

inline const TypeInstance* findEnclosingType(const TypeInstance* enclosing, Scope* scope)
{
	SYMBOLS_ASSERT(scope != 0);
	for(const TypeInstance* i = enclosing; i != 0; i = (*i).enclosing)
	{
		const TypeInstance* result = findEnclosingType(*i, scope);
		if(result != 0)
		{
			return result;
		}
	}
	return 0;
}

inline const DeclarationInstance* findDeclaration(Scope::Declarations& declarations, const Identifier& id, LookupFilter filter);
inline LookupResult findDeclaration(const TypeInstance& instance, const Identifier& id, LookupFilter filter);

inline LookupResult findDeclaration(const UniqueBases& bases, const Identifier& id, LookupFilter filter)
{
	LookupResult result;
	for(UniqueBases::const_iterator i = bases.begin(); i != bases.end(); ++i)
	{
		const TypeInstance& base = *(*i);
		SYMBOLS_ASSERT(base.declaration->enclosed != 0); // TODO: non-fatal error: incomplete type
		SYMBOLS_ASSERT(base.declaration->enclosed->usingDirectives.empty()); // namespace.udir: A using-directive shall not appear in class scope, but may appear in namespace scope or in block scope.
		
		// an identifier looked up in the context of a class may name a base class
		if(base.declaration->getName().value == id.value
			&& filter(*base.declaration))
		{
			result.filtered = &getDeclaration(base.declaration->getName());
			return result;
		}

		if(result.append(findDeclaration(base, id, filter)))
		{
			return result;
		}
	}
	return result;
}

struct RecursionGuard
{
	const TypeInstance& instance;
	RecursionGuard(const TypeInstance& instance)
		: instance(instance)
	{
		SYMBOLS_ASSERT(!instance.visited);
		instance.visited = true;
	}
	~RecursionGuard()
	{
		instance.visited = false;
	}
};

inline LookupResult findDeclaration(const TypeInstance& instance, const Identifier& id, LookupFilter filter)
{
	SYMBOLS_ASSERT(instance.declaration->enclosed != 0);
	SYMBOLS_ASSERT(instance.instantiated); // the qualifying type should have been instantiated by this point

	LookupResult result;

	if(instance.visited) // TODO: this occurs when the same type is found twice when searching bases: can be caused by two templates that differ only in non-type template arguments
	{
		return result;
	}
	RecursionGuard guard(instance);

	if(result.append(findDeclaration(instance.declaration->enclosed->declarations, id, filter)))
	{
		return result;
	}
	if(result.append(findDeclaration(instance.bases, id, filter)))
	{
		return result;
	}
	return result;
}

inline UniqueTypeWrapper makeUniqueObjectType(const TypeInstance& type)
{
	return UniqueTypeWrapper(pushUniqueType(gUniqueTypes, UNIQUETYPE_NULL, ObjectType(type)));
}

// unqualified object name: int, Object,
// qualified object name: Qualifying::Object
// unqualified typedef: Typedef, TemplateParam
// qualified typedef: Qualifying::Type
// /p type
// /p enclosingType The enclosing template, required when uniquing a template-argument: e.g. Enclosing<int>::Type
//			Note: if 'type' is a class-template template default argument, 'enclosingType' will be the class-template, which does not require instantiation!
inline UniqueTypeWrapper makeUniqueType(const Type& type, Location source, const TypeInstance* enclosingType, bool allowDependent, std::size_t depth)
{
	if(depth++ == 256)
	{
		std::cout << "makeUniqueType reached maximum recursion depth!" << std::endl;
		throw TypeError();
	}
	// the type in which template-arguments are looked up: returns qualifying type if specified, else returns enclosingType
	UniqueTypeWrapper dependent;
	const TypeInstance* enclosing = makeUniqueEnclosing(type.qualifying, source, enclosingType, allowDependent, depth, dependent);
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
			if(allowDependent)
			{
				return UniqueTypeWrapper(pushUniqueType(gUniqueTypes, dependent.value, DependentTypename(type.id)));
			}
			else
			{
				std::cout << "lookup failed!" << std::endl;
			}
			extern ObjectTypeId gBaseClass;
			return gBaseClass;
		}
		declaration = *instance;
	}
#if 0
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
#endif
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
			return UniqueTypeWrapper(pushUniqueType(gUniqueTypes, UNIQUETYPE_NULL, DependentType(declaration)));
		}

		throw SymbolsError();
		return gUniqueTypeNull; // error: can't find template specialisation for this template parameter
	}

	const TypeInstance* memberEnclosing = isMember(*declaration) // if the declaration is a class member
		? findEnclosingType(enclosing, declaration->scope) // it must be a member of the base of the qualifying class: find which one.
		: 0; // the declaration is not a class member and cannot be found through qualified name lookup

	if(declaration->specifiers.isTypedef)
	{
		return makeUniqueType(declaration->type, source, memberEnclosing, allowDependent, depth);
	}

	TypeInstance tmp(declaration, memberEnclosing);
	SYMBOLS_ASSERT(declaration->type.declaration != &gArithmetic || tmp.enclosing == 0); // arithmetic types should not have an enclosing template!
	if(declaration->isTemplate)
	{
		tmp.declaration = findPrimaryTemplate(declaration); // TODO: look up explicit specialization

		bool isEnclosingSpecialization = type.isEnclosingClass && isSpecialization(*type.declaration);

		// [temp.local]: when the name of a template is used without arguments, substitute the parameters (in case of an enclosing explicit/partial-specialization, substitute the arguments
		const TypeIds& defaults = tmp.declaration->templateParams.defaults;
		SYMBOLS_ASSERT(!defaults.empty());
		if(type.isImplicitTemplateId // if no template argument list was specified
			&& !isEnclosingSpecialization) // and the type is not the name of an enclosing class-template explicit/partial-specialization
		{
			// when the type refers to a template-name outside an enclosing class, it is a template-template-parameter: we substitute the (possibly dependent) template parameters.
			bool dependent = allowDependent || !type.isEnclosingClass;
			for(Types::const_iterator i = tmp.declaration->templateParams.begin(); i != tmp.declaration->templateParams.end(); ++i)
			{
				const Type& argument = (*i);
				UniqueTypeWrapper result;
				extern Declaration gParam;
				if(argument.declaration->type.declaration == &gParam) // ignore non-type arguments
				{
					result = makeUniqueType(argument, source, enclosingType, dependent, depth);
					SYMBOLS_ASSERT(result.value != UNIQUETYPE_NULL);
				}
				tmp.templateArguments.push_back(result);
			}
			SYMBOLS_ASSERT(!tmp.templateArguments.empty());
		}
		else
		{
			const TemplateArguments& arguments = isEnclosingSpecialization
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
					const TypeInstance* enclosing = isTemplateParamDefault ? &tmp : enclosingType; // resolve dependent template-parameter-defaults in context of template class
					result = makeUniqueType(argument, source, enclosing, allowDependent, depth);
					SYMBOLS_ASSERT(result.value != UNIQUETYPE_NULL);
				}
				tmp.templateArguments.push_back(result);
			}
			SYMBOLS_ASSERT(!tmp.templateArguments.empty());
		}

		if(!allowDependent)
		{
			Declaration* specialization = findTemplateSpecialization(declaration, tmp.templateArguments, source, enclosing);
			if(specialization != 0)
			{
				tmp.declaration = specialization;
			}
		}
	}
	SYMBOLS_ASSERT(tmp.bases.empty());
	SYMBOLS_ASSERT(tmp.specializations.empty());
	static size_t uniqueId = 0;
	tmp.uniqueId = ++uniqueId;
	return makeUniqueObjectType(tmp);
}

inline UniqueTypeWrapper makeUniqueType(const Type& type, Location source)
{
	return makeUniqueType(type, source, 0);
}


struct TypeSequenceMakeUnique : TypeSequenceVisitor
{
	UniqueType& type;
	Location source;
	const TypeInstance* enclosing;
	bool allowDependent;
	TypeSequenceMakeUnique(UniqueType& type, Location source, const TypeInstance* enclosing, bool allowDependent)
		: type(type), source(source), enclosing(enclosing), allowDependent(allowDependent)
	{
	}
	void visit(const DeclaratorPointerType& element)
	{
		pushUniqueType(type, PointerType());
		type.setQualifiers(element.qualifiers);
	}
	void visit(const DeclaratorReferenceType& element)
	{
		pushUniqueType(type, ReferenceType());
	}
	void visit(const DeclaratorArrayType& element)
	{
		for(ArrayRank::const_reverse_iterator i = element.rank.rbegin(); i != element.rank.rend(); ++i)
		{
			pushUniqueType(type, ArrayType(*i));
		}
	}
	void visit(const DeclaratorMemberPointerType& element)
	{
		UniqueTypeWrapper tmp = makeUniqueType(element.type, source, enclosing, allowDependent);
		const TypeInstance* instance = allowDependent && tmp.isDependent() ? 0 : &getObjectType(tmp.value); // TODO: should be non-null even if dependent
		pushUniqueType(type, MemberPointerType(instance));
		type.setQualifiers(element.qualifiers);
	}
	void visit(const DeclaratorFunctionType& element)
	{
		FunctionType result;
		result.parameterTypes.reserve(element.parameters.size());
		for(Parameters::const_iterator i = element.parameters.begin(); i != element.parameters.end(); ++i)
		{
			result.parameterTypes.push_back(makeUniqueType((*i).declaration->type, source, enclosing, allowDependent));
		}
		pushUniqueType(type, result);
		type.setQualifiers(element.qualifiers);
	}
};

inline UniqueTypeWrapper makeUniqueType(const TypeId& type, Location source, const TypeInstance* enclosing, bool allowDependent, std::size_t depth)
{
	UniqueTypeWrapper result = makeUniqueType(*static_cast<const Type*>(&type), source, enclosing, allowDependent, depth);
	result.value.setQualifiers(type.qualifiers);
	TypeSequenceMakeUnique visitor(result.value, source, enclosing, allowDependent);
	type.typeSequence.accept(visitor);
	return result;
}

inline UniqueTypeWrapper makeUniqueType(const TypeId& type, Location source)
{
	return makeUniqueType(type, source, 0);
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

inline UniqueTypeId getExpressionType(cpp::expression* symbol)
{
	return UniqueTypeId(symbol->type.p);
}
inline void setExpressionType(cpp::expression* symbol, UniqueTypeId value)
{
	symbol->type.p = value.value;
}

inline UniqueTypeId getExpressionType(cpp::postfix_expression_suffix* symbol)
{
	return UniqueTypeId(symbol->type.p);
}
inline void setExpressionType(cpp::postfix_expression_suffix* symbol, UniqueTypeId value)
{
	symbol->type.p = value.value;
}

inline UniqueTypeId getExpressionType(cpp::type_id* symbol)
{
	return UniqueTypeId(symbol->type.p);
}
inline void setExpressionType(cpp::type_id* symbol, UniqueTypeId value)
{
	symbol->type.p = value.value;
}

inline UniqueTypeId getExpressionType(cpp::base_specifier* symbol)
{
	return UniqueTypeId(symbol->type.p);
}
inline void setExpressionType(cpp::base_specifier* symbol, UniqueTypeId value)
{
	symbol->type.p = value.value;
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
		value = pushBuiltInType(value, ArrayType(0));
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

inline const Type& getUnderlyingType(const Type& type)
{
	if(type.declaration->specifiers.isTypedef
		&& type.declaration->templateParameter == INDEX_INVALID)
	{
		return getUnderlyingType(type.declaration->type);
	}
	return type;
}


inline bool isEqual(const TypeId& l, const TypeId& r)
{
	UniqueTypeWrapper left = makeUniqueType(l, NAME_NULL, 0, true);
	UniqueTypeWrapper right = makeUniqueType(r, NAME_NULL, 0, true);
	return left == right;
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

// ----------------------------------------------------------------------------
// implicit conversion sequence

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
inline bool isBaseOf(const TypeInstance& type, const TypeInstance& other, Location source)
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
	instantiateClass(other, source);
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
		return UniqueTypeWrapper(pushUniqueType(gUniqueTypes, getInner(from.value), PointerType())); // T[] -> T*
	}
	if(to.isFunctionPointer()
		&& from.isFunction())
	{
		return UniqueTypeWrapper(pushUniqueType(gUniqueTypes, from.value, PointerType())); // T() -> T(*)()
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

inline StandardConversionSequence makeScsConversion(Location source, UniqueTypeId to, UniqueTypeId from, bool isNullPointerConstant = false) // TODO: detect null pointer constant
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
		&& isBaseOf(getObjectType(getInner(to.value)), getObjectType(getInner(from.value)), source))
	{
		to = UniqueTypeWrapper(getInner(to.value));
		from = UniqueTypeWrapper(getInner(from.value));
		return isEqualCvQualification(to, from) || isGreaterCvQualification(to, from)
			? StandardConversionSequence(SCSRANK_CONVERSION, makeQualificationAdjustment(to, from))
			: STANDARDCONVERSIONSEQUENCE_INVALID; // D* -> B*
	}
	if(to.isMemberPointer()
		&& from.isMemberPointer()
		&& isBaseOf(getMemberPointerClass(to.value), getMemberPointerClass(from.value), source))
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
		&& isBaseOf(getObjectType(to.value), getObjectType(from.value), source))
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
inline StandardConversionSequence makeStandardConversionSequence(UniqueTypeWrapper to, UniqueTypeWrapper from, Location source, bool isNullPointerConstant = false, bool isLvalue = false)
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
				&& isBaseOf(getObjectType(to.value), getObjectType(from.value), source))
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
		StandardConversionSequence result = makeScsConversion(source, to, from, isNullPointerConstant); // TODO: ordering of conversions by inheritance distance
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
	StandardConversionSequence sequence = makeStandardConversionSequence(to, from, NAME_NULL, isNullPointerConstant, isLvalue);
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


inline const char* getDeclarationType(const Declaration& declaration)
{
	if(isNamespace(declaration))
	{
		return "namespace";
	}
	if(isType(declaration))
	{
		return declaration.isTemplate ? "templateName" : "type";
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
			return second; // typedef of type previously declared: typedef struct S {} S;
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
				return second; // redeclaration of previously-declared class
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
		return second; // multiple declarations allowed
	}
	// HACK: ignore multiple declarations for members of template - e.g. const char Tmpl<char>::VALUE; const int Tmpl<int>::VALUE;
	if(!first.templateParams.defaults.empty())
	{
		// if enclosing is a template
		return first;
	}
	throw DeclarationError("symbol already defined");
}




inline const DeclarationInstance* findDeclaration(Scope::Declarations& declarations, const Identifier& id, LookupFilter filter = IsAny())
{
	Scope::Declarations::iterator i = declarations.upper_bound(id.value);

	for(; i != declarations.begin()
		&& (*--i).first == id.value;)
	{
		if(filter(*(*i).second))
		{
			return &(*i).second;
		}
	}

	return 0;
}

// find a declaration within a scope.
inline LookupResult findDeclaration(Scope& scope, const Identifier& id, LookupFilter filter = IsAny())
{
	LookupResult result;
	result.append(::findDeclaration(scope.declarations, id, filter));
	return result;
}

inline LookupResult findNamespaceDeclaration(Scope& scope, const Identifier& id, LookupFilter filter = IsAny());


// find a declaration within the set of using-directives present in a namespace
inline LookupResult findDeclaration(Scope::Scopes& scopes, const Identifier& id, LookupFilter filter = IsAny())
{
	LookupResult result;
	for(Scope::Scopes::iterator i = scopes.begin(); i != scopes.end(); ++i)
	{
		Scope& scope = *(*i);

#ifdef LOOKUP_DEBUG
		std::cout << "searching '";
		printName(scope);
		std::cout << "'" << std::endl;
#endif

		if(result.append(findNamespaceDeclaration(scope, id, filter)))
		{
			return result;
		}
	}
	return result;
}

// find a declaration within a namespace scope. Does not search enclosing scopes.
inline LookupResult findNamespaceDeclaration(Scope& scope, const Identifier& id, LookupFilter filter)
{
	SYMBOLS_ASSERT(scope.type == SCOPETYPE_NAMESPACE);
	LookupResult result;
	if(result.append(::findDeclaration(scope, id, filter)))
	{
		return result;
	}
	if(result.append(::findDeclaration(scope.usingDirectives, id, filter)))
	{
		return result;
	}
	return result;
}

inline LookupResult findMemberDeclaration(Scope& scope, const Identifier& id, LookupFilter filter)
{
	LookupResult result;
	if(result.append(findDeclaration(scope, id, filter)))
	{
		return result;
	}
	if(scope.type != SCOPETYPE_CLASS)
	{
		return result;
	}
	for(Types::iterator i = scope.bases.begin(); i != scope.bases.end(); ++i)
	{
		SYMBOLS_ASSERT((*i).unique != 0);
		if((*i).isDependent) // if base class is dependent
		{
			continue;
		}
		const TypeInstance& base = getObjectType((*i).unique);

		// an identifier looked up in the context of a class may name a base class
		if(base.declaration->getName().value == id.value
			&& filter(*base.declaration))
		{
			result.filtered = base.declaration->getName().dec.p;
			return result;
		}
		Scope* scope = base.declaration->enclosed;
		if(scope != 0)
		{
			// [namespace.udir] A using-directive shall not appear in class scope, but may appear in namespace scope or in block scope.
			SYMBOLS_ASSERT(scope->usingDirectives.empty());
			if(result.append(findMemberDeclaration(*scope, id, filter)))
			{
				return result;
			}
		}
	}
	return result;
}

// find a declaration within a class or namespace
inline LookupResult findClassOrNamespaceMemberDeclaration(Scope& scope, const Identifier& id, LookupFilter filter = IsAny())
{
#ifdef LOOKUP_DEBUG
	std::cout << "searching '";
	printName(scope);
	std::cout << "'" << std::endl;
#endif

	LookupResult result;
	if(result.append(findMemberDeclaration(scope, id, filter)))
	{
		return result;
	}
	if(scope.parent != 0)
	{
		if(result.append(findClassOrNamespaceMemberDeclaration(*scope.parent, id, filter)))
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


typedef std::list<UniqueType> TypeElements;


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
			if(scope->type != SCOPETYPE_TEMPLATE)
			{
				printer.out << getValue(scope->name) << ".";
			}
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

	std::vector<CvQualifiers> qualifierStack;

	void visit(const DependentType& dependent)
	{
#if 1
		printer.out << "$T" << dependent.type->scope->templateDepth << "_" << dependent.type->templateParameter;
#else
		printName(dependent.type);
#endif
		visitTypeElement();
	}
	void visit(const DependentTypename& dependent)
	{
		// TODO
		visitTypeElement();
	}
	void visit(const ObjectType& object)
	{
		if(qualifierStack.back().isConst)
		{
			printer.out << "const ";
		}
		if(qualifierStack.back().isVolatile)
		{
			printer.out << "volatile ";
		}
		printName(object.type.declaration);
		if(object.type.declaration->isTemplate)
		{
			printTemplateArguments(object.type.templateArguments);
		}
		visitTypeElement();
	}
	void visit(const ReferenceType& pointer)
	{
		pushType(true);
		printer.out << "&";
		visitTypeElement();
		popType();
	}
	void visit(const PointerType& pointer)
	{
		pushType(true);
		printer.out << "*";
		if(qualifierStack.back().isConst)
		{
			printer.out << " const";
		}
		if(qualifierStack.back().isVolatile)
		{
			printer.out << " volatile";
		}
		visitTypeElement();
		popType();
	}
	void visit(const ArrayType& array)
	{
		pushType(false);
		visitTypeElement();
		printer.out << "[";
		if(array.size != 0)
		{
			printer.out << array.size;
		}
		printer.out << "]";
		popType();
	}
	void visit(const MemberPointerType& pointer)
	{
		pushType(true);
		if(pointer.instance == 0)
		{
			printer.out << "$dependent";
		}
		else
		{
			printName(pointer.instance->declaration);
		}
		printer.out << "::*";
		if(qualifierStack.back().isConst)
		{
			printer.out << " const";
		}
		if(qualifierStack.back().isVolatile)
		{
			printer.out << " volatile";
		}
		visitTypeElement();
		popType();
	}
	void visit(const FunctionType& function)
	{
		pushType(false);
		visitTypeElement();
		printParameters(function.parameterTypes);
		if(qualifierStack.back().isConst)
		{
			printer.out << " const";
		}
		if(qualifierStack.back().isVolatile)
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
			UniqueType element = typeElements.front();
			typeElements.pop_front();
			qualifierStack.push_back(element);
			element->accept(*this);
			qualifierStack.pop_back();
		}
	}


	void printType(const Type& type)
	{
		printType(makeUniqueType(type, NAME_NULL, 0, true));
	}

	void printType(const Declaration& declaration)
	{
		printType(makeUniqueType(declaration.type, NAME_NULL, 0, true));
	}

	void printType(const UniqueTypeId& type)
	{
		for(UniqueType i = type.value; i != UNIQUETYPE_NULL; i = i->next)
		{
			typeElements.push_front(i);
		}
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
	void printParameters(const ParameterTypes& parameters)
	{
		printer.out << "(";
		bool separator = false;
		for(ParameterTypes::const_iterator i = parameters.begin(); i != parameters.end(); ++i)
		{
			if(separator)
			{
				printer.out << ",";
			}
			SymbolPrinter walker(printer);
			walker.printType(*i);
			separator = true;
		}
		printer.out << ")";
	}

	void printTemplateArguments(const TemplateArgumentsInstance& templateArguments)
	{
		printer.out << "{";
		bool separator = false;
		for(TemplateArgumentsInstance::const_iterator i = templateArguments.begin(); i != templateArguments.end(); ++i)
		{
			if(separator)
			{
				printer.out << ",";
			}
			SymbolPrinter walker(printer);
			walker.printType(*i);
			separator = true;
		}
		printer.out << "}";
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

inline void printType(const TypeInstance& type)
{
	FileTokenPrinter tokenPrinter(std::cout);
	SymbolPrinter printer(tokenPrinter);
	printer.printName(type.declaration);
	if(type.declaration->isTemplate)
	{
		printer.printTemplateArguments(type.templateArguments);
	}
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
	Location source;
	CandidateFunction best;
	Declaration* ambiguous;

	OverloadResolver(const UniqueTypeIds& arguments, Location source)
		: arguments(arguments), source(source), ambiguous(0)
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
	void addSingle(Declaration* declaration, const TypeInstance* enclosingType)
	{
		CandidateFunction candidate(declaration);
		candidate.conversions.reserve(best.conversions.size());

		UniqueTypeWrapper type = makeUniqueType(declaration->type, source, enclosingType, true); // TODO: dependent types, template argument deduction
		if(!type.isFunction())
		{
			return; // TODO: invoke operator() on object of class-type
		}
		const ParameterTypes& parameters = getParameterTypes(type.value);
		UniqueTypeIds::const_iterator a = arguments.begin();
		Parameters::const_iterator p = getParameters(declaration->type).begin();
		// TODO: ellipsis
		for(ParameterTypes::const_iterator i = parameters.begin(); i != parameters.end(); ++i)
		{
			UniqueTypeId to = (*i);
			UniqueTypeId from;
			if(a != arguments.end())
			{
				candidate.conversions.push_back(makeStandardConversionSequence(to, *a, source)); // TODO: null-pointer-constant, l-value
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

		add(candidate);
	}
	void add(Declaration* declaration, const TypeInstance* enclosingType = 0)
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

			addSingle(p, enclosingType);
		}
	}
};

// source: where the overload resolution occurs (point of instantiation)
// enclosingType: the class of which the declaration is a member (along with all its overloads).
inline Declaration* findBestMatch(Declaration* declaration, const UniqueTypeIds& arguments, Location source, const TypeInstance* enclosingType)
{
	OverloadResolver resolver(arguments, source);
	resolver.add(declaration, enclosingType);

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


