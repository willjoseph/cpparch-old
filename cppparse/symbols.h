
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
struct SequenceNode : Visitor::Visitable
{
	Reference<SequenceNode> next;
	virtual ~SequenceNode()
	{
	}
	virtual void accept(Visitor& visitor) const
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
		visitor.visit(value, next.get());
	}
};

template<typename A, typename Visitor>
struct Sequence : A
{
	typedef SequenceNode<Visitor> Node;
	typedef Reference<Node> Pointer;
	Node head;

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
		return head.next == 0;
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

	void swap(Sequence& other)
	{
		head.next.swap(other.head.next);
	}

	const typename Visitor::Visitable* get() const
	{
		return head.next.get();
	}
};

// ----------------------------------------------------------------------------
// type sequence


struct TypeElementVisitor
{
	struct Visitable
	{
		virtual void accept(TypeElementVisitor& visitor) const = 0;
	};

	virtual void visit(const struct DeclaratorPointer&, const Visitable* next) = 0;
	virtual void visit(const struct DeclaratorArray&, const Visitable* next) = 0;
	virtual void visit(const struct DeclaratorMemberPointer&, const Visitable* next) = 0;
	virtual void visit(const struct DeclaratorFunction&, const Visitable* next) = 0;
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

struct Type;

typedef Reference<const Type> TypePtr;

typedef CopiedReference<const Type, TreeAllocator<int> > TypeRef;

typedef TypeRef Qualifying;


class Declaration;
typedef SafePtr<Declaration> DeclarationPtr;

struct Type
{
	DeclarationPtr declaration;
	TemplateArguments templateArguments; // may be non-empty if this is a template
	Qualifying qualifying;
	bool isImplicitTemplateId; // true if this is a template but the template-argument-clause has not been specified
	mutable bool visited; // use while iterating a set of types, to avoid visiting the same type twice (an optimisation, and a mechanism for handling cyclic dependencies)
	Type(Declaration* declaration, const TreeAllocator<int>& allocator)
		: declaration(declaration), templateArguments(allocator), qualifying(allocator), isImplicitTemplateId(false), visited(false)
	{
	}
	void swap(Type& other)
	{
		std::swap(declaration, other.declaration);
		templateArguments.swap(other.templateArguments);
		qualifying.swap(other.qualifying);
		std::swap(isImplicitTemplateId, other.isImplicitTemplateId);
	}
	Type& operator=(Declaration* declaration)
	{
		SYMBOLS_ASSERT(templateArguments.empty());
		SYMBOLS_ASSERT(qualifying.empty());
		this->declaration = declaration;
		return *this;
	}
private:
	Type();
};

#define TYPE_NULL Type(0, TREEALLOCATOR_NULL)

const Type gTypeNull = TYPE_NULL;

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

const size_t INDEX_INVALID = size_t(-1);

class Declaration
{
	Identifier* name;

#if 0
	Declaration(const Declaration&);
	Declaration& operator=(const Declaration&);
#endif
public:
	Scope* scope;
	Type type;
	TypeSequence typeSequence;
	Scope* enclosed;
	Declaration* overloaded;
	Dependent valueDependent; // the dependent-types/names that are referred to in the declarator-suffix (array size)
	DeclSpecifiers specifiers;
	size_t templateParameter;
	Types templateParams;
	Types templateParamDefaults;
	TemplateArguments templateArguments;
	bool isTemplate;

	Declaration(
		const TreeAllocator<int>& allocator,
		Scope* scope,
		Identifier& name,
		const Type& type,
		Scope* enclosed,
		DeclSpecifiers specifiers = DeclSpecifiers(),
		bool isTemplate = false,
		const TemplateArguments& templateArguments = TEMPLATEARGUMENTS_NULL,
		size_t templateParameter = INDEX_INVALID,
		const Dependent& valueDependent = DEPENDENT_NULL
	) : scope(scope),
		name(&name),
		type(type),
		typeSequence(allocator),
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
		typeSequence(TREEALLOCATOR_NULL),
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

typedef SafePtr<Scope> ScopePtr;

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

