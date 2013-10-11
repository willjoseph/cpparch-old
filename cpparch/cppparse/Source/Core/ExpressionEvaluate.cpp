
#include "ExpressionEvaluate.h"

inline IntegralConstant evaluateIdExpression(const IdExpression& node, const InstantiationContext& context)
{
	SYMBOLS_ASSERT(node.declaration->templateParameter == INDEX_INVALID);

	const SimpleType* enclosing = node.enclosing != 0 ? node.enclosing : context.enclosingType;

	const SimpleType* memberEnclosing = isMember(*node.declaration) // if the declaration is a class member
		? findEnclosingType(enclosing, node.declaration->scope) // it must be a member of (a base of) the qualifying class: find which one.
		: 0; // the declaration is not a class member and cannot be found through qualified name lookup

	return evaluate(node.declaration->initializer, setEnclosingType(context, memberEnclosing));
}

inline IdExpression substituteIdExpression(const DependentIdExpression& node, const InstantiationContext& context)
{
	SYMBOLS_ASSERT(node.qualifying != gUniqueTypeNull);
	SYMBOLS_ASSERT(context.enclosingType != 0);

	UniqueTypeWrapper substituted = substitute(node.qualifying, context);
	const SimpleType* qualifyingType = substituted.isSimple() ? &getSimpleType(substituted.value) : 0;

	if(qualifyingType == 0
		|| !isClass(*qualifyingType->declaration))
	{
		throw QualifyingIsNotClassError(context.source, node.qualifying);
	}

	instantiateClass(*qualifyingType, context);
	Identifier id;
	id.value = node.name;
	std::size_t visibility = qualifyingType->instantiating ? context.enclosingType->instantiation.pointOfInstantiation : VISIBILITY_ALL;
	LookupResultRef declaration = findDeclaration(*qualifyingType, id, IsAny(visibility));
	if(declaration == 0)
	{
		throw MemberNotFoundError(context.source, node.name, node.qualifying);
	}

	// TODO: substitute template arguments, in case of template-id when making pointer-to-function
	return IdExpression(declaration, qualifyingType, TemplateArgumentsInstance());
}

inline IntegralConstant evaluateIdExpression(const DependentIdExpression& node, const InstantiationContext& context)
{
	const IdExpression expression = substituteIdExpression(node, context);
	return evaluateIdExpression(expression, context);
}

inline const Type& getTemplateParameter(const TemplateParameters& templateParameters, std::size_t index)
{
	SYMBOLS_ASSERT(index < std::size_t(std::distance(templateParameters.begin(), templateParameters.end())));
	TemplateParameters::const_iterator i = templateParameters.begin();
	std::advance(i, index);
	return *i;
}


inline UniqueTypeWrapper getNonTypeTemplateParameterType(const NonTypeTemplateParameter& node, const InstantiationContext& context)
{
	std::size_t index = node.declaration->templateParameter;
	SYMBOLS_ASSERT(index != INDEX_INVALID);
	const SimpleType* enclosingType = findEnclosingTemplate(context.enclosingType, node.declaration->scope);
#if 1 // TEMP HACK
	if(enclosingType == 0) // special case: evaluating the type of a non-type template parameter e.g. 
	{
		SYMBOLS_ASSERT(context.enclosingScope->templateDepth != 0 // template<bool b, int x = sizeof(b)>
			|| context.enclosingScope->type == SCOPETYPE_NAMESPACE // template<int N> class C<A<N>>
			|| (getEnclosingFunction(context.enclosingScope) != 0 // within the body of a function template
				&& node.declaration->scope->templateDepth == getEnclosingFunction(context.enclosingScope)->parent->templateDepth));
		return gSignedInt; // give incorrect result for now, but this is only a temporary workaround.
	}
#endif
	SYMBOLS_ASSERT(enclosingType != 0);
#if 0 // the enclosing type may be an enclosing template definition, in which the type of a non-type template parameter is not dependent
	SYMBOLS_ASSERT(!isDependent(*enclosingType)); // assert that the enclosing type is not dependent
#endif
	SYMBOLS_ASSERT(!enclosingType->declaration->isSpecialization || enclosingType->instantiated); // a specialization must be instantiated (or in the process of instantiating)
	const TemplateParameters& templateParams = enclosingType->declaration->templateParams;
	const Type& parameter = getTemplateParameter(templateParams, index);
	UniqueTypeWrapper type = getUniqueType(parameter.declaration->type, context); // perform substitution if type is dependent
	SYMBOLS_ASSERT(!isDependent(type));
	return type;
}

