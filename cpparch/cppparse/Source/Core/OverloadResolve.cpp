
#include "OverloadResolve.h"
#include "TypeUnique.h"


FunctionSignature substituteFunctionId(const Overload& overload, const UniqueTypeArray& argumentTypes, const TemplateArgumentsInstance* templateArguments, const InstantiationContext& context)
{
	const Declaration& declaration = *overload.declaration;
	SYMBOLS_ASSERT(!isMember(declaration)
		|| &declaration == gCopyAssignmentOperatorInstance.p
		|| overload.memberEnclosing->declaration->enclosed == declaration.scope);
	UniqueTypeWrapper type = &declaration == gCopyAssignmentOperatorInstance.p
		? UniqueTypeWrapper(makeCopyAssignmentOperatorType(*overload.memberEnclosing))
		: getUniqueType(declaration.type, setEnclosingType(context, overload.memberEnclosing), declaration.isTemplate);
	SYMBOLS_ASSERT(type.isFunction());

	const FunctionType& function = getFunctionType(type.value);
	FunctionSignature result;
	result.isEllipsis = function.isEllipsis;
	result.parameterTypes = function.parameterTypes;
	result.returnType = popType(type);
	result.qualifiers = type.value.getQualifiers();

	if(!declaration.isTemplate)
	{
		return result; // non-template signature does not require substitution
	}

	if(declaration.isSpecialization) // function template specializations don't take part in overload resolution?
	{
		return FunctionSignature();
	}

	result.parameters.swap(result.parameterTypes);
	makeUniqueTemplateParameters(declaration.templateParams, result.templateParameters, InstantiationContext(), true);

	if(!function.isEllipsis // if the function accepts a specific number of arguments
		&& argumentTypes.size() > result.parameters.size()) // and we are given more arguments than parameters
	{
		return FunctionSignature(); // early out. TODO: same for non-template?
	}

	// [temp.deduct] When an explicit template argument list is specified, the template arguments must be compatible with the template parameter list
	if(templateArguments != 0
		&& templateArguments->size() > result.templateParameters.size())
	{
		return FunctionSignature(); // too many explicitly specified template arguments
	}
	SimpleType specialization(const_cast<Declaration*>(&declaration), overload.memberEnclosing);
	specialization.instantiated = true;
	{
		std::size_t count = std::max(templateArguments != 0 ? templateArguments->size() : 0, result.templateParameters.size());
		specialization.templateArguments.reserve(count);
		UniqueTypeArray::const_iterator p = result.templateParameters.begin();
		if(templateArguments != 0)
		{
			for(TemplateArgumentsInstance::const_iterator a = templateArguments->begin(); a != templateArguments->end(); ++a, ++p)
			{
				SYMBOLS_ASSERT(p != result.templateParameters.end());
				if((*a).isNonType() != (*p).isDependentNonType())
				{
					return FunctionSignature(); // incompatible explicitly specified arguments
				}
				specialization.templateArguments.push_back(*a);
			}
		}
		for(; p != result.templateParameters.end(); ++p)
		{
			specialization.templateArguments.push_back(*p);
		}
	}

	try
	{
		InstantiationContext functionContext = setEnclosingTypeSafe(context, &specialization);
		// substitute the template-parameters in the function's parameter list with the explicitly specified template-arguments
		ParameterTypes parameters;
		substitute(parameters, result.parameters, functionContext);
		// TODO: [temp.deduct]
		// After this substitution is performed, the function parameter type adjustments described in 8.3.5 are performed.

		specialization.templateArguments.resize(templateArguments == 0 ? 0 : templateArguments->size()); // preserve the explicitly specified arguments
		if(argumentTypes.empty()) // if there are no arguments from which to deduce template parameters
		{
			if(specialization.templateArguments.size() < result.templateParameters.size()) // if the number of explicitly specified template arguments is less than the number of template parameters
			{
				return FunctionSignature(); // not enough explicitly specified template arguments
			}
		}
		else
		{
			specialization.templateArguments.resize(result.templateParameters.size(), gUniqueTypeNull); // prepare to deduce the remaining template arguments

			// NOTE: in rare circumstances, deduction may cause implicit instantiations, which occur at the point of overload resolution
			if(!deduceFunctionCall(parameters, argumentTypes, specialization.templateArguments, context))
			{
				return FunctionSignature(); // deduction failed
			}
		}

		// substitute the template-parameters in the function's parameter list with the deduced template-arguments
		substitute(result.parameterTypes, parameters, functionContext);
		result.returnType = substitute(result.returnType, functionContext); // substitute the return type. TODO: should wait until overload is chosen?

		return result;
	}
	catch(TypeError&)
	{
		// deduction and checking failed
	}

	return FunctionSignature();
}

