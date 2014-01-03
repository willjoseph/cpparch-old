
#ifndef INCLUDED_CPPPARSE_CORE_TYPECONVERT_H
#define INCLUDED_CPPPARSE_CORE_TYPECONVERT_H

#include "Fundamental.h"
#include "TypeInstantiate.h"

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
	bool isReference;
	UniqueTypeWrapper matched;
	StandardConversionSequence(ScsRank rank, CvQualifiers adjustment, UniqueTypeWrapper matched = gUniqueTypeNull)
		: rank(rank), adjustment(adjustment), isReference(false), matched(matched)
	{
	}
};

const StandardConversionSequence STANDARDCONVERSIONSEQUENCE_INVALID = StandardConversionSequence(SCSRANK_INVALID, CvQualifiers());

// 4.5 Integral Promotions
// TODO: handle bitfield types?
inline UniqueTypeWrapper promoteToIntegralType(UniqueTypeWrapper type)
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


// 5 Expressions
// paragraph 9: usual arithmetic conversions
// [expr]
// Many binary operators that expect operands of arithmetic or enumeration type cause conversions and yield
// result types in a similar way. The purpose is to yield a common type, which is also the type of the result.
// This pattern is called the usual arithmetic conversions
inline BuiltInType usualArithmeticConversions(UniqueTypeWrapper left, UniqueTypeWrapper right)
{
	SYMBOLS_ASSERT(left != gUniqueTypeNull);
	SYMBOLS_ASSERT(right != gUniqueTypeNull);

	SYMBOLS_ASSERT(isArithmetic(left) || isEnumeration(left));
	SYMBOLS_ASSERT(isArithmetic(right) || isEnumeration(right));

	if(isEqual(left, gLongDouble)
		|| isEqual(right, gLongDouble))
	{
		return gLongDouble;
	}
	if(isEqual(left, gDouble)
		|| isEqual(right, gDouble))
	{
		return gDouble;
	}
	if(isEqual(left, gFloat)
		|| isEqual(right, gFloat))
	{
		return gFloat;
	}
	left = promoteToIntegralType(left);
	right = promoteToIntegralType(right);
	if(isEqual(left, gUnsignedLongInt)
		|| isEqual(right, gUnsignedLongInt))
	{
		return gUnsignedLongInt;
	}
	if((isEqual(left, gSignedLongInt)
		&& isEqual(right, gUnsignedInt))
		|| (isEqual(left, gUnsignedInt)
		&& isEqual(right, gSignedLongInt)))
	{
		return gUnsignedLongInt;
	}
	if(isEqual(left, gSignedLongInt)
		|| isEqual(right, gSignedLongInt))
	{
		return gSignedLongInt;
	}
	if(isEqual(left, gUnsignedInt)
		|| isEqual(right, gUnsignedInt))
	{
		return gUnsignedInt;
	}
	return gSignedInt;
}

// T[] -> T*
inline UniqueTypeWrapper applyArrayToPointerConversion(UniqueTypeWrapper type)
{
	SYMBOLS_ASSERT(type.isArray());
	type.pop_front();
	type.push_front(PointerType());
	return type;
}
inline ExpressionType applyArrayToPointerConversion(ExpressionType type)
{
	return ExpressionType(applyArrayToPointerConversion(UniqueTypeWrapper(type)), false);
}

// T() -> T(*)()
inline UniqueTypeWrapper applyFunctionToPointerConversion(UniqueTypeWrapper type)
{
	SYMBOLS_ASSERT(type.isFunction());
	type.push_front(PointerType());
	return type;
}
inline ExpressionType applyFunctionToPointerConversion(ExpressionType type)
{
	return ExpressionType(applyFunctionToPointerConversion(UniqueTypeWrapper(type)), false);
}

inline ExpressionType applyLvalueToRvalueConversion(ExpressionType type)
{
	if(type.isArray())
	{
		return applyArrayToPointerConversion(type);
	}
	if(type.isFunction())
	{
		return applyFunctionToPointerConversion(type);
	}
	return ExpressionType(type, false); // non lvalue
}

