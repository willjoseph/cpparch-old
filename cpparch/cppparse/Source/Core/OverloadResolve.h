
#ifndef INCLUDED_CPPPARSE_CORE_OVERLOADRESOLVE_H
#define INCLUDED_CPPPARSE_CORE_OVERLOADRESOLVE_H

#include "TypeConvert.h"
#include "TemplateDeduce.h"
#include "TypeSubstitute.h"
#include "OperatorId.h"



struct ImplicitConversion
{
	StandardConversionSequence sequence; // if user-defined, this is the second standard conversion
	IcsType type;
	Declaration* conversion; // if user-defined, this is the declaration of the conversion function
	explicit ImplicitConversion(StandardConversionSequence sequence, IcsType type = ICSTYPE_STANDARD, Declaration* conversion = 0)
		: sequence(sequence), type(type), conversion(conversion)
	{
	}
};

const ImplicitConversion IMPLICITCONVERSION_ELLIPSIS = ImplicitConversion(StandardConversionSequence(SCSRANK_IDENTITY, CvQualifiers()), ICSTYPE_ELLIPSIS);

inline ExpressionType getFunctionCallExpressionType(UniqueTypeWrapper type)
{
	// [expr.call] A function call is an lvalue if and only if the result type is a reference.
	return ExpressionType(type, type.isReference());
}

struct FunctionOverload
{
	Declaration* declaration;
	ExpressionType type;
	FunctionOverload(Declaration* declaration, ExpressionType type)
		: declaration(declaration), type(type)
	{
	}
};

inline ExpressionType selectOverloadedFunction(UniqueTypeWrapper to, Argument from, const InstantiationContext& context);
inline ExpressionType selectOverloadedFunctionImpl(UniqueTypeWrapper target, const IdExpression& expression, const InstantiationContext& context);

template<typename To>
FunctionOverload findBestConversionFunction(To to, Argument from, const InstantiationContext& context, bool isNullPointerConstant = false);