	Scope(const TreeAllocator<int>& allocator, const Identifier& name, ScopeType type = SCOPETYPE_UNKNOWN)
		: parent(0), name(name), enclosedScopeCount(0), declarations(allocator), type(type), bases(allocator), usingDirectives(allocator), declarationList(allocator)
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


struct DeclaratorPointer
{
	bool isReference;
};

struct DeclaratorMemberPointer
{
	DeclarationPtr classDeclaration;
};

struct DeclaratorArray
{
	// TODO: size
};

struct DeclaratorFunction
{
	ScopePtr paramScope;
	DeclaratorFunction(Scope* scope)
		: paramScope(scope)
	{
	}
};


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
struct BuiltInTypeDeclaration : Declaration
{
	BuiltInTypeDeclaration(Identifier& name)
		: Declaration(TREEALLOCATOR_NULL, 0, name, TYPE_SPECIAL, 0)
	{
	}
};


extern Declaration gNamespace;

#define TYPE_NAMESPACE Type(&gNamespace, TREEALLOCATOR_NULL)

extern Declaration gCtor;
extern Declaration gEnumerator;
extern Declaration gUnknown;

#define TYPE_CTOR Type(&gCtor, TREEALLOCATOR_NULL)
#define TYPE_ENUMERATOR Type(&gEnumerator, TREEALLOCATOR_NULL)
#define TYPE_UNKNOWN Type(&gUnknown, TREEALLOCATOR_NULL)

// fundamental types
extern Type gChar;
extern Type gCharType;
extern Type gSignedChar;
extern Type gUnsignedChar;
extern Type gSignedShortInt;
extern Type gUnsignedShortInt;
extern Type gSignedInt;
extern Type gUnsignedInt;
extern Type gSignedLongInt;
extern Type gUnsignedLongInt;
extern Type gSignedLongLongInt;
extern Type gUnsignedLongLongInt;
extern Type gWCharT;
extern Type gBool;
extern Type gFloat;
extern Type gDouble;
extern Type gLongDouble;
extern Type gVoid;

struct StringLiteralDeclaration : Declaration
{
	StringLiteralDeclaration(Identifier& name, const Type& type, TypeSequence::Pointer node)
		: Declaration(TREEALLOCATOR_NULL, 0, name, type, 0, DECLSPEC_TYPEDEF) // TODO: const
	{
		typeSequence.head.next = node;
	}
};

extern Type gStringLiteral;
extern Type gWideStringLiteral;

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
	case MAKE_FUNDAMENTAL(CHAR): return gChar.declaration;
	case MAKE_FUNDAMENTAL(SIGNED) | MAKE_FUNDAMENTAL(CHAR): return gSignedChar.declaration;
	case MAKE_FUNDAMENTAL(UNSIGNED) | MAKE_FUNDAMENTAL(CHAR): return gUnsignedChar.declaration;
	case MAKE_FUNDAMENTAL(SHORT):
	case MAKE_FUNDAMENTAL(SIGNED) | MAKE_FUNDAMENTAL(SHORT):
	case MAKE_FUNDAMENTAL(SIGNED) | MAKE_FUNDAMENTAL(SHORT) | MAKE_FUNDAMENTAL(INT): return gSignedShortInt.declaration;
	case MAKE_FUNDAMENTAL(UNSIGNED) | MAKE_FUNDAMENTAL(SHORT):
	case MAKE_FUNDAMENTAL(UNSIGNED) | MAKE_FUNDAMENTAL(SHORT) | MAKE_FUNDAMENTAL(INT): return gUnsignedShortInt.declaration;
	case MAKE_FUNDAMENTAL(INT):
	case MAKE_FUNDAMENTAL(SIGNED) | MAKE_FUNDAMENTAL(INT): return gSignedInt.declaration;
	case MAKE_FUNDAMENTAL(UNSIGNED):
	case MAKE_FUNDAMENTAL(UNSIGNED) | MAKE_FUNDAMENTAL(INT): return gUnsignedInt.declaration;
	case MAKE_FUNDAMENTAL(LONG):
	case MAKE_FUNDAMENTAL(SIGNED) | MAKE_FUNDAMENTAL(LONG):
	case MAKE_FUNDAMENTAL(SIGNED) | MAKE_FUNDAMENTAL(LONG) | MAKE_FUNDAMENTAL(INT): return gSignedLongInt.declaration;
	case MAKE_FUNDAMENTAL(UNSIGNED) | MAKE_FUNDAMENTAL(LONG):
	case MAKE_FUNDAMENTAL(UNSIGNED) | MAKE_FUNDAMENTAL(LONG) | MAKE_FUNDAMENTAL(INT): return gUnsignedLongInt.declaration;
	case MAKE_FUNDAMENTAL(LONG) | MAKE_FUNDAMENTAL2(LONG):
	case MAKE_FUNDAMENTAL(SIGNED) | MAKE_FUNDAMENTAL(LONG) | MAKE_FUNDAMENTAL2(LONG):
	case MAKE_FUNDAMENTAL(SIGNED) | MAKE_FUNDAMENTAL(LONG) | MAKE_FUNDAMENTAL2(LONG) | MAKE_FUNDAMENTAL(INT): return gSignedLongLongInt.declaration;
	case MAKE_FUNDAMENTAL(UNSIGNED) | MAKE_FUNDAMENTAL(LONG) | MAKE_FUNDAMENTAL2(LONG):
	case MAKE_FUNDAMENTAL(UNSIGNED) | MAKE_FUNDAMENTAL(LONG) | MAKE_FUNDAMENTAL2(LONG) | MAKE_FUNDAMENTAL(INT): return gUnsignedLongLongInt.declaration;
	case MAKE_FUNDAMENTAL(WCHAR_T): return gWCharT.declaration;
	case MAKE_FUNDAMENTAL(BOOL): return gBool.declaration;
	case MAKE_FUNDAMENTAL(FLOAT): return gFloat.declaration;
	case MAKE_FUNDAMENTAL(DOUBLE): return gDouble.declaration;
	case MAKE_FUNDAMENTAL(LONG) | MAKE_FUNDAMENTAL(DOUBLE): return gLongDouble.declaration;
	case MAKE_FUNDAMENTAL(VOID): return gVoid.declaration;
	}
	return 0;
}

