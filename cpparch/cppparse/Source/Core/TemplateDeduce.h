
#ifndef INCLUDED_CPPPARSE_CORE_TEMPLATEDEDUCE_H
#define INCLUDED_CPPPARSE_CORE_TEMPLATEDEDUCE_H

#include "Ast/Type.h"
#include "Ast/ExpressionImpl.h"
#include "TypeInstantiate.h"

// ----------------------------------------------------------------------------
// template argument deduction

inline bool deduceTemplateArguments(UniqueTypeWrapper parameter, UniqueTypeWrapper argument, TemplateArgumentsInstance& result, bool allowGreaterCvQualification = false);

inline bool deducePairs(const UniqueTypeArray& parameters, const UniqueTypeArray& arguments, TemplateArgumentsInstance& result)
{
	UniqueTypeArray::const_iterator p = parameters.begin();
	for(UniqueTypeArray::const_iterator a = arguments.begin();
		a != arguments.end() && p != parameters.end(); // for each pair P, A
		// fewer arguments than parameters: occurs when some parameters are defaulted
		// more arguments than parameters: occurs when matching a specialization such as 'struct S<>'
		++a, ++p)
	{
		if(!deduceTemplateArguments(*p, *a, result))
		{
			return false;
		}
	}
	return true;
}

// [temp.deduct.type] The nondeduced contexts are:
// - The nestedname-specifier of a type that was specified using a qualifiedid.
// - A type that is a template-id in which one or more of the template-arguments is an expression that references
//	 a templateparameter.
// When a type name is specified in a way that includes a nondeduced context, all of the types that comprise
// that type name are also nondeduced.
inline bool isNonDeduced(const SimpleType& type)
{
	return false; // TODO
}

struct DeduceVisitor : TypeElementVisitor
{
	UniqueTypeWrapper argument;
	TemplateArgumentsInstance& templateArguments;
	bool result;
	DeduceVisitor(UniqueTypeWrapper argument, TemplateArgumentsInstance& templateArguments)
		: argument(argument), templateArguments(templateArguments), result(true)
	{
	}
	void commit(std::size_t index)
	{
		SYMBOLS_ASSERT(index != INDEX_INVALID);
		SYMBOLS_ASSERT(index < templateArguments.size());
		if(templateArguments[index] == gUniqueTypeNull) // if this argument was not already deduced (or explicitly specified)
		{
			templateArguments[index] = argument; // use the deduced argument
		}
		else if(templateArguments[index] != argument) // if the deduced (or explicitly specified) argument is different
		{
			result = false;
		}
	}
#if 0
	virtual void visit(const Namespace& element)
	{
		SYMBOLS_ASSERT(false);
	}
#endif
	virtual void visit(const DependentType& element) // deduce from T, TT, TT<...>
	{
		if(element.templateParameterCount != 0) // TT or TT<..>
		{
			if(element.templateArguments.empty()) // TT
			{
				if(!argument.isTemplateTemplateArgument())
				{
					result = false;
					return;
				}
			}
			else  // TT<..>
			{
				if(!argument.isSimple())
				{
					result = false;
					return;
				}
				const SimpleType& type = getSimpleType(argument.value);
				if(!type.declaration->isTemplate
					|| !deducePairs(element.templateArguments, type.templateArguments, templateArguments)) // template-template-parameter may have template-arguments that refer to a template parameter
				{
					result = false;
					return;
				}
				argument = gUniqueTypeNull;
				argument.push_front(TemplateTemplateArgument(type.declaration, type.enclosing));
			}
		}
		commit(element.type->templateParameter);
	}
	virtual void visit(const DependentTypename&)
	{
		// cannot deduce from T::
	}
	virtual void visit(const DependentNonType& element)
	{
		// if this expression is of the form 'i' where 'i' is a non-type template parameter
		if(isNonTypeTemplateParameter(element.expression))
		{
			// deduce the argument from the name of the non-type template parameter
			commit(getNonTypeTemplateParameter(element.expression).declaration->templateParameter);
		}
	}
	virtual void visit(const TemplateTemplateArgument& element)
	{
		// cannot deduce from name of primary template
	}
	virtual void visit(const NonType&)
	{
		// cannot deduce from integral constant expression
	}
	virtual void visit(const SimpleType& element)
	{
		SYMBOLS_ASSERT(argument.isSimple());
		const SimpleType& type = getSimpleType(argument.value);
		if(type.primary != element.primary) // if the class type does not match
		{
			result = false; // deduction fails
			return;
		}
		// [temp.deduct.type] The nondeduced contexts are:- The nested-name-specifier of a type that was specified using a qualified-id.
		// not attempting to deduce from enclosing type
		if(!isNonDeduced(element))
		{
			result = deducePairs(element.templateArguments, type.templateArguments, templateArguments);
		}
	}
	virtual void visit(const PointerType&)
	{
		// cannot deduce from pointer
	}
	virtual void visit(const ReferenceType&)
	{
		// cannot deduce from reference
	}
	virtual void visit(const ArrayType&)
	{
		// TODO: deduce type-name[i]
	}
	virtual void visit(const MemberPointerType& element)
	{
		SYMBOLS_ASSERT(argument.isMemberPointer());
		result = deduceTemplateArguments(element.type, getMemberPointerType(argument.value).type, templateArguments);
	}
	virtual void visit(const FunctionType& element)
	{
		SYMBOLS_ASSERT(argument.isFunction());
		result = deducePairs(element.parameterTypes, getParameterTypes(argument.value), templateArguments);
	}
};

