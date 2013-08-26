
#ifndef INCLUDED_CPPPARSE_CORE_TYPESUBSTITUTE_H
#define INCLUDED_CPPPARSE_CORE_TYPESUBSTITUTE_H

#include "Ast/Type.h"
#include "Ast/Print.h"
#include "TypeInstantiate.h"


UniqueTypeWrapper substitute(UniqueTypeWrapper dependent, const InstantiationContext& context);
void substitute(UniqueTypeArray& substituted, const UniqueTypeArray& dependent, const InstantiationContext& context);

// ----------------------------------------------------------------------------

struct TypeError
{
	virtual void report() = 0;
};


struct TypeErrorBase : TypeError
{
	Location source;
	TypeErrorBase(Location source) : source(source)
	{
	}
	void report()
	{
		printPosition(source);
	}
};

struct MemberNotFoundError : TypeErrorBase
{
	Name name;
	UniqueTypeWrapper qualifying;
	MemberNotFoundError(Location source, Name name, UniqueTypeWrapper qualifying)
		: TypeErrorBase(source), name(name), qualifying(qualifying)
	{
	}
	void report()
	{
		TypeErrorBase::report();
		std::cout << "member '" << name.c_str() << "' not found in ";
#if 1
		if(getSimpleType(qualifying.value).instantiating)
		{
			std::cout << "(partially instantiated) ";
		}
#endif
		printType(qualifying);
		std::cout << std::endl;
	}
};

struct MemberIsNotTypeError : TypeErrorBase
{
	Name name;
	UniqueTypeWrapper qualifying;
	MemberIsNotTypeError(Location source, Name name, UniqueTypeWrapper qualifying)
		: TypeErrorBase(source), name(name), qualifying(qualifying)
	{
	}
	void report()
	{
		TypeErrorBase::report();
		std::cout << "member '" << name.c_str() << "' is not a type in ";
		printType(qualifying);
		std::cout << std::endl;
	}
};

struct ExpectedTemplateTemplateArgumentError : TypeErrorBase
{
	UniqueTypeWrapper type;
	ExpectedTemplateTemplateArgumentError(Location source, UniqueTypeWrapper type)
		: TypeErrorBase(source), type(type)
	{
	}
	void report()
	{
		TypeErrorBase::report();
		std::cout << "expected template template argument: ";
		printType(type);
		std::cout << std::endl;
	}
};

struct MismatchedTemplateTemplateArgumentError : TypeErrorBase
{
	UniqueTypeWrapper type;
	MismatchedTemplateTemplateArgumentError(Location source, UniqueTypeWrapper type)
		: TypeErrorBase(source), type(type)
	{
	}
	void report()
	{
		TypeErrorBase::report();
		std::cout << "mismatched template template argument: ";
		printType(type);
		std::cout << std::endl;
	}
};

struct QualifyingIsNotClassError : TypeErrorBase
{
	UniqueTypeWrapper qualifying;
	QualifyingIsNotClassError(Location source, UniqueTypeWrapper qualifying)
		: TypeErrorBase(source), qualifying(qualifying)
	{
	}
	void report()
	{
		TypeErrorBase::report();
		std::cout << "qualifying type is not a class: ";
		printType(qualifying);
		std::cout << std::endl;
	}
};

struct PointerToReferenceError : TypeErrorBase
{
	PointerToReferenceError(Location source)
		: TypeErrorBase(source)
	{
	}
	void report()
	{
		TypeErrorBase::report();
		std::cout << "cannot create pointer to reference" << std::endl;
	}
};

struct ReferenceToReferenceError : TypeErrorBase
{
	ReferenceToReferenceError(Location source)
		: TypeErrorBase(source)
	{
	}
	void report()
	{
		TypeErrorBase::report();
		std::cout << "cannot create reference to reference" << std::endl;
	}
};

struct InvalidArrayError : TypeErrorBase
{
	InvalidArrayError(Location source)
		: TypeErrorBase(source)
	{
	}
	void report()
	{
		TypeErrorBase::report();
		std::cout << "cannot create array of type void, function or reference" << std::endl;
	}
};

struct VoidParameterError : TypeErrorBase
{
	VoidParameterError(Location source)
		: TypeErrorBase(source)
	{
	}
	void report()
	{
		TypeErrorBase::report();
		std::cout << "cannot create function with void parameter" << std::endl;
	}
};

struct TooFewTemplateArgumentsError : TypeErrorBase
{
	TooFewTemplateArgumentsError(Location source)
		: TypeErrorBase(source)
	{
	}
	void report()
	{
		TypeErrorBase::report();
		std::cout << "too few template arguments" << std::endl;
	}
};



inline UniqueTypeWrapper getUniqueType(const TypeId& type, const InstantiationContext& context, bool allowDependent = false);
inline UniqueTypeWrapper getUniqueType(const Type& type, const InstantiationContext& context, bool allowDependent = false);

template<typename T>
inline UniqueTypeWrapper getUniqueTypeImpl(const T& type, const InstantiationContext& context, bool allowDependent)
{
	SYMBOLS_ASSERT(type.unique != 0);
	UniqueTypeWrapper result = UniqueTypeWrapper(type.unique);
	if(type.isDependent
		&& !allowDependent)
	{
		UniqueTypeWrapper substituted = substitute(result, context);
		SYMBOLS_ASSERT(!isDependent(substituted));
		return substituted;
	}
	return result;
}

inline UniqueTypeWrapper getUniqueType(const TypeId& type, const InstantiationContext& context, bool allowDependent)
{
	return getUniqueTypeImpl(type, context, allowDependent);
}

inline UniqueTypeWrapper getUniqueType(const Type& type, const InstantiationContext& context, bool allowDependent)
{
	return getUniqueTypeImpl(type, context, allowDependent);
}


#endif