inline const NonType& substituteNonTypeTemplateParameter(const NonTypeTemplateParameter& node, const InstantiationContext& context)
{
	size_t index = node.declaration->templateParameter;
	SYMBOLS_ASSERT(index != INDEX_INVALID);
	const SimpleType* enclosingType = findEnclosingTemplate(context.enclosingType, node.declaration->scope);
	SYMBOLS_ASSERT(enclosingType != 0);
	SYMBOLS_ASSERT(!isDependent(*enclosingType)); // assert that the enclosing type is not dependent
	SYMBOLS_ASSERT(!enclosingType->declaration->isSpecialization || enclosingType->instantiated); // a specialization must be instantiated (or in the process of instantiating)
	const TemplateArgumentsInstance& templateArguments = enclosingType->declaration->isSpecialization
		? enclosingType->deducedArguments : enclosingType->templateArguments;
	SYMBOLS_ASSERT(index < templateArguments.size());
	UniqueTypeWrapper argument = templateArguments[index];
	SYMBOLS_ASSERT(argument.isNonType());
	return getNonTypeValue(argument.value);
}

struct EvaluateVisitor : ExpressionNodeVisitor
{
	IntegralConstant result;
	const InstantiationContext context;
	explicit EvaluateVisitor(const InstantiationContext& context)
		: context(context)
	{
	}
	void visit(const IntegralConstantExpression& node)
	{
		result = node.value;
	}
	void visit(const CastExpression& node)
	{
		result = evaluate(node.operand, context);
	}
	void visit(const NonTypeTemplateParameter& node)
	{
		result = substituteNonTypeTemplateParameter(node, context);
	}
	void visit(const DependentIdExpression& node)
	{
		result = evaluateIdExpression(node, context);
	}
	void visit(const IdExpression& node)
	{
		result = evaluateIdExpression(node, context);
	}
	void visit(const SizeofExpression& node)
	{
		if(node.operand == 0)
		{
			std::cout << "sizeof expression with dependent type!" << std::endl;
			return;
		}

		if(isPointerToMemberExpression(node.operand))
		{
			return; // TODO
		}
		if(isPointerToFunctionExpression(node.operand))
		{
			return; // TODO
		}

		UniqueTypeWrapper type = typeOfExpression(node.operand, context);
		// [expr] If an expression initially has the type "reference to T", the type is adjusted to "T" prior to any further analysis.
		// [expr.sizeof] The sizeof operator shall not be applied to an expression that has function or incomplete type.
		result = IntegralConstant(requireCompleteObjectType(removeReference(type), context));
	}
	void visit(const SizeofTypeExpression& node)
	{
		// TODO: type-substitution for dependent node.type
		// [expr] If an expression initially has the type "reference to T", the type is adjusted to "T" prior to any further analysis.
		// [expr.sizeof] The sizeof operator shall not be applied to an expression that has function or incomplete type... or to the parenthesized name of such types.
		result = IntegralConstant(requireCompleteObjectType(removeReference(node.type), context));
	}
	void visit(const UnaryExpression& node)
	{
		result = node.operation(
			evaluate(node.first, context)
			);
	}
	void visit(const BinaryExpression& node)
	{
		result = node.operation(
			evaluate(node.first, context),
			evaluate(node.second, context)
			);
	}
	void visit(const TernaryExpression& node)
	{
		result = node.operation(
			evaluate(node.first, context),
			evaluate(node.second, context),
			evaluate(node.third, context)
			);
	}
	void visit(const TypeTraitsUnaryExpression& node)
	{
		result = IntegralConstant(node.operation(
			substitute(node.type, context)
			));
	}
	void visit(const TypeTraitsBinaryExpression& node)
	{
		result = IntegralConstant(node.operation(
			substitute(node.first, context),
			substitute(node.second, context),
			context
			));
	}
	void visit(const struct ExplicitTypeExpression& node)
	{
		// cannot be a constant expression
		SYMBOLS_ASSERT(false);
	}
	void visit(const struct ObjectExpression& node)
	{
		// cannot be a constant expression
		SYMBOLS_ASSERT(false);
	}
	void visit(const struct DependentObjectExpression& node)
	{
		// cannot be a constant expression
		SYMBOLS_ASSERT(false);
	}
	void visit(const struct ClassMemberAccessExpression& node)
	{
		// cannot be a constant expression
		SYMBOLS_ASSERT(false);
	}
	void visit(const struct FunctionCallExpression& node)
	{
		// cannot be a constant expression
		SYMBOLS_ASSERT(false);
	}
	void visit(const struct SubscriptExpression& node)
	{
		// cannot be a constant expression
		SYMBOLS_ASSERT(false);
	}
	void visit(const struct PostfixOperatorExpression& node)
	{
		// cannot be a constant expression
		SYMBOLS_ASSERT(false);
	}
};


IntegralConstant evaluate(ExpressionNode* node, const InstantiationContext& context)
{
	EvaluateVisitor visitor(context);
	node->accept(visitor);
	return visitor.result;
}



inline void addOverloads(OverloadResolver& resolver, const DeclarationInstance& declaration, const InstantiationContext& context)
{
	for(Declaration* p = findOverloaded(declaration); p != 0; p = p->overloaded)
	{
		addOverload(resolver, *p, context);
	}
}