template<typename To>
ImplicitConversion makeImplicitConversionSequence(To to, Argument from, const InstantiationContext& context, bool isNullPointerConstant = false, bool isUserDefinedConversion = false)
{
	SYMBOLS_ASSERT(to != gUniqueTypeNull);
	SYMBOLS_ASSERT(from.p != 0);
	SYMBOLS_ASSERT(from.type != gUniqueTypeNull);

	bool isReference = false;
	if(from.type.isReference())
	{
		SYMBOLS_ASSERT(from.type.isLvalue); // TODO: is this always true?
		isReference = true;
		from.type.pop_front(); // TODO: removal of reference won't be detected later
	}

	// 13.3.3.1.4 [over.ics.ref]: reference binding
	if(to.isReference()) 
	{
		to.pop_front();
		// [dcl.init.ref]
		// Given types "cv1 T1" and "cv2 T2," "cv1 T1" is reference-related to "cv2 T2" if T1 is the same type as
		// T2, or T1 is a base class of T2. "cv1 T1" is reference-compatible with "cv2 T2" if T1 is reference-related
		// to T2 and cv1 is the same cv-qualification as, or greater cv-qualification than, cv2. For purposes of overload
		// resolution, cases for which cv1 is greater cv-qualification than cv2 are identified as reference-compatible
		// with added qualification (see 13.3.3.2).
		// 
		// A reference to type "cv1 T1" is initialized by an expression of type "cv2 T2" as follows:
		// If the initializer expression
		// - is an lvalue (but is not a bit-field), and "cv1 T1" is reference-compatible with "cv2 T2," or
		// - has a class type (i.e., T2 is a class type) and can be implicitly converted to an lvalue of type
		//   "cv3 T3," where "cv1 T1" is reference-compatible with "cv3 T3"
		// then the reference is bound directly to the initializer expression lvalue in the first case, and the reference
		// is bound to the lvalue result of the conversion in the second case. In these cases the reference is said to
		// bind directly to the initializer expression.
		if(from.type.isLvalue
			&& (isEqualCvQualification(to, from.type)
			|| isGreaterCvQualification(to, from.type)))
		{
			// [over.ics.ref]
			// When a parameter of reference type binds directly (8.5.3) to an argument expression, the implicit conversion
			// sequence is the identity conversion, unless the argument expression has a type that is a derived class of
			// the parameter type, in which case the implicit conversion sequence is a derived-to-base Conversion
			UniqueTypeWrapper matched = getExactMatchNoQualifiers(to, from.type);
			if(matched != gUniqueTypeNull)
			{
				StandardConversionSequence sequence(SCSRANK_IDENTITY, to.value.getQualifiers(), matched);
				sequence.isReference = true;
				return ImplicitConversion(sequence);
			}
			if(to.isSimple()
				&& from.type.isSimple()
				&& isBaseOf(getSimpleType(to.value), getSimpleType(from.type.value), context))
			{
				StandardConversionSequence sequence(SCSRANK_CONVERSION, to.value.getQualifiers());
				sequence.isReference = true;
				return ImplicitConversion(sequence);
			}
			// drop through...
		}
		if(isClass(from.type))
		{
			To tmp = to;
			tmp.push_front(ReferenceType());
			FunctionOverload overload = findBestConversionFunction(tmp, from, context);
			if(overload.declaration != 0)
			{
				// If the parameter binds directly to the result of applying a conversion function to the argument
				// expression, the implicit conversion sequence is a user-defined conversion sequence (13.3.3.1.2), with
				// the second standard conversion sequence either an identity conversion or, if the conversion function returns
				// an entity of a type that is a derived class of the parameter type, a derived-to-base Conversion.
				UniqueTypeWrapper type = overload.type;
				SYMBOLS_ASSERT(type.isReference());
				type.pop_front();
				bool isIdentity = type.value.getPointer() == to.value.getPointer();
				StandardConversionSequence second(isIdentity ? SCSRANK_IDENTITY : SCSRANK_CONVERSION, to.value.getQualifiers());
				second.isReference = true;
				return ImplicitConversion(second, ICSTYPE_USERDEFINED, overload.declaration);
			}
			// drop through...
		}

		// if not bound directly, a standard conversion is required (which produces an rvalue)
		if(!to.value.getQualifiers().isConst
			|| to.value.getQualifiers().isVolatile) // 8.5.3-5: otherwise, the reference shall be to a non-volatile const type
		{
			// can't bind rvalue to a non-const reference
			return ImplicitConversion(STANDARDCONVERSIONSEQUENCE_INVALID);
		}
		// drop through...
	}


	// [over.best.ics]
	// When the parameter type is not a reference, the implicit conversion sequence models a copy-initialization
	// of the parameter from the argument expression. The implicit conversion sequence is the one required to
	// convert the argument expression to an rvalue of the type of the parameter. [Note: when the parameter has a
	// class type, this is a conceptual conversion defined for the purposes of clause 13; the actual initialization is
	// defined in terms of constructors and is not a conversion. ] Any difference in top-level cv-qualification is
	// subsumed by the initialization itself and does not constitute a conversion. [Example: a parameter of type A
	// can be initialized from an argument of type const A. The implicit conversion sequence for that case is
	// the identity sequence; it contains no "conversion" from const A to A. ] When the parameter has a class
	// type and the argument expression has the same type, the implicit conversion sequence is an identity conversion.
	// When the parameter has a class type and the argument expression has a derived class type, the
	// implicit conversion sequence is a derived-to-base Conversion from the derived class to the base class.
	// [Note: there is no such standard conversion; this derived-to-base Conversion exists only in the description
	// of implicit conversion sequences. ] A derived-to-base Conversion has Conversion rank

	if(isClass(to))
	{
		// [dcl.init] If the destination type is a (possibly cv-qualified) class type: [..] where the cv-unqualified version
		// of the source type is the same class as, or a derived class of, the class of the destination, constructors
		// are considered.
		// TODO: perform overload resolution to choose a constructor
		// TODO: add implicit copy constructor if not already declared
		// for now, always allow conversion if 'from' is same or derived
		UniqueTypeWrapper matched = getExactMatchNoQualifiers(to, from.type);
		if(matched != gUniqueTypeNull)
		{
			return ImplicitConversion(StandardConversionSequence(SCSRANK_IDENTITY, CvQualifiers(), matched));
		}
		if(to.isSimple()
			&& from.type.isSimple()
			&& isBaseOf(getSimpleType(to.value), getSimpleType(from.type.value), context))
		{
			return ImplicitConversion(StandardConversionSequence(SCSRANK_CONVERSION, CvQualifiers()));
		}
	}

	if(!isUserDefinedConversion
		&& (isClass(to)
		|| isClass(from.type)))
	{

		// [over.ics.user]
		// A user-defined conversion sequence consists of an initial standard conversion sequence followed by a
		// user-defined conversion (12.3) followed by a second standard conversion sequence. If the user-defined
		// conversion is specified by a constructor (12.3.1), the initial standard conversion sequence converts the
		// source type to the type required by the argument of the constructor. If the user-defined conversion is specified
		// by a conversion function (12.3.2), the initial standard conversion sequence converts the source type to
		// the implicit object parameter of the conversion function.

		// [dcl.init]
		// Otherwise (i.e., for the remaining copy-initialization cases), user-defined conversion sequences that
		// can convert from the source type to the destination type or (when a conversion function is used) to a
		// derived class thereof are enumerated as described in 13.3.1.4, and the best one is chosen through
		// overload resolution (13.3). If the conversion cannot be done or is ambiguous, the initialization is
		// ill-formed.

		FunctionOverload overload = findBestConversionFunction(to, from, context, isNullPointerConstant);

		if(overload.declaration == 0)
		{
			return ImplicitConversion(STANDARDCONVERSIONSEQUENCE_INVALID);
		}

		// The second standard conversion sequence converts the result of the user-defined conversion to the target
		// type for the sequence.
		StandardConversionSequence second = makeStandardConversionSequence(to, removeReference(overload.type), context, false);
		second.isReference = isReference;
		return ImplicitConversion(second, ICSTYPE_USERDEFINED, overload.declaration);
	}

	from.type = selectOverloadedFunction(to, from, context);

	// standard conversion
	StandardConversionSequence sequence = makeStandardConversionSequence(to, from.type, context, isNullPointerConstant);
	sequence.isReference = isReference;
	return ImplicitConversion(sequence);
}