inline UniqueTypeWrapper applyArrayToPointerConversion(UniqueTypeWrapper type);
inline UniqueTypeWrapper applyFunctionToPointerConversion(UniqueTypeWrapper type);

struct DeductionFailure
{
};

inline const SimpleType* findUniqueBase(const SimpleType& derived, const Declaration& type, const SimpleType* result = 0)
{
	SYMBOLS_ASSERT(derived.instantiated);
	SYMBOLS_ASSERT(derived.declaration->enclosed != 0);
	SYMBOLS_ASSERT(isClass(type));
	for(UniqueBases::const_iterator i = derived.bases.begin(); i != derived.bases.end(); ++i)
	{
		const SimpleType& base = *(*i);
		SYMBOLS_ASSERT(isClass(*base.declaration));
		if(base.primary == &type)
		{
			if(result != 0)
			{
				throw DeductionFailure();
			}
			result = &base;
		}
		result = findUniqueBase(base, type, result);
	}
	return result;
}

inline UniqueTypeWrapper removePointer(UniqueTypeWrapper type)
{
	if(type.isPointer())
	{
		type.pop_front();
	}
	return type;
}

inline const SimpleType* getClassType(UniqueTypeWrapper type)
{
	if(!type.isSimple())
	{
		return 0;
	}
	const SimpleType* result = &getSimpleType(type.value);
	if(!isClass(*result->declaration))
	{
		return 0;
	}
	return result;
}

inline void adjustFunctionCallDeductionPair(UniqueTypeWrapper& parameter, UniqueTypeWrapper& argument, const InstantiationContext& context)
{
	argument = removeReference(argument);

	// [temp.deduct.call]
	// If P is a cv-qualified type, the top level cv-qualifiers of P’s type are ignored for type deduction.
	parameter.value.setQualifiers(CvQualifiers());
	// If P is a reference type, the type referred to by P is used for type deduction.
	if(parameter.isReference())
	{
		parameter = removeReference(parameter);
	}
	// If P is not a reference type:
	else
	{
		// - If A is an array type, the pointer type produced by the array-to-pointer standard conversion (4.2) is used
		// in place of A for type deduction; otherwise,
		if(argument.isArray())
		{
			argument = applyArrayToPointerConversion(argument);
		}
		// - If A is a function type, the pointer type produced by the function-to-pointer
		// standard conversion (4.3) is used in place of A for type deduction; otherwise,
		else if(argument.isFunction())
		{
			argument = applyFunctionToPointerConversion(argument);
		}
		// - If A is a cv-qualified type, the top level cv-qualifiers
		// of A’s type are ignored for type deduction.
		else
		{
			argument.value.setQualifiers(CvQualifiers());
		}
	}

	// [temp.deduct.call]
	// In general, the deduction process attempts to find template argument values that will make the deduced A identical to A
	// However, there are three cases that allow a difference:
	// — If the original P is a reference type, the deduced A (i.e., the type referred to by the reference) can be
	// more cv-qualified than A.
	// 	— A can be another pointer or pointer to member type that can be converted to the deduced A via a qualification
	// 	conversion (4.4).
	// 	— If P is a class, and P has the form template-id, then A can be a derived class of the deduced A. Likewise,
	// 	if P is a pointer to a class of the form template-id, A can be a pointer to a derived class pointed to
	// 		by the deduced A.
	// These alternatives are considered only if type deduction would otherwise fail. If they yield more than one
	// possible deduced A, the type deduction fails.

	const SimpleType* parameterType = getClassType(removePointer(parameter));
	const SimpleType* argumentType = getClassType(removePointer(argument));
	if(parameterType != 0 && parameterType->declaration->isTemplate // if P is a class-template
		&& argumentType != 0 && isComplete(*argumentType->declaration) // and A is a complete class
		&& parameter.isPointer() == argument.isPointer() // and neither (or both) are pointers
		&& argumentType->primary != parameterType->primary) // and deduction would fail
	{
		instantiateClass(*argumentType, context); // A must be instantiated before searching its bases
		const SimpleType* base = findUniqueBase(*argumentType, *parameterType->primary);
		if(base != 0) // if P is an unambiguous base-class of A
		{
			// A can be a derived class of the deduced A
			bool isPointer = argument.isPointer();
			CvQualifiers qualifiers = removePointer(argument).value.getQualifiers();
			argument = makeUniqueSimpleType(*base); // use the base-class in place of A for deduction
			argument.value.setQualifiers(qualifiers); // preserve the cv-qualification of the original A
			if(isPointer)
			{
				argument.push_front(PointerType());
			}
		}
	}
}