inline void addOverloads(OverloadResolver& resolver, const OverloadSet& overloads, const InstantiationContext& context)
{
	for(OverloadSet::const_iterator i = overloads.begin(); i != overloads.end(); ++i)
	{
		const Overload& overload = *i;
		addOverload(resolver, *overload.declaration, setEnclosingType(context, overload.memberEnclosing));
	}
}

UniqueTypeWrapper getOverloadedMemberOperatorType(UniqueTypeWrapper operand, const InstantiationContext& context)
{
	const SimpleType& classType = getSimpleType(operand.value);
	SYMBOLS_ASSERT(isClass(*classType.declaration)); // assert that this is a class type
	// [expr.ref] [the type of the operand-expression shall be complete]
	instantiateClass(classType, context); // searching for overloads requires a complete type

	Identifier id;
	id.value = gOperatorArrowId;
	id.source = context.source;

	ExpressionNodeGeneric<ExplicitTypeExpression> transientExpression = ExplicitTypeExpression(operand);
	Arguments arguments(1, makeArgument(ExpressionWrapper(&transientExpression, false), operand));
	OverloadResolver resolver(arguments, 0, context);

	LookupResultRef declaration = ::findDeclaration(classType, id, IsAny());
	if(declaration != 0)
	{
		const SimpleType* memberEnclosing = findEnclosingType(&classType, declaration->scope); // find the base class which contains the member-declaration
		SYMBOLS_ASSERT(memberEnclosing != 0);
		addOverloads(resolver, declaration, setEnclosingTypeSafe(context, memberEnclosing));
	}

	FunctionOverload result = resolver.get();
	SYMBOLS_ASSERT(result.declaration != 0);
	return result.type;
}


inline void printOverloads(OverloadResolver& resolver, const OverloadSet& overloads, const InstantiationContext& context)
{
	for(OverloadSet::const_iterator i = overloads.begin(); i != overloads.end(); ++i)
	{
		const Overload& overload = *i;
		addOverload(resolver, *overload.declaration, setEnclosingType(context, overload.memberEnclosing));

		const Declaration* p = overload.declaration;
		const ParameterTypes parameters = addOverload(resolver, *p, context);
		printPosition(p->getName().source);
		std::cout << "(";
		bool separator = false;
		for(ParameterTypes::const_iterator i = parameters.begin(); i != parameters.end(); ++i)
		{
			if(separator)
			{
				std::cout << ", ";
			}
			printType(*i);
			separator = true;
		}
		std::cout << ")" << std::endl;
	}
}

// source: where the overload resolution occurs (point of instantiation)
// enclosingType: the class of which the declaration is a member (along with all its overloads).
inline FunctionOverload findBestOverloadedFunction(const OverloadSet& overloads, const TemplateArgumentsInstance* templateArguments, const Arguments& arguments, const InstantiationContext& context)
{
	SYMBOLS_ASSERT(!overloads.empty());
	OverloadResolver resolver(arguments, templateArguments, context);
	addOverloads(resolver, overloads, context);

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

	if(resolver.get().declaration == 0)
	{
		printPosition(context.source);
		std::cout << "overload resolution failed when matching arguments (";
		bool separator = false;
		for(Arguments::const_iterator i = arguments.begin(); i != arguments.end(); ++i)
		{
			if(separator)
			{
				std::cout << ", ";
			}
			printType((*i).type);
			separator = true;
		}
		std::cout << ")" << std::endl;
		std::cout << "candidates for ";
		const Declaration* declaration = overloads.front().declaration;
		printName(declaration->scope);
		std::cout << getValue(declaration->getName());
		std::cout << std::endl;
		printOverloads(resolver, overloads, context);
	}

	return resolver.get();
}



inline void addBuiltInOperatorOverload(OverloadResolver& resolver, UniqueTypeWrapper type)
{
	const ParameterTypes& parameters = getParameterTypes(type.value);
	resolver.add(FunctionOverload(&gUnknown, popType(type)), parameters, false, 0);
}

inline void addBuiltInOperatorOverloads(OverloadResolver& resolver, BuiltInTypeArrayRange overloads)
{
	for(const BuiltInType* i = overloads.first; i != overloads.last; ++i)
	{
		BuiltInType overload = *i;
		const ParameterTypes& parameters = getParameterTypes(overload.value);
		if(resolver.arguments.size() != parameters.size())
		{
			continue;
		}
		addBuiltInOperatorOverload(resolver, overload);
	}
}

typedef std::vector<UserType> UserTypeArray;

extern BuiltInTypeArrayRange gIntegralTypesRange;
extern BuiltInTypeArrayRange gPromotedIntegralTypesRange;
extern BuiltInTypeArrayRange gArithmeticTypesRange;
extern BuiltInTypeArrayRange gPromotedArithmeticTypesRange;