inline IcsRank getIcsRank(UniqueTypeWrapper to, UniqueTypeWrapper from, const InstantiationContext& context, bool isNullPointerConstant = false, bool isLvalue = false)
{
	ExpressionNodeGeneric<ExplicitTypeExpression> transientExpression = ExplicitTypeExpression(from);
	Argument argument = makeArgument(ExpressionWrapper(&transientExpression, false), ExpressionType(from, isLvalue));
	ImplicitConversion conversion = makeImplicitConversionSequence(to, argument, context, isNullPointerConstant);
	return getIcsRank(conversion.sequence.rank);
}

// [over.ics.rank]
inline bool isBetter(const ImplicitConversion& l, const ImplicitConversion& r)
{
	if(l.type != r.type)
	{
		return l.type < r.type;
	}
	// TODO: User-defined conversion sequence U1 is a better conversion sequence than another user-defined conversion
	// sequence U2 if they contain the same user-defined conversion function or constructor and if the
	// second standard conversion sequence of U1 is better than the second standard conversion sequence of
	// U2.
	return isBetter(l.sequence, r.sequence);
}

inline bool isValid(const ImplicitConversion& conversion)
{
	return conversion.sequence.rank != SCSRANK_INVALID;
}



typedef std::vector<ImplicitConversion> ArgumentConversions;

