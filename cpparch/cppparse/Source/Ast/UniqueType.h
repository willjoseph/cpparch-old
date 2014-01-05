
#ifndef INCLUDED_CPPPARSE_AST_UNIQUETYPE_H
#define INCLUDED_CPPPARSE_AST_UNIQUETYPE_H

#include "Common/IndirectSet.h"
#include "Parse/Grammar.h"
#include <typeinfo>

// ----------------------------------------------------------------------------
// unique types
// Representation of a declarator, with type-elements linked in 'normal' order.
// e.g. int(*)[] == pointer to array of == DeclaratorPointerType -> DeclaratorArrayType
// Note that this is the reverse of the order that the declarator is parsed in.
// This means a given unique type sub-sequence need only be stored once.
// This allows fast comparison of types and simplifies printing of declarators.

struct TypeElementVisitor
{
#if 0
	virtual void visit(const struct Namespace&) = 0;
#endif
	virtual void visit(const struct DependentType&) = 0;
	virtual void visit(const struct DependentTypename&) = 0;
	virtual void visit(const struct DependentNonType&) = 0;
	virtual void visit(const struct DependentDecltype&) = 0;
	virtual void visit(const struct TemplateTemplateArgument&) = 0;
	virtual void visit(const struct NonType&) = 0;
	virtual void visit(const struct SimpleType&) = 0;
	virtual void visit(const struct PointerType&) = 0;
	virtual void visit(const struct ReferenceType&) = 0;
	virtual void visit(const struct ArrayType&) = 0;
	virtual void visit(const struct MemberPointerType&) = 0;
	virtual void visit(const struct FunctionType&) = 0;
};

struct TypeElement : TypeInfo
{
	UniqueType next;
	bool isDependent;

	TypeElement(TypeInfo type) : TypeInfo(type)
	{
	}
	virtual ~TypeElement()
	{
	}
	virtual void accept(TypeElementVisitor& visitor) const = 0;
	virtual bool operator<(const TypeElement& other) const = 0;
};

struct TypeElementEmpty : TypeElement
{
	TypeElementEmpty() : TypeElement(getTypeInfo<TypeElementEmpty>())
	{
		next = 0;
		isDependent = false;
	}
	virtual void accept(TypeElementVisitor& visitor) const
	{
		throw SymbolsError();
	}
	virtual bool operator<(const TypeElement& other) const
	{
		throw SymbolsError();
	}
};

extern const TypeElementEmpty gTypeElementEmpty;


template<typename T>
struct TypeElementGeneric : TypeElement
{
	T value;
	TypeElementGeneric(const T& value)
		 : TypeElement(getTypeInfo<TypeElementGeneric>()), value(value)
	{
	}
	void accept(TypeElementVisitor& visitor) const
	{
		visitor.visit(value);
	}
	bool operator<(const TypeElementGeneric& other) const
	{
		return value < other.value;
	}
	bool operator<(const TypeElement& other) const
	{
		return next != other.next
			? next < other.next
			: abstractLess(*this, other);
	}
};

const UniqueType UNIQUETYPE_NULL = &gTypeElementEmpty;


typedef IndirectSet<UniqueType> UniqueTypes;

extern UniqueTypes gBuiltInTypes;

template<typename T>
inline UniqueType pushBuiltInType(UniqueType type, const T& value)
{
	TypeElementGeneric<T> node(value);
	node.next = type;
	node.isDependent = type->isDependent || isDependent(value);
	return *gBuiltInTypes.insert(node);
}

template<typename T>
inline UniqueType pushUniqueType(UniqueTypes& types, UniqueType type, const T& value)
{
	TypeElementGeneric<T> node(value);
	node.next = type;
	node.isDependent = type->isDependent || isDependent(value);
	{
		UniqueTypes::iterator i = gBuiltInTypes.find(node);
		if(i != gBuiltInTypes.end())
		{
			return *i;
		}
	}
	return *types.insert(node);
}

extern UniqueTypes gUniqueTypes;

template<typename T>
inline void pushUniqueType(UniqueType& type, const T& value)
{
	type = pushUniqueType(gUniqueTypes, type, value);
}

inline void popUniqueType(UniqueType& type)
{
	SYMBOLS_ASSERT(type.getBits() != 0);
	type = type->next;
}

struct UniqueTypeWrapper
{
	UniqueType value;