inline void addBuiltInTypeConversions(UserTypeArray& conversions, BuiltInTypeArrayRange types)
{
	for(const BuiltInType* i = types.first; i != types.last; ++i)
	{
		conversions.push_back(UserType(*i));
	}
}

template<typename Op>
inline void forEachBase(const SimpleType& classType, Op op)
{
	SYMBOLS_ASSERT(classType.instantiated);
	op(classType);
	for(UniqueBases::const_iterator i = classType.bases.begin(); i != classType.bases.end(); ++i)
	{
		forEachBase(**i, op);
	}
}

template<typename T>
inline void addQualificationPermutations(UserTypeArray& conversions, UniqueTypeWrapper type, const T& pointerType)
{
	conversions.push_back(UserType(pushType(qualifyType(type, CvQualifiers(false, false)), pointerType)));
	conversions.push_back(UserType(pushType(qualifyType(type, CvQualifiers(true, false)), pointerType)));
	conversions.push_back(UserType(pushType(qualifyType(type, CvQualifiers(false, true)), pointerType)));
	conversions.push_back(UserType(pushType(qualifyType(type, CvQualifiers(true, true)), pointerType)));
}

inline void addQualificationPermutations(UserTypeArray& conversions, UniqueTypeWrapper type)
{
	addQualificationPermutations(conversions, type, PointerType());
}


struct AddPointerConversions
{
	UserTypeArray& conversions;
	CvQualifiers qualifiers;
	AddPointerConversions(UserTypeArray& conversions, CvQualifiers qualifiers)
		: conversions(conversions), qualifiers(qualifiers)
	{
	}
	void operator()(const SimpleType& classType) const
	{
		addQualificationPermutations(conversions, qualifyType(makeUniqueSimpleType(classType), qualifiers));
	}
};

struct AddMemberPointerConversions
{
	UserTypeArray& conversions;
	UniqueTypeWrapper type;
	AddMemberPointerConversions(UserTypeArray& conversions, UniqueTypeWrapper type)
		: conversions(conversions), type(type)
	{
	}
	void operator()(const SimpleType& classType) const
	{
		addQualificationPermutations(conversions, type, MemberPointerType(makeUniqueSimpleType(classType)));
	}
};

inline bool isPlaceholder(UniqueTypeWrapper type)
{
	type = removeReference(type);
	return type == gArithmeticPlaceholder
		|| type == gIntegralPlaceholder
		|| type == gPromotedIntegralPlaceholder
		|| type == gPromotedArithmeticPlaceholder
		|| type == gEnumerationPlaceholder
		|| type == gPointerToAnyPlaceholder
		|| type == gPointerToObjectPlaceholder
		|| type == gPointerToClassPlaceholder
		|| type == gPointerToFunctionPlaceholder
		|| type == gPointerToMemberPlaceholder;
}

// to: The placeholder parameter of the built-in operator.
// from: The type of the argument expression after lvalue-to-rvalue conversion, or the type yielded by the best conversion function.
inline void addBuiltInOperatorConversions(UserTypeArray& conversions, UniqueTypeWrapper to, UniqueTypeWrapper from, const InstantiationContext& context)
{
	SYMBOLS_ASSERT(isPlaceholder(removeReference(to)));
	if(to.isReference())
	{
		// built-in operators that have reference parameters are always non-const, so must be an exact match.
		conversions.push_back(UserType(from));
		return;
	}
	if(isPointerPlaceholder(to))
	{
		SYMBOLS_ASSERT(from.isPointer());
		UniqueTypeWrapper type = popType(from);
		CvQualifiers qualifiers = type.value.getQualifiers();
		if(isClass(type)
			&& isComplete(type))
		{
			// the argument type 'pointer to class X' is convertible to a pointer to any base class of X
			const SimpleType& classType = getSimpleType(type.value);
			instantiateClass(classType, context);
			forEachBase(classType, AddPointerConversions(conversions, qualifiers));
		}
		else
		{
			addQualificationPermutations(conversions, type);
		}
		// the argument type 'pointer to X' is convertible to a pointer to void
		addQualificationPermutations(conversions, qualifyType(gVoid, qualifiers));
		return;
	}

	if(to == gPointerToMemberPlaceholder)
	{
		SYMBOLS_ASSERT(from.isMemberPointer());
		UniqueTypeWrapper type = popType(from);
		if(isComplete(from))
		{
			// the argument type 'pointer to member of class X of type Y' is convertible to a pointer to member of any base class of X of type Y
			const SimpleType& classType = getMemberPointerClass(from.value);
			instantiateClass(classType, context);
			forEachBase(classType, AddMemberPointerConversions(conversions, type));
		}
		else
		{
			addQualificationPermutations(conversions, type, getMemberPointerType(from.value));
		}
		return;
	}
	if(to == gEnumerationPlaceholder)
	{
		SYMBOLS_ASSERT(isEnum(from));
		conversions.push_back(UserType(from));
		// the argument type 'enumeration of type E' is also convertible to any arithmetic type 
		// drop through...
	}
	return addBuiltInTypeConversions(conversions, gArithmeticTypesRange);
}

