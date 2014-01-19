
#ifndef INCLUDED_CPPPARSE_AST_TYPE_H
#define INCLUDED_CPPPARSE_AST_TYPE_H

#include "Declaration.h"
#include "Scope.h"
#include "Expression.h" // IntegralConstant
#include "Common/Vector.h"



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
	return declaration.enclosed != 0 && declaration.enclosed->type == SCOPETYPE_FUNCTION;
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
// Returns the primary template for the given template declaration.
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


// Returns the most recent redeclaration of the given declaration.
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


// Returns the most recent redeclaration of the primary template for the given template declaration
inline const DeclarationInstance& findPrimaryTemplateLastDeclaration(const DeclarationInstance& instance)
{
	return findLastDeclaration(instance, findPrimaryTemplate(instance));
}


// Returns the most recently declared overload of the given declaration that is not a redeclaration.
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
typedef UniqueTypeWrapper UniqueTypeId;

const UniqueTypeId gUniqueTypeNull = UniqueTypeId(UNIQUETYPE_NULL);

inline bool isEqual(UniqueTypeWrapper l, UniqueTypeWrapper r)
{
	return l.value == r.value;
}

inline UniqueType getInner(UniqueType type)
{
	SYMBOLS_ASSERT(!isEqual(getTypeInfo(*type), getTypeInfo<TypeElementGeneric<struct SimpleType> >()));
	return type->next;
}

inline bool isEqualInner(UniqueTypeWrapper l, UniqueTypeWrapper r)
{
	return getInner(l.value) == getInner(r.value);
}

// Returns true if both sides are of the same element type.
// e.g. both are references, both are pointers, or both are functions.
inline bool isEqualTypeElement(UniqueTypeWrapper l, UniqueTypeWrapper r)
{
	return isEqual(getTypeInfo(*l.value), getTypeInfo(*r.value));
}


// ----------------------------------------------------------------------------
// Represents a non-type template argument value. 
// The type of the parameter is intentionally ignored in comparisons.
struct NonType : IntegralConstant
{
	explicit NonType(IntegralConstant value)
		: IntegralConstant(value)
	{
	}
};

inline bool operator<(const NonType& left, const NonType& right)
{
	return left.value < right.value;
}

inline const NonType& getNonTypeValue(UniqueType type)
{
	SYMBOLS_ASSERT(isEqual(getTypeInfo(*type), getTypeInfo<TypeElementGeneric<NonType> >()));
	return static_cast<const TypeElementGeneric<NonType>*>(type.getPointer())->value;
}


// ----------------------------------------------------------------------------
#if 1
typedef SharedVector<UniqueTypeWrapper> UniqueTypeArray;
#else
struct UniqueTypeArray : std::vector<UniqueTypeWrapper>
{
	typedef std::vector<UniqueTypeWrapper> Base;
	UniqueTypeArray()
	{
	}
	UniqueTypeArray(std::size_t count, UniqueTypeWrapper value)
		: Base(count, value)
	{
	}
	void push_back(const UniqueTypeWrapper& value)
	{
		SYMBOLS_ASSERT(size() != capacity());
		Base::push_back(value);
	}
	const_iterator begin() const
	{
		return Base::begin();
	}
	const_iterator end() const
	{
		return Base::end();
	}
};
#endif
typedef UniqueTypeArray TemplateArgumentsInstance;
typedef std::vector<UniqueTypeWrapper> InstantiatedTypes;

struct DeferredExpression : ExpressionWrapper
{
	DeferredExpression(const ExpressionWrapper& expression, TokenValue message)
		: ExpressionWrapper(expression), message(message)
	{
	}
	TokenValue message; // if non-null, this is a static_assert
};

typedef std::vector<DeferredExpression> InstantiatedExpressions;

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

struct ChildInstantiations : std::vector<ChildInstantiation>
{
	ChildInstantiations()
	{
	}
	ChildInstantiations(const ChildInstantiations&)
	{
		// do nothing
	}
	ChildInstantiations operator=(const ChildInstantiations&)
	{
		// do nothing
	}
};


typedef std::vector<Location> InstanceLocations; // temporary scaffolding!

// 14.4 Type equivalence [temp.type]
// Two template-ids refer to the same class or function if their template names are identical, they refer to the
// same template, their type template-arguments are the same type, their non-type template-arguments of integral
// or enumeration type have identical values, their non-type template-arguments of pointer or reference
// type refer to the same external object or function, and their template template-arguments refer to the same
// template.

