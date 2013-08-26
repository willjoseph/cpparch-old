
#ifndef INCLUDED_CPPPARSE_AST_TYPE_H
#define INCLUDED_CPPPARSE_AST_TYPE_H

#include "Declaration.h"
#include "Scope.h"
#include "Expression.h" // IntegralConstant



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


// ----------------------------------------------------------------------------
// built-in symbols

#define TYPE_ARITHMETIC TypeId(&gArithmetic, AST_ALLOCATOR_NULL)
#define TYPE_SPECIAL TypeId(&gSpecial, AST_ALLOCATOR_NULL)
#define TYPE_CLASS TypeId(&gClass, AST_ALLOCATOR_NULL)
#define TYPE_ENUM TypeId(&gEnum, AST_ALLOCATOR_NULL)

struct BuiltInTypeDeclaration : Declaration
{
	BuiltInTypeDeclaration(Identifier& name, const TypeId& type = TYPE_ARITHMETIC)
		: Declaration(AST_ALLOCATOR_NULL, 0, name, type, 0)
	{
	}
};

#define TYPE_NAMESPACE TypeId(&gNamespace, AST_ALLOCATOR_NULL)
#define TYPE_CTOR TypeId(&gCtor, AST_ALLOCATOR_NULL)
#define TYPE_ENUMERATOR TypeId(&gEnumerator, AST_ALLOCATOR_NULL)
#define TYPE_UNKNOWN TypeId(&gUnknown, AST_ALLOCATOR_NULL)

extern Declaration gDependentType;
extern const DeclarationInstance gDependentTypeInstance;
extern Declaration gDependentObject;
extern const DeclarationInstance gDependentObjectInstance;
extern Declaration gDependentTemplate;
extern const DeclarationInstance gDependentTemplateInstance;
extern Declaration gDependentNested;
extern const DeclarationInstance gDependentNestedInstance;
extern Declaration gDependentNestedTemplate;
extern const DeclarationInstance gDependentNestedTemplateInstance;

extern Declaration gParam;
extern Declaration gNonType;


#define TYPE_PARAM TypeId(&gParam, AST_ALLOCATOR_NULL)

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


// returns the most recent declaration that is not a redeclaration
inline Declaration* findOverloaded(const DeclarationInstance& instance)
{
	for(const DeclarationInstance* p = &instance; p != 0; p = p->overloaded)
	{
		if((*p).redeclared == 0)
		{
			return *p;
		}
	}
	return 0;
}


// ----------------------------------------------------------------------------
// unique types
// Representation of a declarator, with type-elements linked in 'normal' order.
// e.g. int(*)[] == pointer to array of == DeclaratorPointerType -> DeclaratorArrayType
// Note that this is the reverse of the order that the declarator is parsed in.
// This means a given unique type sub-sequence need only be stored once.
// This allows fast comparison of types and simplifies printing of declarators.

struct TypeElementVisitor
{
#if 0
	virtual void visit(const struct Namespace&) = 0;
#endif
	virtual void visit(const struct DependentType&) = 0;
	virtual void visit(const struct DependentTypename&) = 0;
	virtual void visit(const struct DependentNonType&) = 0;
	virtual void visit(const struct TemplateTemplateArgument&) = 0;
	virtual void visit(const struct NonType&) = 0;
	virtual void visit(const struct SimpleType&) = 0;
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
	bool operator<(const TypeElementGeneric& other) const
	{
		return value < other.value;
	}
	bool operator<(const TypeElement& other) const
	{
		return next != other.next
			? next < other.next
			: abstractLess(*this, other);
	}
};

const UniqueType UNIQUETYPE_NULL = &gTypeElementEmpty;


typedef IndirectSet<UniqueType> UniqueTypes;

extern UniqueTypes gBuiltInTypes;

template<typename T>
inline UniqueType pushBuiltInType(UniqueType type, const T& value)
{
	TypeElementGeneric<T> node(value);
	node.next = type;
	return *gBuiltInTypes.insert(node);
}

template<typename T>
inline UniqueType pushUniqueType(UniqueTypes& types, UniqueType type, const T& value)
{
	TypeElementGeneric<T> node(value);
	node.next = type;
	{
		UniqueTypes::iterator i = gBuiltInTypes.find(node);
		if(i != gBuiltInTypes.end())
		{
			return *i;
		}
	}
	return *types.insert(node);
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
		return typeid(*value) == typeid(TypeElementGeneric<SimpleType>);
	}
#if 0
	bool isNamespace() const
	{
		return typeid(*value) == typeid(TypeElementGeneric<struct Namespace>);
	}