typedef std::vector<UserTypeArray> ConversionPairs;
typedef TypeTuple<false, 1> Permutation1;
typedef TypeTuple<false, 2> Permutation2;
typedef std::vector<Permutation1> Permutation1Array;
typedef std::vector<Permutation2> Permutation2Array;

inline void addBuiltInOperatorPermutations(Permutation1Array& result, ConversionPairs& conversionPairs)
{
	SYMBOLS_ASSERT(!conversionPairs.empty());
	if(conversionPairs.size() == 1) // one argument was not a placeholder, or one argument is a null pointer constant expression
	{
		std::copy(conversionPairs[0].begin(), conversionPairs[0].end(), std::back_inserter(result));
	}
	else
	{
		SYMBOLS_ASSERT(conversionPairs.size() == 2);
		std::sort(conversionPairs[0].begin(), conversionPairs[0].end());
		std::sort(conversionPairs[1].begin(), conversionPairs[1].end());

		// find the union of both sets
		std::set_intersection(conversionPairs[0].begin(), conversionPairs[0].end(), conversionPairs[1].begin(), conversionPairs[1].end(), std::back_inserter(result));
	}
}

inline void addBuiltInOperatorPermutations(Permutation2Array& result, ConversionPairs& conversionPairs)
{
	SYMBOLS_ASSERT(!conversionPairs.empty());
	if(conversionPairs.size() == 1) // one argument was not a placeholder, or one argument is a null pointer constant expression
	{
		return;
	}
	else
	{
		SYMBOLS_ASSERT(conversionPairs.size() == 2);
		for(UserTypeArray::const_iterator i = conversionPairs[0].begin(); i != conversionPairs[0].end(); ++i)
		{
			UserType left = *i;
			for(UserTypeArray::const_iterator i = conversionPairs[0].begin(); i != conversionPairs[0].end(); ++i)
			{
				UserType right = *i;
				result.push_back(Permutation2(left, right));
			}
		}
	}
}

template<int N>
inline void addBuiltInOperatorOverloads(OverloadResolver& resolver, ArrayRange<BuiltInGenericType<N> > overloads)
{
	for(const BuiltInGenericType<N>* i = overloads.first; i != overloads.last; ++i)
	{
		BuiltInGenericType<N> overload = *i;
		const ParameterTypes& parameters = getParameterTypes(overload.value);
		if(resolver.arguments.size() != parameters.size())
		{
			continue;
		}

		ConversionPairs conversionPairs;
		conversionPairs.reserve(2);
		Arguments::const_iterator a = resolver.arguments.begin();
		ParameterTypes::const_iterator p = parameters.begin();
		for(; a != resolver.arguments.end(); ++a, ++p)
		{
			UniqueTypeWrapper to = *p;
			const Argument& from = *a;
			ImplicitConversion conversion = resolver.makeConversion(TargetType(to), from);
			if(!isValid(conversion)) // if the argument could not be converted
			{
				conversionPairs.clear();
				break;
			}
			if(!isPlaceholder(to))
			{
				continue;
			}
			if(isGeneralPointer(TargetType(to))
				&& isIntegral(from.type)) // if this argument is a null pointer constant expression
			{
				continue; // null pointer matches any pointer type, but does not give enough information to add built-in overloads
			}
			conversionPairs.push_back(UserTypeArray());
			UserTypeArray& conversions = conversionPairs.back();
			addBuiltInOperatorConversions(conversions, to, conversion.sequence.matched, resolver.context);
		}

		if(conversionPairs.empty()) // no built-in overloads can be matched by this argument list
		{
			continue;
		}

		typedef TypeTuple<false, N> Permutation;
		typedef std::vector<Permutation> Permutations;
		Permutations permutations;
		addBuiltInOperatorPermutations(permutations, conversionPairs);

		// TODO: limit qualification permutations for pointer / member-pointer to only those with equal or greater cv-qualification than type of argument expression
		for(typename Permutations::const_iterator i = permutations.begin(); i != permutations.end(); ++i)
		{
			Permutation permutation = *i;
			UserType substituted = overload.substitute(permutation);
			addBuiltInOperatorOverload(resolver, substituted);
		}
	}
}

extern BuiltInTypeArrayRange gUnaryPostIncOperatorTypes;
extern BuiltInTypeArrayRange gUnaryPreIncOperatorTypes;
extern BuiltInTypeArrayRange gUnaryArithmeticOperatorTypes;
extern BuiltInTypeArrayRange gUnaryIntegralOperatorTypes;
extern BuiltInTypeArrayRange gBinaryArithmeticOperatorTypes;
extern BuiltInTypeArrayRange gBinaryIntegralOperatorTypes;
extern BuiltInTypeArrayRange gRelationalArithmeticOperatorTypes;
extern BuiltInTypeArrayRange gShiftOperatorTypes;
extern BuiltInTypeArrayRange gAssignArithmeticOperatorTypes;
extern BuiltInTypeArrayRange gAssignIntegralOperatorTypes;
extern BuiltInTypeArrayRange gBinaryLogicalOperatorTypes;
extern BuiltInTypeArrayRange gUnaryLogicalOperatorTypes;

