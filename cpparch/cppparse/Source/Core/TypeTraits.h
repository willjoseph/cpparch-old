
#ifndef INCLUDED_CPPPARSE_CORE_TYPETRAITS_H
#define INCLUDED_CPPPARSE_CORE_TYPETRAITS_H

#include "TypeInstantiate.h"
#include "Fundamental.h"

template<typename T>
inline Name getTypeTraitName(T* symbol)
{
	return symbol->trait->value.value;
}

inline bool hasNothrowConstructor(UniqueTypeWrapper type)
{
	return true; // TODO
}

inline bool hasNothrowCopy(UniqueTypeWrapper type)
{
	return true; // TODO
}

inline bool hasTrivialAssign(UniqueTypeWrapper type)
{
	return true; // TODO
}

inline bool hasTrivialConstructor(UniqueTypeWrapper type)
{
	return true; // TODO
}

inline bool hasTrivialCopy(UniqueTypeWrapper type)
{
	return true; // TODO
}

inline bool hasTrivialDestructor(UniqueTypeWrapper type)
{
	return true; // TODO
}

inline bool hasVirtualDestructor(UniqueTypeWrapper type)
{
	return false; // TODO
}

inline bool isAbstract(UniqueTypeWrapper type)
{
	return false; // TODO
}

inline bool isEmpty(UniqueTypeWrapper type)
{
	return false; // TODO
}

inline bool isPod(UniqueTypeWrapper type)
{
	return false; // TODO
}

inline bool isPolymorphic(UniqueTypeWrapper type)
{
	return false; // TODO
}

inline bool isUnion(UniqueTypeWrapper type)
{
	return false; // TODO
}

inline bool isConvertibleTo(UniqueTypeWrapper type, UniqueTypeWrapper other, const InstantiationContext& context)
{
	return false; // TODO
}


inline UnaryTypeTraitsOp getUnaryTypeTraitsOp(cpp::typetraits_unary* symbol)
{
	switch(symbol->id)
	{
	case cpp::typetraits_unary::HAS_NOTHROW_CONSTRUCTOR: return hasNothrowConstructor;
	case cpp::typetraits_unary::HAS_NOTHROW_COPY: return hasNothrowCopy;
	case cpp::typetraits_unary::HAS_TRIVIAL_ASSIGN: return hasTrivialAssign;
	case cpp::typetraits_unary::HAS_TRIVIAL_CONSTRUCTOR: return hasTrivialConstructor;
	case cpp::typetraits_unary::HAS_TRIVIAL_COPY: return hasTrivialCopy;
	case cpp::typetraits_unary::HAS_TRIVIAL_DESTRUCTOR: return hasTrivialDestructor;
	case cpp::typetraits_unary::HAS_VIRTUAL_DESTRUCTOR: return hasVirtualDestructor;
	case cpp::typetraits_unary::IS_ABSTRACT: return isAbstract;
	case cpp::typetraits_unary::IS_CLASS: return isClass;
	case cpp::typetraits_unary::IS_EMPTY: return isEnum;
	case cpp::typetraits_unary::IS_ENUM: return isEmpty;
	case cpp::typetraits_unary::IS_POD: return isPod;
	case cpp::typetraits_unary::IS_POLYMORPHIC: return isPolymorphic;
	case cpp::typetraits_unary::IS_UNION: return isUnion;
	default: break;
	}
	throw SymbolsError();
}

inline BinaryTypeTraitsOp getBinaryTypeTraitsOp(cpp::typetraits_binary* symbol)
{
	switch(symbol->id)
	{
	case cpp::typetraits_binary::IS_BASE_OF: return isBaseOf;
	case cpp::typetraits_binary::IS_CONVERTIBLE_TO: return isConvertibleTo;
	default: break;
	}
	throw SymbolsError();
}

#endif