struct FunctionTemplate
{
	ParameterTypes parameters;
	UniqueTypeArray templateParameters;
	bool isStaticMember;
	FunctionTemplate() : isStaticMember(false)
	{
	}
	FunctionTemplate(const ParameterTypes& parameters, const UniqueTypeArray& templateParameters)
		: parameters(parameters), templateParameters(templateParameters), isStaticMember(false)
	{
	}
};

struct CandidateFunction : FunctionOverload, FunctionTemplate
{
	ArgumentConversions conversions;
	bool isTemplate;
	CandidateFunction()
		: FunctionOverload(0, gNullExpressionType)
	{
	}
	CandidateFunction(FunctionOverload overload, const FunctionTemplate& functionTemplate = FunctionTemplate())
		: FunctionOverload(overload), FunctionTemplate(functionTemplate), isTemplate(overload.declaration->isTemplate)
	{
	}
};

inline bool isMoreSpecialized(const FunctionTemplate& left, const FunctionTemplate& right)
{
	bool isMoreCvQualified = false;
	UniqueTypeArray leftDeduced(left.templateParameters.size(), gUniqueTypeNull);
	UniqueTypeArray rightDeduced(right.templateParameters.size(), gUniqueTypeNull);
	SYMBOLS_ASSERT(left.parameters.size() == right.parameters.size());
	UniqueTypeArray::const_iterator l = left.parameters.begin();
	UniqueTypeArray::const_iterator r = right.parameters.begin();
#if 0 // implicit object argument is no longer included in parameter list
	if(left.isStaticMember // if the left template is a static member
		|| right.isStaticMember) // or the right template is a static member
	{
		// ignore the first parameter
		++l;
		++r;
	}
#endif
	for(; l != left.parameters.end(); ++l, ++r)
	{
		UniqueTypeWrapper leftType = *l;
		UniqueTypeWrapper rightType = *r;
		// [temp.deduct.partial] (C++11 n3242)
		// Before the partial ordering is done, certain transformations are performed on the types used for partial
		// ordering:
		// - If P is a reference type, P is replaced by the type referred to.
		// - If A is a reference type, A is replaced by the type referred to.
		leftType = removeReference(leftType);
		rightType = removeReference(rightType);
		bool leftMoreCvQualified = isGreaterCvQualification(leftType, rightType);
		bool rightMoreCvQualified = isGreaterCvQualification(rightType, leftType);
		// Remove any top-level cv-qualifiers:
		// - If P is a cv-qualified type, P is replaced by the cv-unqualified version of P.
		// - If A is a cv-qualified type, A is replaced by the cv-unqualified version of A.
		leftType.value.setQualifiers(CvQualifiers());
		rightType.value.setQualifiers(CvQualifiers());

		// Using the resulting types P and A the deduction is then done [..]. If deduction succeeds
		// for a given type, the type from the argument template is considered to be at least as specialized as the type
		// from the parameter template.
		// If for each type being considered a given template is at least as specialized for all types and more specialized
		// for some set of types and the other template is not more specialized for any types or is not at least as
		// specialized for any types, then the given template is more specialized than the other template. Otherwise,
		// neither template is more specialized than the other.
		if(!deduceTemplateArguments(rightType, leftType, rightDeduced)) // if left is not at least as specialized as right
		{
			return false;
		}
		if(!deduceTemplateArguments(leftType, rightType, leftDeduced)) // if right is not at least as specialized as left
		{
			return true;
		}
		// If, for a given type, deduction succeeds in both directions (i.e., the types are identical after the transformations
		// above) and both P and A were reference types (before being replaced with the type referred to above):
		if((*l).isReference()
			&& (*r).isReference())
		{
			// - if the type from the argument template was an lvalue reference and the type from the parameter
			// template was not, the argument type is considered to be more specialized than the other; otherwise,
			// - if the type from the argument template is more cv-qualified than the type from the parameter template
			// (as described above), the argument type is considered to be more specialized than the other; otherwise,
			// - neither type is more specialized than the other.
			// TODO: rvalue reference?
			isMoreCvQualified |= leftMoreCvQualified;
			if(rightMoreCvQualified)
			{
				return false; // the template is only more specialized if the other template is not more specialized for any types.
			}
		}
	}
	return isMoreCvQualified; // after transformation, all parameters are identical; the template is more specialized if it is more specialized for any type.
}