ParameterTypes addOverload(OverloadResolver& resolver, const Overload& overload)
{
	if(!overload.declaration->isTemplate
		&& resolver.templateArguments != 0)
	{
		// [temp.arg.explicit] An empty template argument list can be used to indicate that a given use refers to a
		// specialization of a function template even when a normal (i.e., non-template) function is visible that would
		// otherwise be used.
		return ParameterTypes();
	}

	bool hasImplicitObjectParameter = isMember(*overload.declaration)
		&& overload.declaration->type.declaration != &gCtor;

	UniqueTypeArray argumentTypes;
	argumentTypes.reserve(resolver.arguments.size());
	for(Arguments::const_iterator a = resolver.arguments.begin() + hasImplicitObjectParameter; a != resolver.arguments.end(); ++a)
	{
		argumentTypes.push_back((*a).type);
	}

	FunctionSignature result = substituteFunctionId(overload, argumentTypes, resolver.templateArguments, resolver.context);
	if(result.returnType == gUniqueTypeNull) // if substitution failed
	{
		return ParameterTypes();
	}

	resolver.add(FunctionOverload(const_cast<Declaration*>(overload.declaration), getFunctionCallExpressionType(result.returnType)), result.parameterTypes, result.isEllipsis, result.qualifiers, overload.memberEnclosing, result);
	return result.parameterTypes;
}

// f<int> -> int()
// &f<int> -> int(*)()
// C::mf<int> -> int() [ill-formed if not part of unary address-of]
// &C::mf<int> -> int(C::*)()
inline ExpressionType selectOverloadedFunctionImpl(UniqueTypeWrapper target, const IdExpression& expression, const InstantiationContext& context)
{
	// the source type is an overloaded function, or is a pointer to an overloaded function
	// select matching function from overload set;
	DeclarationInstanceRef declaration = expression.declaration;
	for(Declaration* p = findOverloaded(declaration); p != 0; p = p->overloaded)
	{
		if(p->specifiers.isFriend)
		{
			continue; // ignore (namespace-scope) friend functions
		}
		UniqueTypeWrapper type;
		if(!p->isTemplate)
		{
			type = getUniqueType(p->type, context);
		}
		else
		{
			const SimpleType* idEnclosing = getIdExpressionClass(expression.enclosing, declaration, context.enclosingType);
			FunctionSignature result = substituteFunctionId(Overload(declaration, idEnclosing), UniqueTypeArray(), &expression.templateArguments, context);
			if(result.returnType == gUniqueTypeNull)
			{
				continue; // substitution failed
			}
			type = pushType(result.returnType, result.getFunctionType());
		}
		SYMBOLS_ASSERT(type.isFunction());
		if(target == gUniqueTypeNull // if no target type was specified (e.g. auto, decltype)
			|| type == target) // or the function type matches the target type
		{
			return ExpressionType(type, true);
		}
	}
	return gOverloadedExpressionType; // no matching function in overload set
}

inline ExpressionType selectOverloadedFunction(UniqueTypeWrapper to, Argument from, const InstantiationContext& context)
{
	// [over.over]
	// A use of an overloaded function name without arguments is resolved in certain contexts to a function, a
	// pointer to function or a pointer to member function for a specific function from the overload set.
	if(!to.isPointer()
		&& !to.isReference()
		&& !to.isMemberPointer())
	{
		return from.type;
	}
	UniqueTypeWrapper target = popType(to);
	if(!target.isFunction())
	{
		return from.type;
	}
	// the target is a function type
	UniqueTypeWrapper overloaded = from.type;
	if(overloaded.isPointer()
		|| overloaded.isMemberPointer()) // TODO: select non-static-member over static-member?
	{
		overloaded = popType(overloaded);
	}
	if(overloaded != gOverloaded)
	{
		return from.type;
	}
	ExpressionNode* node = from.p;
	if(isUnaryExpression(node))
	{
		SYMBOLS_ASSERT(getUnaryExpression(node).operatorName == gOperatorAndId);
		node = getUnaryExpression(node).first;
	}
	SYMBOLS_ASSERT(isIdExpression(node));
	return selectOverloadedFunctionImpl(target, getIdExpression(node), context);
}