	UniqueTypeWrapper()
		: value(&gTypeElementEmpty)
	{
	}
	explicit UniqueTypeWrapper(UniqueType value)
		: value(value)
	{
	}
	template<typename T>
	void push_front(const T& t)
	{
		pushUniqueType(value, t);
	}
	void pop_front()
	{
		SYMBOLS_ASSERT(value != 0);
		SYMBOLS_ASSERT(value != UNIQUETYPE_NULL);
		popUniqueType(value);
	}
	void swap(UniqueTypeWrapper& other)
	{
		std::swap(value, other.value);
	}
	bool empty() const
	{
		return value == UNIQUETYPE_NULL;
	}
	bool isSimple() const
	{
		return isEqual(getTypeInfo(*value), getTypeInfo<TypeElementGeneric<SimpleType> >());
	}
#if 0
	bool isNamespace() const
	{
		return isEqual(getTypeInfo(*value), getTypeInfo<TypeElementGeneric<struct Namespace> >());
	}
#endif
	bool isPointer() const
	{
		return isEqual(getTypeInfo(*value), getTypeInfo<TypeElementGeneric<PointerType> >());
	}
	bool isReference() const
	{
		return isEqual(getTypeInfo(*value), getTypeInfo<TypeElementGeneric<ReferenceType> >());
	}
	bool isArray() const
	{
		return isEqual(getTypeInfo(*value), getTypeInfo<TypeElementGeneric<ArrayType> >());
	}
	bool isMemberPointer() const
	{
		return isEqual(getTypeInfo(*value), getTypeInfo<TypeElementGeneric<MemberPointerType> >());
	}
	bool isFunction() const
	{
		return isEqual(getTypeInfo(*value), getTypeInfo<TypeElementGeneric<FunctionType> >());
	}
	bool isDependentNonType() const
	{
		return isEqual(getTypeInfo(*value), getTypeInfo<TypeElementGeneric<DependentNonType> >());
	}
	bool isDependentType() const
	{
		return isEqual(getTypeInfo(*value), getTypeInfo<TypeElementGeneric<DependentType> >());
	}
	bool isDependent() const
	{
		return isDependentType()
			|| isEqual(getTypeInfo(*value), getTypeInfo<TypeElementGeneric<DependentTypename> >())
			|| isEqual(getTypeInfo(*value), getTypeInfo<TypeElementGeneric<DependentDecltype> >())
			|| isDependentNonType();
	}
	bool isNonType() const
	{
		return isEqual(getTypeInfo(*value), getTypeInfo<TypeElementGeneric<NonType> >());
	}
	bool isTemplateTemplateArgument() const
	{
		return isEqual(getTypeInfo(*value), getTypeInfo<TypeElementGeneric<TemplateTemplateArgument> >());
	}
	bool isSimplePointer() const
	{
		return isPointer()
			&& UniqueTypeWrapper(value->next).isSimple();
	}
	bool isSimpleReference() const
	{
		return isReference()
			&& UniqueTypeWrapper(value->next).isSimple();
	}
	bool isSimpleArray() const
	{
		return isArray()
			&& UniqueTypeWrapper(value->next).isSimple();
	}
	bool isFunctionPointer() const
	{
		return isPointer()
			&& UniqueTypeWrapper(value->next).isFunction();
	}
};

template<typename T>
inline UniqueTypeWrapper pushType(UniqueTypeWrapper type, const T& t)
{
	pushUniqueType(type.value, t);
	return type;
}

template<typename T>
inline UniqueTypeWrapper pushBuiltInType(UniqueTypeWrapper type, const T& value)
{
	return UniqueTypeWrapper(pushBuiltInType(type.value, value));
}

inline UniqueTypeWrapper popType(UniqueTypeWrapper type)
{
	type.pop_front();
	return type;
}

inline UniqueTypeWrapper qualifyType(UniqueTypeWrapper type, CvQualifiers qualifiers)
{
	type.value.setQualifiers(qualifiers);
	return type;
}

inline bool operator==(UniqueTypeWrapper l, UniqueTypeWrapper r)
{
	return l.value == r.value;
}

inline bool operator!=(UniqueTypeWrapper l, UniqueTypeWrapper r)
{
	return !operator==(l, r);
}

inline bool operator<(UniqueTypeWrapper l, UniqueTypeWrapper r)
{
	return l.value < r.value;
}

inline bool isGreaterCvQualification(CvQualifiers l, CvQualifiers r)
{
	return l.isConst + l.isVolatile > r.isConst + r.isVolatile;
}

inline bool isGreaterCvQualification(UniqueTypeWrapper to, UniqueTypeWrapper from)
{
	return isGreaterCvQualification(to.value.getQualifiers(), from.value.getQualifiers());
}

inline bool isEqualCvQualification(UniqueTypeWrapper to, UniqueTypeWrapper from)
{
	return to.value.getQualifiers() == from.value.getQualifiers();
}

#endif