extern BuiltInGenericType1ArrayRange gPointerAddOperatorTypes;
extern BuiltInGenericType1ArrayRange gPointerSubtractOperatorTypes;
extern BuiltInGenericType1ArrayRange gSubscriptOperatorTypes;
extern BuiltInGenericType1ArrayRange gRelationalOperatorTypes;
extern BuiltInGenericType1ArrayRange gEqualityOperatorTypes;
extern BuiltInGenericType2ArrayRange gMemberPointerOperatorTypes;

// TODO:
// the built-in candidates include all of the candidate operator functions defined in 13.6
// that, compared to the given operator,
// - have the same operator name, and
// - accept the same number of operands, and
// - accept operand types to which the given operand or operands can be converted according to
//   13.3.3.1, and
// - do not have the same parameter type list as any non-template non-member candidate.
inline void addBuiltInOperatorOverloads(OverloadResolver& resolver, const Identifier& id)
{
	if(id.value == gOperatorPlusPlusId
		|| id.value == gOperatorMinusMinusId) // TODO: exclude 'bool' overloads for operator--
	{
		addBuiltInOperatorOverloads(resolver, gUnaryPreIncOperatorTypes);
		addBuiltInOperatorOverloads(resolver, gUnaryPostIncOperatorTypes);
	}
	else if(id.value == gOperatorStarId
		|| id.value == gOperatorDivideId)
	{
		addBuiltInOperatorOverloads(resolver, gBinaryArithmeticOperatorTypes);
	}
	else if(id.value == gOperatorPlusId
		|| id.value == gOperatorMinusId)
	{
		addBuiltInOperatorOverloads(resolver, gBinaryArithmeticOperatorTypes);
		addBuiltInOperatorOverloads(resolver, gUnaryArithmeticOperatorTypes); // +x, -x
	}
	else if(id.value == gOperatorComplId)
	{
		addBuiltInOperatorOverloads(resolver, gUnaryIntegralOperatorTypes); // ~x
	}
	else if(id.value == gOperatorLessId
		|| id.value == gOperatorGreaterId
		|| id.value == gOperatorLessEqualId
		|| id.value == gOperatorGreaterEqualId
		|| id.value == gOperatorEqualId
		|| id.value == gOperatorNotEqualId)
	{
		addBuiltInOperatorOverloads(resolver, gRelationalArithmeticOperatorTypes);
	}
	else if(id.value == gOperatorPercentId
		|| id.value == gOperatorAndId
		|| id.value == gOperatorXorId
		|| id.value == gOperatorOrId)
	{
		addBuiltInOperatorOverloads(resolver, gBinaryIntegralOperatorTypes);
	}
	else if(id.value == gOperatorShiftLeftId
		|| id.value == gOperatorShiftRightId)
	{
		addBuiltInOperatorOverloads(resolver, gShiftOperatorTypes);
	}
	else if(id.value == gOperatorAssignId
		|| id.value == gOperatorStarAssignId
		|| id.value == gOperatorDivideAssignId
		|| id.value == gOperatorPlusAssignId
		|| id.value == gOperatorMinusAssignId)
	{
		addBuiltInOperatorOverloads(resolver, gAssignArithmeticOperatorTypes);
	}
	else if(id.value == gOperatorPercentAssignId
		|| id.value == gOperatorShiftLeftAssignId
		|| id.value == gOperatorShiftRightAssignId
		|| id.value == gOperatorAndAssignId
		|| id.value == gOperatorXorAssignId
		|| id.value == gOperatorOrAssignId)
	{
		addBuiltInOperatorOverloads(resolver, gAssignIntegralOperatorTypes);
	}
	else if(id.value == gOperatorAndAndId
		|| id.value == gOperatorOrOrId)
	{
		addBuiltInOperatorOverloads(resolver, gBinaryLogicalOperatorTypes);
	}
	else if(id.value == gOperatorNotId)
	{
		addBuiltInOperatorOverloads(resolver, gUnaryLogicalOperatorTypes);
	}

	if(id.value == gOperatorPlusId)
	{
		addBuiltInOperatorOverloads(resolver, gPointerAddOperatorTypes);
	}
	else if(id.value == gOperatorMinusId)
	{
		addBuiltInOperatorOverloads(resolver, gPointerSubtractOperatorTypes);
	}
	else if(id.value == gOperatorSubscriptId)
	{
		addBuiltInOperatorOverloads(resolver, gSubscriptOperatorTypes);
	}
	else if(id.value == gOperatorLessId
		|| id.value == gOperatorGreaterId
		|| id.value == gOperatorLessEqualId
		|| id.value == gOperatorGreaterEqualId)
	{
		addBuiltInOperatorOverloads(resolver, gRelationalOperatorTypes);
	}
	else if(id.value == gOperatorEqualId
		|| id.value == gOperatorNotEqualId)
	{
		addBuiltInOperatorOverloads(resolver, gEqualityOperatorTypes);
	}
	else if(id.value == gOperatorArrowStarId)
	{
		addBuiltInOperatorOverloads(resolver, gMemberPointerOperatorTypes);
	}
}