// [over.match.best]
// a viable function F1 is defined to be a better function than another viable function
// F2 if for all arguments i, ICSi(F1) is not a worse conversion sequence than ICSi(F2), and then
// - for some argument j, ICSj(F1) is a better conversion sequence than ICSj(F2), or, if not that,
// - F1 is a non-template function and F2 is a function template specialization, or, if not that,
// - F1 and F2 are function template specializations, and the function template for F1 is more specialized
// - than the template for F2 according to the partial ordering rules
inline bool isBetter(const CandidateFunction& l, const CandidateFunction& r)
{
	SYMBOLS_ASSERT(l.conversions.size() == r.conversions.size());
	std::size_t first = 0;
	if(l.isStaticMember // if the left function is a static member
		|| r.isStaticMember) // or the right function is a static member
	{
		// [over.match.best]
		// If a function is a static member function [...] the first argument, the implied object parameter, has no effect
		// in the determination of whether the function is better or worse than any other function
		++first; // ignore the implicit object parameter when ranking
	}
	for(std::size_t i = first; i != l.conversions.size(); ++i)
	{
		if(isBetter(r.conversions[i], l.conversions[i]))
		{
			return false; // at least one argument is not a better conversion sequence
		}
	}
	for(std::size_t i = first; i != l.conversions.size(); ++i)
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
	if(l.isTemplate && r.isTemplate
		&& isMoreSpecialized(l, r))
	{
		return true;
	}
	// TODO: in context of initialisation by user defined conversion, consider return type
	return false;
}

const CandidateFunction gOverloadNull;

// TODO: fix circular dependency!
inline IntegralConstant evaluateExpression(const ExpressionWrapper& expression, const InstantiationContext& context);

struct OverloadResolver
{
	const Arguments& arguments;
	const TemplateArgumentsInstance* templateArguments;
	InstantiationContext context;
	CandidateFunction best;
	Declaration* ambiguous;
	bool isUserDefinedConversion;