inline CvQualifiers makeQualificationAdjustment(UniqueTypeId to, UniqueTypeId from)
{
	return CvQualifiers(to.value.getQualifiers().isConst > from.value.getQualifiers().isConst,
		to.value.getQualifiers().isVolatile > from.value.getQualifiers().isVolatile);
}


struct TargetType : UniqueTypeWrapper
{
	TargetType(UniqueTypeWrapper type)
		: UniqueTypeWrapper(type)
	{
	}
};

inline UniqueTypeWrapper getExactMatch(UniqueTypeWrapper to, UniqueTypeWrapper from)
{
	return isEqual(to, from) ? from : gUniqueTypeNull;
}

inline UniqueTypeWrapper getExactMatch(TargetType to, UniqueTypeWrapper from)
{
	if(to == gPromotedArithmeticPlaceholder
		&& isPromotedArithmetic(from))
	{
		return from;
	}
	if(to == gArithmeticPlaceholder
		&& isArithmetic(from))
	{
		return from;
	}
	if(to == gIntegralPlaceholder
		&& isIntegral(from))
	{
		return from;
	}
	if(to == gPromotedIntegralPlaceholder
		&& isPromotedIntegral(from))
	{
		return from;
	}
	if(to == gEnumerationPlaceholder
		&& isEnumeration(from))
	{
		return from;
	}
	if(from.isPointer())
	{
		if(to == gPointerToAnyPlaceholder)
		{
			return from;
		}
		if(to == gPointerToObjectPlaceholder
			&& isObject(popType(from)))
		{
			return from;
		}
		if(to == gPointerToClassPlaceholder
			&& isClass(popType(from)))
		{
			return from;
		}
		if(to == gPointerToFunctionPlaceholder
			&& popType(from).isFunction())
		{
			return from;
		}
	}
	if(to == gPointerToMemberPlaceholder
		&& from.isMemberPointer())
	{
		return from;
	}
	return getExactMatch(UniqueTypeWrapper(to), from);
}

template<typename To>
inline UniqueTypeWrapper getExactMatchNoQualifiers(To to, UniqueTypeWrapper from)
{
	to.value.setQualifiers(CvQualifiers());
	from.value.setQualifiers(CvQualifiers());
	return getExactMatch(to, from);
}

inline bool isArithmetic(TargetType to)
{
	if(to == gArithmeticPlaceholder
		|| to == gPromotedArithmeticPlaceholder
		|| to == gIntegralPlaceholder
		|| to == gPromotedIntegralPlaceholder)
	{
		return true;
	}
	return isArithmetic(UniqueTypeWrapper(to));
}

template<typename To>
inline StandardConversionSequence makeScsPromotion(To to, UniqueTypeWrapper from)
{
	if(isArithmetic(from)
		&& isArithmetic(to))
	{
		if(from == gFloat)
		{
			UniqueTypeWrapper matched = getExactMatch(to, gDouble);
			if(matched != gUniqueTypeNull)
			{
				return StandardConversionSequence(SCSRANK_PROMOTION, CvQualifiers(), matched);
			}
		}
		UniqueTypeWrapper matched = getExactMatch(to, promoteToIntegralType(from));
		if(matched != gUniqueTypeNull)
		{
			return StandardConversionSequence(SCSRANK_PROMOTION, CvQualifiers(), matched);
		}	
	}
	if(isEnum(from)
		&& getExactMatch(to, gSignedInt) != gUniqueTypeNull) // TODO: correct type of integral promotion for enum
	{
		return StandardConversionSequence(SCSRANK_PROMOTION, CvQualifiers(), gSignedInt);
	}
	return STANDARDCONVERSIONSEQUENCE_INVALID;
}

inline bool isGeneralPointer(UniqueTypeWrapper type)
{
	return type.isPointer() || type.isMemberPointer();
}