inline bool deduceTemplateArguments(UniqueTypeWrapper parameter, UniqueTypeWrapper argument, TemplateArgumentsInstance& result, bool allowGreaterCvQualification)
{
	// [temp.deduct.type]
	// Template arguments can be deduced in several different contexts, but in each case a type that is specified in
	// terms of template parameters (call it P) is compared with an actual type (call it A), and an attempt is made
	// to find template argument values (a type for a type parameter, a value for a non-type parameter, or a template
	// for a template parameter) that will make P, after substitution of the deduced values (call it the deduced
	// A), compatible with A.
	if(!isDependent(parameter))
	{
		// P is not specified in terms of template parameters. Deduction succeeds, but does not deduce anything,
		// unless the argument is specified in terms of template parameters.
		return !isDependent(argument);
	}

	std::size_t depth = 0;
	// compare P and A, to find a deduced A that matches P.
	// 'parameter' becomes the deduced A, while 'argument' is the original A.
	for(; !parameter.empty() && !argument.empty(); parameter.pop_front(), argument.pop_front(), ++depth)
	{
		if(allowGreaterCvQualification) // if this is a function-call and we are comparing either the outermost elements, or the outer elements form a const pointer/member-pointer sequence
		{
			// greater cv-qualification of the inner elements is allowed only if the outer elements form a const pointer/member-pointer sequence
			allowGreaterCvQualification = (parameter.isPointer() || parameter.isMemberPointer())
				&& (depth == 0 || parameter.value.getQualifiers().isConst);

			if(isGreaterCvQualification(parameter, argument)) // and the deduced A is more qualified than A (i.e. deduction would fail)
			{
				parameter.value.setQualifiers(argument.value.getQualifiers()); // use cv-qualification of A to ensure deduction succeeds
			}
		}

		if(!parameter.isDependent())
		{
			if(!isSameType(parameter, argument))
			{
				return false; // the deduced A must be identical to A: e.g. T* <- int*, S<T> <- S<int>
			}

			if(!isEqualCvQualification(parameter, argument))
			{
				return false; // the deduced A can not be differently cv-qualified than A
			}
		}

		if(parameter.isDependent()) // if template-parameter 'T' is found
		{
			// if only P is qualified, fail!: e.g. const T <- int
			if(isGreaterCvQualification(parameter, argument))
			{
				return false;
			}
			// if both are qualified, remove qualification: e.g. const T <- const int = int
			// if only A is qualified, add qualification: e.g. T <- const int = const int
			CvQualifiers qualifiers = argument.value.getQualifiers();
			qualifiers.isConst ^= parameter.value.getQualifiers().isConst;
			qualifiers.isVolatile ^= parameter.value.getQualifiers().isVolatile;
			argument.value.setQualifiers(qualifiers);
		}
		DeduceVisitor visitor(argument, result);
		parameter.value->accept(visitor);
		if(!visitor.result)
		{
			return false;
		}
	}
	return true;
}

inline bool isValid(const UniqueTypeArray& result)
{
	return std::find(result.begin(), result.end(), gUniqueTypeNull) == result.end();
}

// deduce the function's template arguments by comparing the original argument list with the substituted parameters
inline bool deduceFunctionCall(const ParameterTypes& parameters, const UniqueTypeArray& arguments, TemplateArgumentsInstance& result, const InstantiationContext& context)
{
	try
	{
		ParameterTypes::const_iterator p = parameters.begin();
		for(UniqueTypeArray::const_iterator a = arguments.begin();
			a != arguments.end() && p != parameters.end(); // for each pair P, A
			// fewer arguments than parameters: occurs when some parameters are defaulted
			// TODO: more arguments than parameters: occurs when ???
			++a, ++p)
		{
			UniqueTypeWrapper parameter = *p;
			UniqueTypeWrapper argument = *a;
			adjustFunctionCallDeductionPair(parameter, argument, context);
			if(!deduceTemplateArguments(parameter, argument, result, true))
			{
				return false;
			}
		}
		if(!isValid(result))
		{
			return false;
		}
	}
	catch(DeductionFailure)
	{
		return false;
	}
	return true;
}


#endif