	OverloadResolver(const Arguments& arguments, const TemplateArgumentsInstance* templateArguments, const InstantiationContext& context, bool isUserDefinedConversion = false)
		: arguments(arguments), templateArguments(templateArguments), context(context), ambiguous(0), isUserDefinedConversion(isUserDefinedConversion)
	{
		best.conversions.resize(arguments.size(), ImplicitConversion(STANDARDCONVERSIONSEQUENCE_INVALID));
	}
	const CandidateFunction& get() const
	{
		return ambiguous != 0 ? gOverloadNull : best;
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
	template<typename To>
	ImplicitConversion makeConversion(To to, const Argument& from)
	{
		// DR 903: a value-dependent expression may or may not be a null pointer constant, but the behaviour is unspecified.
		// simple fix: don't allow a value-dependent expression to be a null pointer constant.
		bool isNullPointerConstant = !from.isValueDependent && from.isConstant && evaluateExpression(from, context).value == 0;
		return makeImplicitConversionSequence(to, from, context, isNullPointerConstant, isUserDefinedConversion);
	}
	void add(const FunctionOverload& overload, const ParameterTypes& parameters, bool isEllipsis, CvQualifiers qualifiers, const SimpleType* memberEnclosing, FunctionTemplate& functionTemplate = FunctionTemplate())
	{
		bool hasImplicitObjectParameter = isMember(*overload.declaration)
			&& overload.declaration->type.declaration != &gCtor;

		CandidateFunction candidate(overload, functionTemplate);
		candidate.conversions.reserve(best.conversions.size());

		Arguments::const_iterator a = arguments.begin();

		if(hasImplicitObjectParameter)
		{
			// [over.match.funcs]
			// a member function is considered to have an extra parameter, called the implicit object parameter, which
			// represents the object for which the member function has been called. For the purposes of overload resolution,
			// both static and non-static member functions have an implicit object parameter, but constructors do not.
			SYMBOLS_ASSERT(isClass(*memberEnclosing->declaration));
			// For static member functions, the implicit object parameter is considered to match any object
			UniqueTypeWrapper implicitObjectParameter = gImplicitObjectParameter;
			if(!isStatic(*overload.declaration))
			{
				// For non-static member functions, the type of the implicit object parameter is "reference to cv X" where X is
				// the class of which the function is a member and cv is the cv-qualification on the member function declaration.
				// TODO: conversion-functions, non-conversions introduced by using-declaration
				implicitObjectParameter = makeUniqueSimpleType(*memberEnclosing);
				implicitObjectParameter.value.setQualifiers(qualifiers);
				implicitObjectParameter.push_front(ReferenceType());
			}
			else
			{
				candidate.isStaticMember = true;
			}

			SYMBOLS_ASSERT(a != arguments.end());
			const Argument& impliedObjectArgument = *a++;
			if(!isStatic(*overload.declaration))
			{
				// [over.match.funcs]
				// even if the implicit object parameter is not const-qualified, an rvalue temporary can be bound to the
				// parameter as long as in all other respects the temporary can be converted to the type of the implicit
				// object parameter.
				Argument tmpObjectArgument = impliedObjectArgument;
				tmpObjectArgument.type = ExpressionType(tmpObjectArgument.type, true); // treat as lvalue to allow binding reference to temporary
				candidate.conversions.push_back(makeImplicitConversionSequence(implicitObjectParameter, tmpObjectArgument, context));
			}
			else
			{
				// TODO: [over.match.funcs] static members:
				// - no temporary object can be introduced to hold the argument for the implicit object parameter;
				// - no user-defined conversions can be applied to achieve a type match with it; and
				// - even if the implicit object parameter is not const-qualified, an rvalue temporary can be bound to the
				//   parameter as long as in all other respects the temporary can be converted to the type of the implicit
				//   object parameter.
				candidate.conversions.push_back(ImplicitConversion(StandardConversionSequence(SCSRANK_IDENTITY, CvQualifiers())));
			}
		}

		if(arguments.size() < parameters.size() + hasImplicitObjectParameter)
		{
			if(overload.declaration == &gUnknown)
			{
				return; // TODO: don't include built-in operator candidates with wrong number of arguments
			}
			std::size_t argumentCount = arguments.end() - a;
			const Parameters& defaults = getParameters(overload.declaration->type);
			Parameters::const_iterator d = defaults.begin();
			std::advance(d, argumentCount);
			for(ParameterTypes::const_iterator i = parameters.begin() + argumentCount; i != parameters.end(); ++i)
			{
				SYMBOLS_ASSERT(d != defaults.end());
				if((*d).defaultArgument == 0) // TODO: catch this earlier
				{
					return; // [over.match.viable] no default-argument available, this candidate is not viable
				}
				else
				{
					SYMBOLS_ASSERT((*d).defaultArgument->expr != 0); // TODO: non-fatal error: trying to use a default-argument before it has been declared. 
				}
				++d;
			}
		}

		for(ParameterTypes::const_iterator p = parameters.begin(); a != arguments.end() && p != parameters.end(); ++a, ++p)
		{
			candidate.conversions.push_back(makeConversion(*p, *a)); // TODO: l-value
		}

		// [over.match.viable]
		// A candidate function having fewer than m parameters is viable only if it has an ellipsis in its parameter
		// list. For the purposes of overload resolution, any argument for which there is no corresponding
		// parameter is considered to "match the ellipsis"
		if(!isEllipsis
			&& a != arguments.end())
		{
			return;
		}
		for(; a != arguments.end(); ++a)
		{
			candidate.conversions.push_back(IMPLICITCONVERSION_ELLIPSIS);
		}

		add(candidate);
	}
};



struct Overload
{
	const Declaration* declaration;
	const SimpleType* memberEnclosing;
	Overload(const Declaration* declaration, const SimpleType* memberEnclosing)
		: declaration(declaration), memberEnclosing(memberEnclosing)
	{
	}
};

inline bool operator==(const Overload& left, const Overload& right)
{
	return left.declaration == right.declaration
		&& left.memberEnclosing == right.memberEnclosing;
}

struct FunctionSignature : FunctionType, FunctionTemplate
{
	UniqueTypeWrapper returnType;
	CvQualifiers qualifiers;