inline bool isPointerPlaceholder(UniqueTypeWrapper type)
{
	return type == gPointerToAnyPlaceholder
		|| type == gPointerToObjectPlaceholder
		|| type == gPointerToClassPlaceholder
		|| type == gPointerToFunctionPlaceholder;
}

inline bool isGeneralPointer(TargetType type)
{
	return isPointerPlaceholder(type)
		|| type == gPointerToMemberPlaceholder
		|| isGeneralPointer(UniqueTypeWrapper(type));
}

template<typename To>
inline StandardConversionSequence makeScsConversion(const InstantiationContext& context, To to, UniqueTypeWrapper from, bool isNullPointerConstant = false)
{
	SYMBOLS_ASSERT(to.value.getQualifiers() == CvQualifiers());
	SYMBOLS_ASSERT(from.value.getQualifiers() == CvQualifiers());
	if((isArithmetic(from) || isEnumeration(from))
		&& isArithmetic(to))
	{
		// can convert from enumeration to integer/floating/bool, but not in reverse
		return StandardConversionSequence(SCSRANK_CONVERSION, CvQualifiers(), gSignedInt); // TODO: correct type of integral conversion
	}
	if(isGeneralPointer(to)
		&& isIntegral(from)
		&& isNullPointerConstant)
	{
		return StandardConversionSequence(SCSRANK_CONVERSION, CvQualifiers()); // 0 -> T*, 0 -> T C::*
	}
	if(to.isSimplePointer()
		&& from.isSimplePointer()
		&& getInner(to.value).getPointer() == gVoid.value.getPointer()) // ignore cv-qualifiers here!
	{
		to.pop_front();
		from.pop_front();
		return isEqualCvQualification(to, from) || isGreaterCvQualification(to, from)
			? StandardConversionSequence(SCSRANK_CONVERSION, makeQualificationAdjustment(to, from)) // T* -> void*
			: STANDARDCONVERSIONSEQUENCE_INVALID;
	}
	if(to.isSimplePointer()
		&& from.isSimplePointer()
		&& isBaseOf(getSimpleType(getInner(to.value)), getSimpleType(getInner(from.value)), context))
	{
		to.pop_front();
		from.pop_front();
		return isEqualCvQualification(to, from) || isGreaterCvQualification(to, from)
			? StandardConversionSequence(SCSRANK_CONVERSION, makeQualificationAdjustment(to, from)) // D* -> B*
			: STANDARDCONVERSIONSEQUENCE_INVALID;
	}
	if(to.isMemberPointer()
		&& from.isMemberPointer()
		&& isBaseOf(getMemberPointerClass(to.value), getMemberPointerClass(from.value), context))
	{
		return StandardConversionSequence(SCSRANK_CONVERSION, CvQualifiers()); // T D::* -> T B::*
	}
	if(to == gBool
		&& (from.isPointer() || from.isMemberPointer()))
	{
		return StandardConversionSequence(SCSRANK_CONVERSION, CvQualifiers()); // T* -> bool, T C::* -> bool
	}
	if(to.isSimple()
		&& from.isSimple()
		&& isBaseOf(getSimpleType(to.value), getSimpleType(from.value), context))
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

template<typename To>
inline StandardConversionSequence makeScsExactMatch(To target, UniqueTypeWrapper from)
{
	UniqueTypeWrapper matched = getExactMatchNoQualifiers(target, from);
	UniqueTypeWrapper to = target;
	for(;;)
	{
		if(matched != gUniqueTypeNull)
		{
			return isEqualCvQualification(to, from) || isGreaterCvQualification(to, from)
				? StandardConversionSequence(SCSRANK_EXACT, makeQualificationAdjustment(to, from), matched)
				: STANDARDCONVERSIONSEQUENCE_INVALID;
		}
		if(to.isPointer()
			&& from.isPointer())
		{
		}
		else if(to.isMemberPointer()
			&& from.isMemberPointer()
			&& getMemberPointerType(to.value).type == getMemberPointerType(from.value).type)
		{
		}
		else
		{
			break;
		}
		to.pop_front();
		from.pop_front();
		matched = getExactMatchNoQualifiers(to, from);
	}
	return STANDARDCONVERSIONSEQUENCE_INVALID;
}

// 13.3.3.1 [over.best.ics]
template<typename To>
inline StandardConversionSequence makeStandardConversionSequence(To to, ExpressionType from, const InstantiationContext& context, bool isNullPointerConstant = false)
{
	SYMBOLS_ASSERT(to != gUniqueTypeNull);
	SYMBOLS_ASSERT(from != gUniqueTypeNull);

	// ignore top level cv-qualifiers
	to.value.setQualifiers(CvQualifiers());
	from.value.setQualifiers(CvQualifiers());

	UniqueTypeWrapper tmp = from;
	from = applyLvalueToRvalueConversion(from); // TODO: lvalue-ness

	if(tmp == from // no l-value to rvalue conversion
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
		StandardConversionSequence result = makeScsConversion(context, to, from, isNullPointerConstant); // TODO: ordering of conversions by inheritance distance
		if(result.rank != SCSRANK_INVALID)
		{
			return result;
		}
	}

	return STANDARDCONVERSIONSEQUENCE_INVALID;
}

inline bool isProperSubsequence(CvQualifiers l, CvQualifiers r)
{
	return !l.isConst && !l.isVolatile
		&& (r.isConst || r.isVolatile); // true if r has any cv-qualification while l does not
}

inline bool isProperSubsequence(const StandardConversionSequence& l, const StandardConversionSequence& r)
{
	// TODO: consider lvalue-transformation!
	return (l.rank == SCSRANK_IDENTITY && r.rank != SCSRANK_IDENTITY)
		|| (l.rank == r.rank
		&& !l.isReference && !r.isReference && isProperSubsequence(l.adjustment, r.adjustment)); // TODO: compare cv-qualification signature of multi-level pointer
}

// [over.ics.rank]
// Standard conversion sequence S1 is a better conversion sequence than standard conversion sequence
// S2 if
// - S1 is a proper subsequence of S2 (comparing the conversion sequences in the canonical form
//   defined by 13.3.3.1.1, excluding any Lvalue Transformation; the identity conversion sequence is
//   considered to be a subsequence of any non-identity conversion sequence) or, if not that,
// - the rank of S1 is better than the rank of S2, or S1 and S2 have the same rank and are distinguishable
//   by the rules in the paragraph below, or, if not that,
// - S1 and S2 differ only in their qualification conversion and yield similar types T1 and T2 (4.4),
//   respectively, and the cv-qualification signature of type T1 is a proper subset of the cv-qualification
//   signature of type T2.
// - S1 and S2 are reference bindings (8.5.3), and the types to which the references refer are the same
//   type except for top-level cv-qualifiers, and the type to which the reference initialized by S2 refers
//   is more cv-qualified than the type to which the reference initialized by S1 refers.
// - User-defined conversion sequence U1 is a better conversion sequence than another user-defined conversion
//   sequence U2 if they contain the same user-defined conversion function or constructor and if the
//   second standard conversion sequence of U1 is better than the second standard conversion sequence of
//   U2.
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
	if(l.isReference && r.isReference
		&& isGreaterCvQualification(r.adjustment, l.adjustment))
	{
		return true; // both sequences are similar references, and the second is more cv-qualified than the first.
	}
	// TODO: user-defined conversion sequence ranking
	return false;
}

enum IcsType
{
	ICSTYPE_STANDARD,
	ICSTYPE_USERDEFINED,
	ICSTYPE_ELLIPSIS,
};

inline IcsRank getIcsRank(ScsRank rank)
{
	switch(rank)
	{
	case SCSRANK_IDENTITY:
	case SCSRANK_EXACT: return ICSRANK_STANDARDEXACT;
	case SCSRANK_PROMOTION: return ICSRANK_STANDARDPROMOTION;
	case SCSRANK_CONVERSION: return ICSRANK_STANDARDCONVERSION;
	}
	return ICSRANK_INVALID;
}


#endif