inline FunctionOverload findBestOverloadedOperator(const Identifier& id, const Arguments& arguments, const InstantiationContext& context)
{
	Arguments::const_iterator i = arguments.begin();
	UniqueTypeWrapper left = (*i++).type;
	UniqueTypeWrapper right = i == arguments.end() ? gUniqueTypeNull : (*i).type;
	if(!isClass(left) && !isEnumeration(left)
		&& !isClass(right) && !isEnumeration(right)) // if the operand does not have class or enum type
	{
		return FunctionOverload(&gUnknown, gUniqueTypeNull);
	}
	// TODO: lookup for postfix operator++(int)

	// [over.match.oper]
	// If either operand has a type that is a class or an enumeration, a user-defined operator function might be
	// declared that implements this operator or a user-defined conversion can be necessary to convert the operand
	// to a type that is appropriate for a built-in operator. In this case, overload resolution is used to determine
	// which operator function or built-in operator is to be invoked to implement the operator.
	OverloadResolver resolver(arguments, 0, context);

	// For a unary operator @ with an operand of a type whose cv-unqualified version is T1, and for a binary operator
	// @ with a left operand of a type whose cv-unqualified version is T1 and a right operand of a type whose
	// cv-unqualified version is T2, three sets of candidate functions, designated member candidates, non-member
	// candidates and built-in candidates, are constructed as follows:	
	// - If T1 is a class type, the set of member candidates is the result of the qualified lookup of
	//   T1::operator@ (13.3.1.1.1); otherwise, the set of member candidates is empty.
	if(isClass(left)
		&& isComplete(left)) // can only find overloads if class is complete
	{
		const SimpleType& operand = getSimpleType(left.value);
		instantiateClass(operand, context); // searching for overloads requires a complete type
		LookupResultRef declaration = ::findDeclaration(operand, id, IsAny());
		if(declaration != 0)
		{
			const SimpleType* memberEnclosing = findEnclosingType(&operand, declaration->scope); // find the base class which contains the member-declaration
			SYMBOLS_ASSERT(memberEnclosing != 0);
			addOverloads(resolver, declaration, setEnclosingTypeSafe(context, memberEnclosing));
		}
	}
	// - The set of non-member candidates is the result of the unqualified lookup of operator@ in the context
	//   of the expression according to the usual rules for name lookup in unqualified function calls (3.4.2)
	//   except that all member functions are ignored. However, if no operand has a class type, only those nonmember
	//   functions in the lookup set that have a first parameter of type T1 or "reference to (possibly cv-qualified)
	//   T1", when T1 is an enumeration type, or (if there is a right operand) a second parameter of
	//   type T2 or "reference to (possibly cv-qualified) T2", when T2 is an enumeration type, are candidate
	//   functions.
	OverloadSet overloads;
	LookupResultRef declaration = findClassOrNamespaceMemberDeclaration(*context.enclosingScope, id, IsNonMemberName()); // look up non-member candidates in the enclosing scope (ignoring members)
	if(declaration != 0)
	{
		// TODO: ignore non-member candidates if no operand has a class type, unless one or more params has enum type
		addOverloaded(overloads, declaration);
	}
	addArgumentDependentOverloads(overloads, id, arguments);
	addOverloads(resolver, overloads, setEnclosingType(context, 0));

	// TODO: 13.3.1.2: built-in operators for overload resolution
	// These are relevant either when the operand has a user-defined conversion to a non-class type, or is an enum that can be converted to an arithmetic type
	// TODO: ignore built-in overloads that have same signature as a non-member
	addBuiltInOperatorOverloads(resolver, id);

	return resolver.get();
}



