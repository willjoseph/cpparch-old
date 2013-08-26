
#ifndef INCLUDED_CPPPARSE_CORE_TYPEINSTANTIATE_H
#define INCLUDED_CPPPARSE_CORE_TYPEINSTANTIATE_H

#include "Ast/Type.h"
#include "NameLookup.h"
#include "Ast/Print.h"

std::size_t instantiateClass(const SimpleType& instanceConst, const InstantiationContext& context, bool allowDependent = false);


inline std::size_t requireCompleteObjectType(UniqueTypeWrapper type, const InstantiationContext& context)
{
	std::size_t count = 1;
	while(type.isArray()
		&& getArrayType(type.value).size != 0)
	{
		count *= getArrayType(type.value).size;
		type.pop_front(); // arrays of known size are complete object types
	}
	if(type.isSimple())
	{
		const SimpleType& objectType = getSimpleType(type.value);
		if(isClass(*objectType.declaration))
		{
			return instantiateClass(objectType, context) * count;
		}
	}
	return 4; // TODO: size of non-object types
}



inline bool findBase(const SimpleType& other, const SimpleType& type)
{
	SYMBOLS_ASSERT(other.declaration->enclosed != 0);
	SYMBOLS_ASSERT(isClass(*type.declaration));
	for(UniqueBases::const_iterator i = other.bases.begin(); i != other.bases.end(); ++i)
	{
		const SimpleType& base = *(*i);
		SYMBOLS_ASSERT(isClass(*base.declaration));
		if(&base == &type)
		{
			return true;
		}
		if(findBase(base, type))
		{
			return true;
		}
	}
	return false;
}

// Returns true if 'type' is a base of 'other'
inline bool isBaseOf(const SimpleType& type, const SimpleType& other, const InstantiationContext& context)
{
	if(!isClass(*type.declaration)
		|| !isClass(*other.declaration))
	{
		return false;
	}
	if(isIncomplete(*type.declaration)
		|| isIncomplete(*other.declaration))
	{
		return false;
	}
	instantiateClass(other, context);
	return findBase(other, type);
}


inline bool isBaseOf(UniqueTypeWrapper base, UniqueTypeWrapper derived, const InstantiationContext& context)
{
	if(!base.isSimple()
		|| !derived.isSimple())
	{
		return false;
	}
	const SimpleType& baseType = getSimpleType(base.value);
	const SimpleType& derivedType = getSimpleType(derived.value);
	if(&baseType == &derivedType)
	{
		return true;
	}
	SYMBOLS_ASSERT(!isClass(*derivedType.declaration) || !isIncomplete(*derivedType.declaration)); // TODO: does SFINAE apply?
	return isBaseOf(baseType, derivedType, context);
}

#endif