	const FunctionType& getFunctionType() const
	{
		return *this;
	}
};

FunctionSignature substituteFunctionId(const Overload& overload, const UniqueTypeArray& argumentTypes, const TemplateArgumentsInstance* templateArguments, const InstantiationContext& context);
ParameterTypes addOverload(OverloadResolver& resolver, const Overload& overload);



template<typename To>
inline void addConversionFunctionOverloads(OverloadResolver& resolver, const SimpleType& classType, To to)
{
	InstantiationContext& context = resolver.context;
	instantiateClass(classType, context); // searching for overloads requires a complete type
	Identifier id;
	extern Name gConversionFunctionId; // TODO
	id.value = gConversionFunctionId;
	LookupResultRef declaration = ::findDeclaration(classType, id, IsAny());
	// TODO: conversion functions in base classes should not be hidden by those in derived
	if(declaration != 0)
	{
		const SimpleType* memberEnclosing = findEnclosingType(&classType, declaration->scope); // find the base class which contains the member-declaration
		SYMBOLS_ASSERT(memberEnclosing != 0);

		for(Declaration* p = findOverloaded(declaration); p != 0; p = p->overloaded)
		{
			SYMBOLS_ASSERT(p->enclosed != 0);

			SYMBOLS_ASSERT(!p->isTemplate); // TODO: template-argument-deduction for conversion function
			// 'template<typename T> operator T()' can be explicitly invoked with template argument list: e.g. 'x.operator int()'
			// [temp.deduct.conv] Template argument deduction is done by comparing the return type of the template conversion function
			// (call it P) with the type that is required as the result of the conversion (call it A)

			UniqueTypeWrapper result = getUniqueType(p->type, setEnclosingTypeSafe(context, memberEnclosing));
			result.pop_front(); // T() -> T
			ExpressionType yielded = getFunctionCallExpressionType(result);

			if(isClass(to)) // [over.match.copy]
			{
				// When the type of the initializer expression is a class type "cv S", the conversion functions of S and its
				// base classes are considered. Those that are not hidden within S and yield a type whose cv-unqualified
				// version is the same type as T or is a derived class thereof are candidate functions. Conversion functions
				// that return "reference to X" return lvalues of type X and are therefore considered to yield X for this process
				// of selecting candidate functions.
				if(!(getExactMatch(to, removeReference(yielded)) != gUniqueTypeNull
					|| isBaseOf(to, removeReference(yielded), context)))
				{
					continue;
				}
			}
			else if(to.isReference()) // [over.match.ref]
			{
				// Assuming that "cv1 T" is the underlying type of the reference being initialized,
				// and "cv S" is the type of the initializer expression, with S a class type, the candidate functions are
				// selected as follows:
				UniqueTypeWrapper tmpTo = to;
				tmpTo.pop_front(); // remove reference
				// The conversion functions of S and its base classes are considered. Those that are not hidden within S
				// and yield type "reference to cv2 T2", where "cv1 T" is reference-compatible with "cv2 T2", are
				// candidate functions.
				if(!yielded.isReference())
				{
					continue;
				}
				yielded.pop_front(); // remove reference
				if(!((getExactMatch(tmpTo, yielded) != gUniqueTypeNull
					|| isBaseOf(tmpTo, yielded, context))
					&& (isEqualCvQualification(tmpTo, yielded)
					|| isGreaterCvQualification(tmpTo, yielded))))
				{
					continue;
				}
			}
			else // [over.match.conv]
			{
				// The conversion functions of S and its base classes are considered. Those that are not hidden within S
				// and yield type T or a type that can be converted to type T via a standard conversion sequence
				// are candidate functions. Conversion functions that return a cv-qualified type are considered
				// to yield the cv-unqualified version of that type for this process of selecting candidate functions. Conversion
				// functions that return "reference to cv2 X" return lvalues of type "cv2 X" and are therefore considered
				// to yield X for this process of selecting candidate functions.
				bool isLvalue = yielded.isReference(); // TODO: lvalueness!
				yielded = removeReference(yielded);
				yielded.value.setQualifiers(CvQualifiers());
				if(makeStandardConversionSequence(to, yielded, context, false).rank == SCSRANK_INVALID)
				{
					continue;
				}
			}

			addOverload(resolver, Overload(p, memberEnclosing));
		}
	}
}

template<typename To>
FunctionOverload findBestConversionFunction(To to, Argument from, const InstantiationContext& context, bool isNullPointerConstant)
{
	Arguments arguments;
	arguments.push_back(from);

	// [over.best.ics]
	// However, when considering the argument of a user-defined conversion function that is a candidate by
	// 13.3.1.3 when invoked for the copying of the temporary in the second step of a class copy-initialization, or
	// by 13.3.1.4, 13.3.1.5, or 13.3.1.6 in all cases, only standard conversion sequences and ellipsis conversion
	// sequences are allowed.
	OverloadResolver resolver(arguments, 0, context, true); // disallow user-defined conversion when considering argument to conversion function

	// [dcl.init]\14
	if(isClass(to)
		&& isComplete(to)) // can only convert to a class that is complete
	{
		// add converting constructors of 'to'
		const SimpleType& classType = getSimpleType(to.value);
		instantiateClass(classType, context); // searching for overloads requires a complete type
		Identifier tmp;
		tmp.value = classType.declaration->getName().value;
		tmp.source = context.source;
		LookupResultRef declaration = ::findDeclaration(classType, tmp, IsConstructor());

		if(declaration != 0) // TODO: add implicit copy constructor!
		{
			const SimpleType* memberEnclosing = findEnclosingType(&classType, declaration->scope); // find the base class which contains the member-declaration
			SYMBOLS_ASSERT(memberEnclosing != 0);

			for(Declaration* p = findOverloaded(declaration); p != 0; p = p->overloaded)
			{
				SYMBOLS_ASSERT(p->enclosed != 0);

				// [class.conv.ctor]
				// A constructor declared without the function-specifier explicit that can be called with a single parameter
				// specifies a conversion from the type of its first parameter to the type of its class. Such a constructor is
				// called a converting constructor.
				if(declaration->specifiers.isExplicit)
				{
					continue;
				}

				addOverload(resolver, Overload(p, memberEnclosing)); // will reject constructors that cannot be called with a single argument, because they are not viable.
			}
		}
	}

	if(isClass(from.type)
		&& isComplete(from.type)) // can only convert from a class that is complete
	{
		addConversionFunctionOverloads(resolver, getSimpleType(from.type.value), to);
	}

	// TODO: return-type of constructor should be 'to'
	FunctionOverload result = resolver.get();
	if(result.declaration != 0
		&& result.type.isSimple()
		&& getSimpleType(result.type.value).declaration == &gCtor)
	{
		result.type = getFunctionCallExpressionType(to);
		result.type.value.setQualifiers(CvQualifiers());
	}
	return result;
}


#endif