struct TypeOfVisitor : ExpressionNodeVisitor
{
	UniqueTypeWrapper result;
	InstantiationContext context;
	explicit TypeOfVisitor(const InstantiationContext& context)
		: context(context)
	{
	}
	void visit(const IntegralConstantExpression& node)
	{
		result = node.type;
	}
	void visit(const CastExpression& node)
	{
		result = node.type;
		SYMBOLS_ASSERT(!isDependent(result));
	}
	void visit(const NonTypeTemplateParameter& node)
	{
		result = getNonTypeTemplateParameterType(node, context);
	}
	void visit(const DependentIdExpression& node)
	{
		const IdExpression expression = substituteIdExpression(node, context);
		result = typeOfIdExpression(expression.enclosing, expression.declaration, context);
		SYMBOLS_ASSERT(!isDependent(result));
	}
	void visit(const IdExpression& node)
	{
		result = typeOfIdExpression(node.enclosing, node.declaration, context);
		SYMBOLS_ASSERT(!isDependent(result));
	}
	void visit(const SizeofExpression& node)
	{
		result = gUnsignedInt;
	}
	void visit(const SizeofTypeExpression& node)
	{
		result = gUnsignedInt;
	}
	void visit(const UnaryExpression& node)
	{
		result = typeOfUnaryExpression(node.operatorName,
			makeArgument(node.first, removeReference(typeOfExpressionWrapper(node.first, context))),
			context);
		SYMBOLS_ASSERT(!isDependent(result));
	}
	void visit(const BinaryExpression& node)
	{
		result = node.type(node.operatorName,
			makeArgument(node.first, removeReference(typeOfExpressionWrapper(node.first, context))),
			makeArgument(node.second, removeReference(typeOfExpressionWrapper(node.second, context))),
			context);
		SYMBOLS_ASSERT(!isDependent(result));
	}
	void visit(const TernaryExpression& node)
	{
		result = getConditionalOperatorType(
			removeReference(typeOfExpressionWrapper(node.second, context)),
			removeReference(typeOfExpressionWrapper(node.third, context)));
		SYMBOLS_ASSERT(!isDependent(result));
	}
	void visit(const TypeTraitsUnaryExpression& node)
	{
		result = gBool;
	}
	void visit(const TypeTraitsBinaryExpression& node)
	{
		result = gBool;
	}
	void visit(const struct ExplicitTypeExpression& node)
	{
		result = node.type;
		SYMBOLS_ASSERT(!isDependent(result));
	}
	void visit(const struct ObjectExpression& node)
	{
		result = makeUniqueSimpleType(*node.classType);
		SYMBOLS_ASSERT(!isDependent(result));
	}
	void visit(const struct DependentObjectExpression& node)
	{
		UniqueTypeWrapper type = typeOfExpressionWrapper(node.left, context);
		const SimpleType& classType = getMemberOperatorType(makeArgument(node.left, removeReference(type)), node.isArrow, context);
		result = makeUniqueSimpleType(classType);
		SYMBOLS_ASSERT(!isDependent(result));
	}
	void visit(const struct ClassMemberAccessExpression& node)
	{
		UniqueTypeWrapper type = typeOfExpressionWrapper(node.left, context);
		SYMBOLS_ASSERT(!isDependent(type)); // TODO: substitute dependent
		const SimpleType& classType = getSimpleType(type.value);
		result = typeOfExpression(node.right, setEnclosingTypeSafe(context, &classType));
		SYMBOLS_ASSERT(!isDependent(result));
	}
	void visit(const struct FunctionCallExpression& node)
	{
		result = typeOfFunctionCallExpression(
			makeArgument(node.left, removeReference(typeOfExpressionWrapper(node.left, context))),
			node.arguments,
			context);
	}
	void visit(const struct SubscriptExpression& node)
	{
		result = typeOfSubscriptExpression(
			makeArgument(node.left, removeReference(typeOfExpressionWrapper(node.left, context))),
			makeArgument(node.right, removeReference(typeOfExpressionWrapper(node.right, context))),
			context);
	}
	void visit(const struct PostfixOperatorExpression& node)
	{
		result = typeOfPostfixOperatorExpression(node.operatorName,
			makeArgument(node.operand, removeReference(typeOfExpressionWrapper(node.operand, context))),
			context);
	}
};


inline UniqueTypeWrapper typeOfExpression(ExpressionNode* node, const InstantiationContext& context)
{
	if(isPointerToMemberExpression(node)
		|| isDependentPointerToMemberExpression(node))
	{
		return gUniqueTypeNull; // TODO
	}
	if(isIdExpression(node)) // if attempting to evaluate type of id-expression with no context
	{
		const IdExpression& idExpression = getIdExpression(node);
		if(UniqueTypeWrapper(idExpression.declaration->type.unique).isFunction() // if this id-expression names a function
			&& isOverloaded(idExpression.declaration)) // which is overloaded
		{
			// can't evaluate id-expression within function-call-expression
			return gUniqueTypeOverloaded; // do not evaluate the type!
		}
	}
	if(isDependentIdExpression(node) // if attempting to evaluate type of id-expression with no context
		&& getDependentIdExpression(node).qualifying == gUniqueTypeNull) // if this name is unqualified: e.g. call to named function, find via ADL
	{
		// must defer evaluation until function call expression is evaluated
		return gUniqueTypeOverloaded; // do not evaluate the type!
	}

	TypeOfVisitor visitor(context);
	node->accept(visitor);
	return visitor.result;
}