// Represents a class, enumeration, union or fundamental type.
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
	InstantiatedExpressions childExpressions; // the dependent expressions in the specialization
	InstanceLocations childLocations; // temporary scaffolding!
	InstanceLocations childExpressionLocations; // temporary scaffolding!
	bool instantiated;
	bool instantiating;
	bool allowLookup;
	bool hasCopyAssignmentOperator;
	mutable bool visited; // used during findDeclaration to prevent infinite recursion
	mutable bool dumped; // used during dumpTemplateInstantiations to prevent duplicates
	Location instantiation;
	ChildInstantiations childInstantiations; // not copied by copy-constructor

	SimpleType(Declaration* declaration, const SimpleType* enclosing)
		: uniqueId(0), primary(declaration), declaration(declaration), enclosing(enclosing), size(0),
		instantiated(false), instantiating(false), allowLookup(false), hasCopyAssignmentOperator(false),
		visited(false), dumped(false)
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
		: left.templateArguments < right.templateArguments;
}

inline const SimpleType& getSimpleType(UniqueType type)
{
	SYMBOLS_ASSERT(isEqual(getTypeInfo(*type), getTypeInfo<TypeElementGeneric<SimpleType> >()));
	return static_cast<const TypeElementGeneric<SimpleType>*>(type.getPointer())->value;
}

// ----------------------------------------------------------------------------
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
	SYMBOLS_ASSERT(isEqual(getTypeInfo(*type), getTypeInfo<TypeElementGeneric<Namespace> >()));
	return static_cast<const TypeElementGeneric<Namespace>*>(type.getPointer())->value;
}


inline bool operator<(const Namespace& left, const Namespace& right)
{
	return left.declaration.p < right.declaration.p;
}
#endif

// ----------------------------------------------------------------------------
// Represents a template-template-argument
// Tmpl
// C::Tmpl
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
	SYMBOLS_ASSERT(isEqual(getTypeInfo(*type), getTypeInfo<TypeElementGeneric<TemplateTemplateArgument> >()));
	return static_cast<const TypeElementGeneric<TemplateTemplateArgument>*>(type.getPointer())->value;
}


// ----------------------------------------------------------------------------
// Represents a template-parameter (or template-template-parameter)
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
	SYMBOLS_ASSERT(isEqual(getTypeInfo(*type), getTypeInfo<TypeElementGeneric<DependentType> >()));
	return static_cast<const TypeElementGeneric<DependentType>*>(type.getPointer())->value;
}

// ----------------------------------------------------------------------------
// Represents the name of a type that depends on a template parameter.
// Name lookup is deferred until the enclosing template is instantiated.
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

// ----------------------------------------------------------------------------
// Represents a non-type template argument expression with a value that depends on a template parameter.
// Evaluation is deferred until the enclosing template is instantiated.
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

// ----------------------------------------------------------------------------
// Represents a decltype-specifier with a dependent expression.
// Evaluation of the expression is deferred until the enclosing template is instantiated.
struct DependentDecltype
{
	ExpressionWrapper expression;
	DependentDecltype(ExpressionWrapper expression)
		: expression(expression)
	{
	}
};

inline bool operator<(const DependentDecltype& left, const DependentDecltype& right)
{
	// [temp.type]`
	// If an expression e involves a template parameter, decltype(e) denotes a unique dependent type. Two such
	// decltype-specifiers refer to the same type only if their expressions are equivalent
	return left.expression.p < right.expression.p;
}


// ----------------------------------------------------------------------------
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

// Can be combined with other types to form T*
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

// ----------------------------------------------------------------------------
struct DeclaratorReferenceType
{
};

// Can be combined with other types to form T&
struct ReferenceType
{
};

inline bool operator<(const ReferenceType& left, const ReferenceType& right)
{
	return false;
}

// ----------------------------------------------------------------------------
struct DeclaratorMemberPointerType
{
	Type type;
	CvQualifiers qualifiers;
	DeclaratorMemberPointerType(const Type& type, CvQualifiers qualifiers)
		: type(type), qualifiers(qualifiers)
	{
	}
};

// Can be combined with other types to form T C::*
struct MemberPointerType
{
	UniqueTypeWrapper type; // the type of the class
	MemberPointerType(UniqueTypeWrapper type)
		: type(type)
	{
	}
};