#endif
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
	bool isDependentNonType() const
	{
		return typeid(*value) == typeid(TypeElementGeneric<DependentNonType>);
	}
	bool isDependentType() const
	{
		return typeid(*value) == typeid(TypeElementGeneric<DependentType>);
	}
	bool isDependent() const
	{
		return isDependentType()
			|| typeid(*value) == typeid(TypeElementGeneric<DependentTypename>)
			|| isDependentNonType();
	}
	bool isNonType() const
	{
		return typeid(*value) == typeid(TypeElementGeneric<NonType>);
	}
	bool isTemplateTemplateArgument() const
	{
		return typeid(*value) == typeid(TypeElementGeneric<TemplateTemplateArgument>);
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

template<typename T>
inline UniqueTypeWrapper pushType(UniqueTypeWrapper type, const T& t)
{
	pushUniqueType(type.value, t);
	return type;
}

template<typename T>
inline UniqueTypeWrapper pushBuiltInType(UniqueTypeWrapper type, const T& value)
{
	return UniqueTypeWrapper(pushBuiltInType(type.value, value));
}

inline UniqueTypeWrapper popType(UniqueTypeWrapper type)
{
	type.pop_front();
	return type;
}

inline UniqueTypeWrapper qualifyType(UniqueTypeWrapper type, CvQualifiers qualifiers)
{
	type.value.setQualifiers(qualifiers);
	return type;
}

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

inline bool isGreaterCvQualification(CvQualifiers l, CvQualifiers r)
{
	return l.isConst + l.isVolatile > r.isConst + r.isVolatile;
}

inline bool isGreaterCvQualification(UniqueTypeWrapper to, UniqueTypeWrapper from)
{
	return isGreaterCvQualification(to.value.getQualifiers(), from.value.getQualifiers());
}

inline bool isEqualCvQualification(UniqueTypeWrapper to, UniqueTypeWrapper from)
{
	return to.value.getQualifiers() == from.value.getQualifiers();
}

// ----------------------------------------------------------------------------
struct Parameter
{
	DeclarationPtr declaration;
	cpp::default_argument* defaultArgument;
	Parameter(Declaration* declaration, cpp::default_argument* defaultArgument)
		: declaration(declaration), defaultArgument(defaultArgument)
	{
	}
};

struct Parameters : std::vector<Parameter>
{
	bool isEllipsis;
	Parameters() : isEllipsis(false)
	{
	}
};


// ----------------------------------------------------------------------------

typedef UniqueTypeWrapper UniqueTypeId;

const UniqueTypeId gUniqueTypeNull = UniqueTypeId(UNIQUETYPE_NULL);

inline bool isEqual(const UniqueTypeId& l, const UniqueTypeId& r)
{
	return l.value == r.value;
}

inline UniqueType getInner(UniqueType type)
{
	SYMBOLS_ASSERT(typeid(*type) != typeid(TypeElementGeneric<struct SimpleType>));
	return type->next;
}

inline bool isEqualInner(const UniqueTypeId& l, const UniqueTypeId& r)
{
	return getInner(l.value) == getInner(r.value);
}

inline bool isSameType(const UniqueTypeId& l, const UniqueTypeId& r)
{
	return typeid(*l.value) == typeid(*r.value);
}


struct NonType : IntegralConstant
{
	explicit NonType(IntegralConstant value)
		: IntegralConstant(value)
	{
	}
};

// ignoring type when comparing non-type template parameter
inline bool operator<(const NonType& left, const NonType& right)
{
	return left.value < right.value;
}

inline const NonType& getNonTypeValue(UniqueType type)
{
	SYMBOLS_ASSERT(typeid(*type) == typeid(TypeElementGeneric<NonType>));
	return static_cast<const TypeElementGeneric<NonType>*>(type.getPointer())->value;
}


// 14.4 Type equivalence [temp.type]
// Two template-ids refer to the same class or function if their template names are identical, they refer to the
// same template, their type template-arguments are the same type, their non-type template-arguments of integral
// or enumeration type have identical values, their non-type template-arguments of pointer or reference
// type refer to the same external object or function, and their template template-arguments refer to the same
// template.

typedef std::vector<UniqueTypeWrapper> UniqueTypeArray;
typedef UniqueTypeArray TemplateArgumentsInstance;
typedef UniqueTypeArray InstantiatedTypes;
typedef std::vector<const struct SimpleType*> UniqueBases;

struct ChildInstantiation
{
	const struct SimpleType* instance;
	Location source;
	ChildInstantiation(const struct SimpleType* instance, Location source)
		: instance(instance), source(source)
	{
	}
};
typedef std::vector<ChildInstantiation> ChildInstantiations;


typedef std::vector<Location> InstanceLocations; // temporary scaffolding!


struct SimpleType
{
	std::size_t uniqueId;
	DeclarationPtr primary;
	DeclarationPtr declaration; // don't compare declaration pointer - it will change on instantiation if an explicit/partial specialization is chosen
	TemplateArgumentsInstance templateArguments;
	TemplateArgumentsInstance deducedArguments; // the deduced arguments for the partial-specialization's template-parameters
	const SimpleType* enclosing; // the enclosing template
	UniqueBases bases;
	size_t size;
	InstantiatedTypes children; // the dependent types in the specialization
	InstanceLocations childLocations; // temporary scaffolding!
	bool instantiated;
	bool instantiating;
	bool allowLookup;
	mutable bool visited; // used during findDeclaration to prevent infinite recursion
	mutable bool dumped; // used during dumpTemplateInstantiations to prevent duplicates
	Location instantiation;
	ChildInstantiations childInstantiations;

	SimpleType(Declaration* declaration, const SimpleType* enclosing)
		: uniqueId(0), primary(declaration), declaration(declaration), enclosing(enclosing), size(0), instantiated(false), instantiating(false), allowLookup(false), visited(false), dumped(false)
	{
		SYMBOLS_ASSERT(enclosing == 0 || isClass(*enclosing->declaration));
	}
};

inline bool operator==(const SimpleType& left, const SimpleType& right)
{
	return left.primary.p == right.primary.p
		&& left.enclosing == right.enclosing
		&& left.templateArguments == right.templateArguments;
}

inline bool operator<(const SimpleType& left, const SimpleType& right)
{
	return left.primary.p != right.primary.p ? left.primary.p < right.primary.p
		: left.enclosing != right.enclosing ? left.enclosing < right.enclosing
		: left.templateArguments != right.templateArguments ? left.templateArguments < right.templateArguments
		: false;
}

inline const SimpleType& getSimpleType(UniqueType type)
{
	SYMBOLS_ASSERT(typeid(*type) == typeid(TypeElementGeneric<SimpleType>));
	return static_cast<const TypeElementGeneric<SimpleType>*>(type.getPointer())->value;
}

#if 0
struct Namespace
{
	DeclarationPtr declaration;
	Namespace(DeclarationPtr declaration)
		: declaration(declaration)
	{
	}
};

inline const Namespace& getNamespace(UniqueType type)
{
	SYMBOLS_ASSERT(typeid(*type) == typeid(TypeElementGeneric<Namespace>));
	return static_cast<const TypeElementGeneric<Namespace>*>(type.getPointer())->value;
}


inline bool operator<(const Namespace& left, const Namespace& right)
{
	return left.declaration.p < right.declaration.p;
}
#endif

struct TemplateTemplateArgument
{
	DeclarationPtr declaration; // the primary declaration of the template template argument
	const SimpleType* enclosing;
	TemplateTemplateArgument(Declaration* declaration, const SimpleType* enclosing)
		: declaration(declaration), enclosing(enclosing)
	{
	}
};

inline bool operator<(const TemplateTemplateArgument& left, const TemplateTemplateArgument& right)
{
	return left.declaration.p < right.declaration.p;
}

inline const TemplateTemplateArgument& getTemplateTemplateArgument(UniqueType type)
{
	SYMBOLS_ASSERT(typeid(*type) == typeid(TypeElementGeneric<TemplateTemplateArgument>));
	return static_cast<const TypeElementGeneric<TemplateTemplateArgument>*>(type.getPointer())->value;
}


// represents a template-parameter (or template-template-parameter)
// T
// TT
// TT<> (TODO)
// TT<T>
struct DependentType
{
	DeclarationPtr type; // the declaration of the template parameter
	TemplateArgumentsInstance templateArguments;
	std::size_t templateParameterCount;
	DependentType(Declaration* type, TemplateArgumentsInstance templateArguments = TemplateArgumentsInstance(), std::size_t templateParameterCount = 0)
		: type(type), templateArguments(templateArguments), templateParameterCount(templateParameterCount)
	{
		SYMBOLS_ASSERT(type->templateParameter != INDEX_INVALID);
	}
};

inline bool operator<(const DependentType& left, const DependentType& right)
{
	return left.templateParameterCount != right.templateParameterCount
		? left.templateParameterCount < right.templateParameterCount
		: left.type->scope->templateDepth != right.type->scope->templateDepth
		? left.type->scope->templateDepth < right.type->scope->templateDepth
		: left.type->templateParameter != right.type->templateParameter
		? left.type->templateParameter < right.type->templateParameter
		: left.templateArguments < right.templateArguments;
}

inline const DependentType& getDependentType(UniqueType type)
{
	SYMBOLS_ASSERT(typeid(*type) == typeid(TypeElementGeneric<DependentType>));
	return static_cast<const TypeElementGeneric<DependentType>*>(type.getPointer())->value;
}


struct DependentTypename
{
	Name name; // the type name
	UniqueTypeWrapper qualifying; // the qualifying type: T::, C<T>::
	TemplateArgumentsInstance templateArguments;
	bool isNested; // T::dependent::
	bool isTemplate; // T::template dependent<>
	DependentTypename(Name name, UniqueTypeWrapper qualifying, TemplateArgumentsInstance templateArguments, bool isNested, bool isTemplate)
		: name(name), qualifying(qualifying), templateArguments(templateArguments), isNested(isNested), isTemplate(isTemplate)
	{
	}
};

inline bool operator<(const DependentTypename& left, const DependentTypename& right)
{
	return left.name != right.name
		? left.name < right.name
		: left.qualifying != right.qualifying
		? left.qualifying < right.qualifying
		: left.isNested != right.isNested
		? left.isNested < right.isNested
		: left.isTemplate != right.isTemplate
		? left.isTemplate < right.isTemplate
		: left.templateArguments < right.templateArguments;
}


struct DependentNonType
{
	UniqueExpression expression;
	DependentNonType(UniqueExpression expression)
		: expression(expression)
	{
	}
};

inline bool operator<(const DependentNonType& left, const DependentNonType& right)
{
	return left.expression < right.expression;
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
	UniqueTypeWrapper type;
	MemberPointerType(UniqueTypeWrapper type)
		: type(type)
	{
	}
};

inline const MemberPointerType& getMemberPointerType(UniqueType type)
{
	SYMBOLS_ASSERT(typeid(*type) == typeid(TypeElementGeneric<MemberPointerType>));
	return static_cast<const TypeElementGeneric<MemberPointerType>*>(type.getPointer())->value;
}

inline const SimpleType& getMemberPointerClass(UniqueType type)
{
	return getSimpleType(getMemberPointerType(type).type.value);
}

inline bool operator<(const MemberPointerType& left, const MemberPointerType& right)
{
	return left.type < right.type;
}

typedef std::vector<ExpressionWrapper> ArrayRank;

struct DeclaratorArrayType
{
	ArrayRank rank; // store expressions to be evaluated when template-params are known
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
	DeclaratorFunctionType()
	{
	}
	DeclaratorFunctionType(const Parameters& parameters, CvQualifiers qualifiers)
		: parameters(parameters), qualifiers(qualifiers)
	{
	}
};

typedef UniqueTypeArray ParameterTypes;

struct FunctionType
{
	ParameterTypes parameterTypes;
	bool isEllipsis;
	FunctionType()
		: isEllipsis(false)
	{
	}
};

inline bool operator<(const FunctionType& left, const FunctionType& right)
{
	return left.isEllipsis != right.isEllipsis ? left.isEllipsis < right.isEllipsis
		: left.parameterTypes < right.parameterTypes;
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
inline const DeclaratorFunctionType& getDeclaratorFunctionType(const TypeSequence::Node* node)
{
	SYMBOLS_ASSERT(node != 0);
	SYMBOLS_ASSERT(typeid(*node) == typeid(SequenceNodeGeneric<DeclaratorFunctionType, TypeSequenceVisitor>));
	return static_cast<const SequenceNodeGeneric<DeclaratorFunctionType, TypeSequenceVisitor>*>(node)->value;
}
inline const Parameters& getParameters(const TypeId& type)
{
	const TypeSequence::Node* node = getLastNode(type.typeSequence);
	return getDeclaratorFunctionType(node).parameters;
}

inline const FunctionType& getFunctionType(UniqueType type)
{
	SYMBOLS_ASSERT(typeid(*type) == typeid(TypeElementGeneric<FunctionType>));
	return static_cast<const TypeElementGeneric<FunctionType>*>(type.getPointer())->value;
}
inline const ParameterTypes& getParameterTypes(UniqueType type)
{
	return getFunctionType(type).parameterTypes;
}



// ----------------------------------------------------------------------------


struct InstantiationContext
{
	Location source;
	const SimpleType* enclosingType;
	ScopePtr enclosingScope;
	InstantiationContext(Location source, const SimpleType* enclosingType, ScopePtr enclosingScope)
		: source(source), enclosingType(enclosingType), enclosingScope(enclosingScope)
	{
	}
};

inline InstantiationContext setEnclosingType(const InstantiationContext& context, const SimpleType* enclosingType)
{
	return InstantiationContext(context.source, enclosingType, context.enclosingScope);
}

inline InstantiationContext setEnclosingTypeSafe(const InstantiationContext& context, const SimpleType* enclosingType)
{
	SYMBOLS_ASSERT(enclosingType != 0);
	return setEnclosingType(context, enclosingType);
}


inline UniqueTypeWrapper makeUniqueSimpleType(const SimpleType& type)
{
	SYMBOLS_ASSERT(!(type.primary->isTemplate && isSpecialization(*type.primary))); // primary declaration must not be a specialization!
	return UniqueTypeWrapper(pushUniqueType(gUniqueTypes, UNIQUETYPE_NULL, type));
}

// ----------------------------------------------------------------------------

template<bool builtIn>
struct UniqueTypeGeneric : UniqueTypeWrapper
{
	UniqueTypeGeneric()
	{
	}
	explicit UniqueTypeGeneric(UniqueTypeWrapper value) : UniqueTypeWrapper(value)
	{
	}
};


typedef UniqueTypeGeneric<true> BuiltInType; 

struct BuiltInTypeId : BuiltInType
{
	BuiltInTypeId(Declaration* declaration, const AstAllocator<int>& allocator)
	{
		value = pushBuiltInType(value, SimpleType(declaration, 0));
		declaration->type.unique = value;
		declaration->isComplete = true;
	}
};



inline bool isDependent(UniqueTypeWrapper type);


inline bool isDependent(const UniqueTypeArray& types)
{
	for(UniqueTypeArray::const_iterator i = types.begin(); i != types.end(); ++i)
	{
		if(isDependent(*i))
		{
			return true;
		}
	}
	return false;
}


inline bool isDependent(const SimpleType& type)
{
	if(type.enclosing
		&& isDependent(*type.enclosing))
	{
		return true;
	}
	if(isDependent(type.templateArguments))
	{
		return true;
	}
	return false;
}


struct IsDependentVisitor : TypeElementVisitor
{
	bool result;
	IsDependentVisitor()
		: result(false)
	{
	}
#if 0
	virtual void visit(const Namespace& element)
	{
	}
#endif
	virtual void visit(const DependentType&)
	{
		result = true;
	}
	virtual void visit(const DependentTypename&)
	{
		result = true;
	}
	virtual void visit(const DependentNonType&)
	{
		result = true;
	}
	virtual void visit(const TemplateTemplateArgument&)
	{
	}
	virtual void visit(const NonType&)
	{
	}
	virtual void visit(const SimpleType& element)
	{
		if(isDependent(element))
		{
			result = true;
		}
	}
	virtual void visit(const PointerType&)
	{
	}
	virtual void visit(const ReferenceType&)
	{
	}
	virtual void visit(const ArrayType&)
	{
	}
	virtual void visit(const MemberPointerType& element)
	{
		if(isDependent(element.type))
		{
			result = true;
		}
	}
	virtual void visit(const FunctionType& element)
	{
		if(isDependent(element.parameterTypes))
		{
			result = true;
		}
	}
};


inline bool isDependent(UniqueTypeWrapper type)
{
	for(UniqueTypeWrapper i = type; !i.empty(); i.pop_front())
	{
		IsDependentVisitor visitor;
		i.value->accept(visitor);
		if(visitor.result)
		{
			return true;
		}
	}
	return false;
}



inline const SimpleType* findEnclosingType(const SimpleType& enclosing, Scope* scope)
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
		const SimpleType* result = findEnclosingType(*(*i), scope);
		if(result != 0)
		{
			return result;
		}
	}
	return 0;
}

inline const SimpleType* findEnclosingType(const SimpleType* enclosing, Scope* scope)
{
	SYMBOLS_ASSERT(scope != 0);
	for(const SimpleType* i = enclosing; i != 0; i = (*i).enclosing)
	{
		const SimpleType* result = findEnclosingType(*i, scope);
		if(result != 0)
		{
			return result;
		}
	}
	return 0;
}


inline const SimpleType* findEnclosingTemplate(const SimpleType* enclosing, Scope* scope)
{
	SYMBOLS_ASSERT(scope != 0);
	SYMBOLS_ASSERT(scope->type == SCOPETYPE_TEMPLATE);
	for(const SimpleType* i = enclosing; i != 0; i = (*i).enclosing)
	{
		if((*i).declaration->templateParamScope != 0
			&& (*i).declaration->templateParamScope->templateDepth == scope->templateDepth)
		{
			return i;
		}
	}
	return 0;
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

#endif
