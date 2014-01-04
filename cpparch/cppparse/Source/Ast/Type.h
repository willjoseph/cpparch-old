
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
	SYMBOLS_ASSERT(!isEqual(getTypeInfo(*type), getTypeInfo<TypeElementGeneric<struct SimpleType> >()));
	return type->next;
}

inline bool isEqualInner(const UniqueTypeId& l, const UniqueTypeId& r)
{
	return getInner(l.value) == getInner(r.value);
}

inline bool isSameType(const UniqueTypeId& l, const UniqueTypeId& r)
{
	return isEqual(getTypeInfo(*l.value), getTypeInfo(*r.value));
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
	SYMBOLS_ASSERT(isEqual(getTypeInfo(*type), getTypeInfo<TypeElementGeneric<NonType> >()));
	return static_cast<const TypeElementGeneric<NonType>*>(type.getPointer())->value;
}


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
	SYMBOLS_ASSERT(isEqual(getTypeInfo(*type), getTypeInfo<TypeElementGeneric<DependentType> >()));
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
	SYMBOLS_ASSERT(isEqual(getTypeInfo(*type), getTypeInfo<TypeElementGeneric<ArrayType> >()));
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


struct InstantiationContext
{
	Location source;
	const SimpleType* enclosingType;
	const SimpleType* enclosingFunction;
	ScopePtr enclosingScope;
	InstantiationContext(Location source, const SimpleType* enclosingType, const SimpleType* enclosingFunction, ScopePtr enclosingScope)
		: source(source), enclosingType(enclosingType), enclosingFunction(enclosingFunction), enclosingScope(enclosingScope)
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

template<typename T>
inline bool isDependent(const T&)
{
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

inline const SimpleType* findEnclosingTemplate(const InstantiationContext& context, Scope* scope)
{
	return findEnclosingTemplate(context.enclosingFunction != 0 ? context.enclosingFunction : context.enclosingType, scope);
}


const ExpressionType gNullExpressionType = ExpressionType(gUniqueTypeNull, false);

// [expr] If an expression initially has the type "reference to T", the type is adjusted to "T" prior to any further analysis.
inline ExpressionType removeReference(ExpressionType type)
{
	if(type.isReference())
	{
		type.pop_front();
	}
	return type;
}

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



extern BuiltInTypeId gOverloaded;
const ExpressionType gOverloadedExpressionType = ExpressionType(gOverloaded, false);

#endif