inline const MemberPointerType& getMemberPointerType(UniqueType type)
{
	SYMBOLS_ASSERT(isEqual(getTypeInfo(*type), getTypeInfo<TypeElementGeneric<MemberPointerType> >()));
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

// ----------------------------------------------------------------------------
typedef std::vector<ExpressionWrapper> ArrayRank;

struct DeclaratorArrayType
{
	ArrayRank rank; // store expressions to be evaluated when template-params are known
	DeclaratorArrayType(const ArrayRank& rank)
		: rank(rank)
	{
	}
};

// Can be combined with other types to form T[size]
struct ArrayType
{
	std::size_t size; // the size of the array
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
	SYMBOLS_ASSERT(isEqual(getTypeInfo(*type), getTypeInfo<TypeElementGeneric<ArrayType> >()));
	return static_cast<const TypeElementGeneric<ArrayType>*>(type.getPointer())->value;
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

// Can be combined with other types to form T(parameters)
struct FunctionType
{
	ParameterTypes parameterTypes; // the types of the function parameters
	bool isEllipsis; // true if the parameter list ends in ellipsis
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
	SYMBOLS_ASSERT(isEqual(getTypeInfo(*node), getTypeInfo<SequenceNodeGeneric<DeclaratorFunctionType, TypeSequenceVisitor> >()));
	return static_cast<const SequenceNodeGeneric<DeclaratorFunctionType, TypeSequenceVisitor>*>(node)->value;
}
inline const Parameters& getParameters(const TypeId& type)
{
	const TypeSequence::Node* node = getLastNode(type.typeSequence);
	return getDeclaratorFunctionType(node).parameters;
}

inline const FunctionType& getFunctionType(UniqueType type)
{
	SYMBOLS_ASSERT(isEqual(getTypeInfo(*type), getTypeInfo<TypeElementGeneric<FunctionType> >()));
	return static_cast<const TypeElementGeneric<FunctionType>*>(type.getPointer())->value;
}
inline const ParameterTypes& getParameterTypes(UniqueType type)
{
	return getFunctionType(type).parameterTypes;
}


// ----------------------------------------------------------------------------
// The context at the point of instantiation.
struct InstantiationContext
{
	Location source;
	const SimpleType* enclosingType;
	const SimpleType* enclosingFunction;
	ScopePtr enclosingScope;
	bool ignoreClassMemberAccess; // true if the id-expression is the unparenthesised operand of decltype() or part of a pointer-to-member expression
	InstantiationContext(Location source, const SimpleType* enclosingType, const SimpleType* enclosingFunction, ScopePtr enclosingScope)
		: source(source), enclosingType(enclosingType), enclosingFunction(enclosingFunction), enclosingScope(enclosingScope), ignoreClassMemberAccess(false)
	{
	}
};

inline InstantiationContext setEnclosingType(const InstantiationContext& context, const SimpleType* enclosingType)
{
	return InstantiationContext(context.source, enclosingType, context.enclosingFunction, context.enclosingScope);
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
// Enabled compile-time polymorphism depending on whether a type is built-in or user-defined.
// Note that built-in types generally have a global lifetime.
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


// ----------------------------------------------------------------------------
// Returns true if the given type depends on a template parameter.
// Generally used only for debugging purposes.
inline bool isDependent(UniqueTypeWrapper type)
{
	return type.value->isDependent;
}

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

inline bool isDependent(const MemberPointerType& element)
{
	return isDependent(element.type);
}

inline bool isDependent(const FunctionType& element)
{
	return isDependent(element.parameterTypes);
}

inline bool isDependent(const DependentType&)
{
	return true;
}
inline bool isDependent(const DependentTypename&)
{
	return true;
}
inline bool isDependent(const DependentNonType&)
{
	return true;
}
inline bool isDependent(const DependentDecltype&)
{
	return true;
}

template<typename T>
inline bool isDependent(const T&)
{
	return false;
}


// ----------------------------------------------------------------------------
// Returns the base class or enclosing class that directly contains the given scope.
inline const SimpleType* findEnclosingType(const SimpleType& enclosingType, Scope* scope)
{
	SYMBOLS_ASSERT(scope != 0);
	if(scope->type == SCOPETYPE_TEMPLATE)
	{
		return enclosingType.declaration->templateParamScope == scope
			? &enclosingType
			: 0; // don't search base classes for template-parameter
	}

	if(enclosingType.declaration->enclosed == scope)
	{
		return &enclosingType;
	}

	if(enclosingType.declaration->enclosed != 0) // TODO: 'enclosingType' may be incomplete if we're finding the enclosing type for a template default argument. 
	{
		SYMBOLS_ASSERT(enclosingType.instantiated); // the enclosing type should have been instantiated by this point
	}

	for(UniqueBases::const_iterator i = enclosingType.bases.begin(); i != enclosingType.bases.end(); ++i)
	{
		const SimpleType* result = findEnclosingType(*(*i), scope);
		if(result != 0)
		{
			return result;
		}
	}
	return 0;
}

// Returns the base class or enclosing class that directly contains the given scope.
inline const SimpleType* findEnclosingType(const SimpleType* enclosingType, Scope* scope)
{
	SYMBOLS_ASSERT(scope != 0);
	for(const SimpleType* i = enclosingType; i != 0; i = (*i).enclosing)
	{
		const SimpleType* result = findEnclosingType(*i, scope);
		if(result != 0)
		{
			return result;
		}
	}
	return 0;
}

// Returns the enclosing template class/function that directly contains the given template parameter scope.
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

// Returns the enclosing template class/function that directly contains the given template parameter scope.
inline const SimpleType* findEnclosingTemplate(const InstantiationContext& context, Scope* scope)
{
	return findEnclosingTemplate(context.enclosingFunction != 0 ? context.enclosingFunction : context.enclosingType, scope);
}


// ----------------------------------------------------------------------------
const ExpressionType gNullExpressionType = ExpressionType(gUniqueTypeNull, false);
extern BuiltInTypeId gOverloaded; // represents the type of an expression referring to a set of overloaded functions
const ExpressionType gOverloadedExpressionType = ExpressionType(gOverloaded, false);

// [expr] If an expression initially has the type "reference to T", the type is adjusted to "T" prior to any further analysis.
inline UniqueTypeWrapper removeReference(UniqueTypeWrapper type)
{
	if(type.isReference())
	{
		type.pop_front();
	}
	return type;
}
inline ExpressionType removeReference(ExpressionType type)
{
	return ExpressionType(removeReference(UniqueTypeWrapper(type)), type.isLvalue);
}

// Returns the adjusted function parameter type for the given type.
// This adjusted type is used when comparing function declarations for equivalence.
inline UniqueTypeWrapper adjustFunctionParameter(UniqueTypeWrapper type)
{
	// [dcl.fct]
	// The type of a function is determined using the following rules. The
	// type of each parameter is determined from its own decl-specifier-seq and declarator. After determining the
	// type of each parameter, any parameter of type "array of T" or "function returning T" is adjusted to be
	// "pointer to T" or "pointer to function returning T," respectively. After producing the list of parameter
	// types, several transformations take place upon these types to determine the function type. Any cv-qualifier
	// modifying a parameter type is deleted. [Example: the type void(*)(const int) becomes
	// void(*)(int) —end example] Such cv-qualifiers affect only the definition of the parameter within the
	// body of the function; they do not affect the function type.
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



// ----------------------------------------------------------------------------
// expression helper

template<typename T, bool isExpression = IsConvertible<Visitable<T>, cpp::expression>::RESULT>
struct ExpressionTypeHelper;

template<typename T>
struct ExpressionTypeHelper<T, false>
{
	static UniqueTypeId get(T* symbol)
	{
		return gUniqueTypeNull;
	}
	static void set(T* symbol, UniqueTypeId declaration)
	{
	}
};

inline UniqueTypeId getExpressionTypeImpl(Visitable<cpp::expression>* symbol)
{
	return UniqueTypeId(symbol->type.p);
}
inline void setExpressionTypeImpl(Visitable<cpp::expression>* symbol, UniqueTypeId value)
{
	symbol->type.p = value.value;
}

inline UniqueTypeId getExpressionTypeImpl(Visitable<cpp::postfix_expression_suffix>* symbol)
{
	return UniqueTypeId(symbol->type.p);
}
inline void setExpressionTypeImpl(Visitable<cpp::postfix_expression_suffix>* symbol, UniqueTypeId value)
{
	symbol->type.p = value.value;
}

inline UniqueTypeId getExpressionTypeImpl(Visitable<cpp::type_id>* symbol)
{
	return UniqueTypeId(symbol->type.p);
}
inline void setExpressionTypeImpl(Visitable<cpp::type_id>* symbol, UniqueTypeId value)
{
	symbol->type.p = value.value;
}

inline UniqueTypeId getExpressionTypeImpl(Visitable<cpp::base_specifier>* symbol)
{
	return UniqueTypeId(symbol->type.p);
}
inline void setExpressionTypeImpl(Visitable<cpp::base_specifier>* symbol, UniqueTypeId value)
{
	symbol->type.p = value.value;
}


template<typename T>
inline UniqueTypeId getExpressionType(T* symbol)
{
	return getExpressionTypeImpl(makeVisitable(symbol));
}

template<typename T>
inline void setExpressionType(T* symbol, UniqueTypeId value)
{
	setExpressionTypeImpl(makeVisitable(symbol), value);
}

template<typename T>
inline UniqueTypeId getExpressionType(Visitable<T>* symbol)
{
	return getExpressionTypeImpl(symbol);
}

template<typename T>
inline void setExpressionType(Visitable<T>* symbol, UniqueTypeId value)
{
	setExpressionTypeImpl(symbol, value);
}

template<typename T>
struct ExpressionTypeHelper<T, true>
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


#endif