#define MAKE_INTEGERLITERALSUFFIX(token) (1 << cpp::simple_type_specifier_builtin::token)

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

inline const Type& getIntegerLiteralType(const char* value)
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

inline const Type& getFloatingLiteralType(const char* value)
{
	const char* suffix = getIntegerLiteralSuffix(value);
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

inline const Type& getCharacterLiteralType(const char* value)
{
	return *value == 'L' ? gWCharT : gSignedChar; // TODO: multicharacter literal
}

inline const Type& getNumericLiteralType(cpp::numeric_literal* symbol)
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

inline const Type& getStringLiteralType(cpp::string_literal* symbol)
{
	const char* value = symbol->value.value.c_str();
	return *value == 'L' ? gWideStringLiteral : gStringLiteral;
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
	TemplateArguments::const_iterator a = type.templateArguments.begin();
	Types::const_iterator p = type.declaration->templateParamDefaults.begin();
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
		SYMBOLS_ASSERT(p != type.declaration->templateParamDefaults.end());
		if(index == 0)
		{
			SYMBOLS_ASSERT((*p).declaration != 0);
			return *p;
		}
		++p;
	}
}

inline const Type& getInstantiatedSimpleType(const Type& type);

template<typename Inner>
inline const Type& getInstantiatedTypeGeneric(const Type& type, Inner inner)
{
	if(type.declaration->specifiers.isTypedef // if this is a typedef..
		&& type.declaration->templateParameter == INDEX_INVALID) // .. and not a template-parameter
	{
		const Type& original = inner(type.declaration->type);

		size_t index = original.declaration->templateParameter;
		if(index != INDEX_INVALID) // if the original type is a template-parameter.
		{
			// Find the template-specialisation it belongs to:
			for(const Type* i = type.qualifying.get(); i != 0; i = (*i).qualifying.get())
			{
				const Type& instantiated = getInstantiatedSimpleType(*i);
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

inline const Type& getInstantiatedType(const Type& type)
{
	return getInstantiatedTypeGeneric(type, getInstantiatedType);
}

inline bool isSimple(const Type& type)
{
	return type.declaration->typeSequence.empty();
}

// asserts that the resulting type is not a complex type. e.g. type-id found in nested-name-specifier
inline const Type& getInstantiatedSimpleType(const Type& type)
{
	SYMBOLS_ASSERT(isSimple(type));
	return getInstantiatedTypeGeneric(type, getInstantiatedSimpleType);
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
		return isEqual(primary->templateParamDefaults, left.templateArguments, right.templateArguments);
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
		if(!declaration->templateArguments.empty()
			&& isEqual(primary->templateParamDefaults, declaration->templateArguments, arguments))
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


