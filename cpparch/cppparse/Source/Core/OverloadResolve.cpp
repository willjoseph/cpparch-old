
#include "OverloadResolve.h"
#include "TypeUnique.h"

struct FunctionSignature
{
	ParameterTypes parameterTypes;
	UniqueTypeWrapper returnType;
};

ParameterTypes addOverload(OverloadResolver& resolver, const Declaration& declaration, const InstantiationContext& context)
{
	SYMBOLS_ASSERT(!isMember(declaration)
		|| &declaration == gCopyAssignmentOperatorInstance.p
		|| context.enclosingType->declaration->enclosed == declaration.scope);
	UniqueTypeWrapper type = &declaration == gCopyAssignmentOperatorInstance.p
		? makeCopyAssignmentOperatorType(*context.enclosingType)
		: getUniqueType(declaration.type, context, declaration.isTemplate);
	SYMBOLS_ASSERT(type.isFunction());

	const FunctionType& function = getFunctionType(type.value);
	FunctionSignature result;
	bool hasImplicitObjectParameter = isMember(declaration)
		&& declaration.type.declaration != &gCtor;
	result.parameterTypes.reserve(function.parameterTypes.size() + hasImplicitObjectParameter);
	if(hasImplicitObjectParameter)
	{
		// [over.match.funcs]
		// a member function is considered to have an extra parameter, called the implicit object parameter, which
		// represents the object for which the member function has been called. For the purposes of overload resolution,
		// both static and non-static member functions have an implicit object parameter, but constructors do not.
		SYMBOLS_ASSERT(isClass(*context.enclosingType->declaration));
		// For static member functions, the implicit object parameter is considered to match any object
		UniqueTypeWrapper implicitObjectParameter = gImplicitObjectParameter;
		if(!isStatic(declaration))
		{
			// For non-static member functions, the type of the implicit object parameter is "reference to cv X" where X is
			// the class of which the function is a member and cv is the cv-qualification on the member function declaration.
			// TODO: conversion-functions, non-conversions introduced by using-declaration
			implicitObjectParameter = makeUniqueSimpleType(*context.enclosingType);
			implicitObjectParameter.value.setQualifiers(type.value.getQualifiers());
			implicitObjectParameter.push_front(ReferenceType());
		}
		result.parameterTypes.push_back(implicitObjectParameter);
	}
	bool isEllipsis = function.isEllipsis;
	for(ParameterTypes::const_iterator p = function.parameterTypes.begin(); p != function.parameterTypes.end(); ++p)
	{
		result.parameterTypes.push_back(*p);
	}
	result.returnType = popType(type);

	if(!declaration.isTemplate)
	{
		// [temp.arg.explicit] An empty template argument list can be used to indicate that a given use refers to a
		// specialization of a function template even when a normal (i.e., non-template) function is visible that would
		// otherwise be used.
		if(resolver.templateArguments != 0)
		{
			return ParameterTypes();
		}
		resolver.add(FunctionOverload(const_cast<Declaration*>(&declaration), getFunctionCallExpressionType(result.returnType)), result.parameterTypes, isEllipsis, context.enclosingType);
		return result.parameterTypes;
	}

	if(declaration.isSpecialization) // function template specializations don't take part in overload resolution?
	{
		return ParameterTypes();
	}

	FunctionTemplate functionTemplate;
	functionTemplate.parameters.swap(result.parameterTypes);
	makeUniqueTemplateParameters(declaration.templateParams, functionTemplate.templateParameters, context, true);

	if(resolver.arguments.size() > functionTemplate.parameters.size())
	{
		return ParameterTypes(); // more arguments than parameters. TODO: same for non-template?
	}

	// [temp.deduct] When an explicit template argument list is specified, the template arguments must be compatible with the template parameter list
	if(resolver.templateArguments != 0
		&& resolver.templateArguments->size() > functionTemplate.templateParameters.size())
	{
		return ParameterTypes(); // too many explicitly specified template arguments
	}
	SimpleType specialization(const_cast<Declaration*>(&declaration), context.enclosingType);
	specialization.instantiated = true;
	{
		std::size_t count = std::max(resolver.templateArguments != 0 ? resolver.templateArguments->size() : 0, functionTemplate.templateParameters.size());
		specialization.templateArguments.reserve(count);
		UniqueTypeArray::const_iterator p = functionTemplate.templateParameters.begin();
		if(resolver.templateArguments != 0)
		{
			for(TemplateArgumentsInstance::const_iterator a = resolver.templateArguments->begin(); a != resolver.templateArguments->end(); ++a, ++p)
			{
				SYMBOLS_ASSERT(p != functionTemplate.templateParameters.end());
				if((*a).isNonType() != (*p).isDependentNonType())
				{
					return ParameterTypes(); // incompatible explicitly specified arguments
				}
				specialization.templateArguments.push_back(*a);
			}
		}
		for(; p != functionTemplate.templateParameters.end(); ++p)
		{
			specialization.templateArguments.push_back(*p);
		}
	}

	try
	{
		// substitute the template-parameters in the function's parameter list with the explicitly specified template-arguments
		ParameterTypes parameterTypes;
		substitute(parameterTypes, functionTemplate.parameters, setEnclosingTypeSafe(context, &specialization));
		// TODO: [temp.deduct]
		// After this substitution is performed, the function parameter type adjustments described in 8.3.5 are performed.

		UniqueTypeArray arguments;
		arguments.reserve(resolver.arguments.size());
		for(Arguments::const_iterator a = resolver.arguments.begin(); a != resolver.arguments.end(); ++a)
		{
			arguments.push_back((*a).type);
		}

		specialization.templateArguments.resize(resolver.templateArguments == 0 ? 0 : resolver.templateArguments->size()); // preserve the explicitly specified arguments
		specialization.templateArguments.resize(functionTemplate.templateParameters.size(), gUniqueTypeNull);
		// NOTE: in rare circumstances, deduction may cause implicit instantiations, which occur at the point of overload resolution 
		if(!deduceFunctionCall(parameterTypes, arguments, specialization.templateArguments, resolver.context))
		{
			return ParameterTypes(); // deduction failed
		}

		// substitute the template-parameters in the function's parameter list with the deduced template-arguments
		substitute(result.parameterTypes, parameterTypes, setEnclosingTypeSafe(context, &specialization));
		result.returnType = substitute(result.returnType, setEnclosingTypeSafe(context, &specialization)); // substitute the return type. TODO: should wait until overload is chosen?

		resolver.add(FunctionOverload(const_cast<Declaration*>(&declaration), getFunctionCallExpressionType(result.returnType)), result.parameterTypes, isEllipsis, context.enclosingType, functionTemplate);
		return result.parameterTypes;
	}
	catch(TypeError&)
	{
		// deduction and checking failed
	}

	return ParameterTypes();
}

