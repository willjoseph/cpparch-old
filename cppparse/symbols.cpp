
#include "symbols.h"

const TypeElementEmpty gTypeElementEmpty;

UniqueNames gUniqueNames;
UniqueTypes gUniqueTypes;
UniqueTypes gBuiltInTypes;

size_t gScopeCount = 0;


// special-case
Identifier gUndeclaredId = makeIdentifier("$undeclared");
Declaration gUndeclared(TREEALLOCATOR_NULL, 0, gUndeclaredId, TYPE_NULL, 0);
const DeclarationInstance gUndeclaredInstance(&gUndeclared);

// meta types
Identifier gArithmeticId = makeIdentifier("$arithmetic");
Declaration gArithmetic(TREEALLOCATOR_NULL, 0, gArithmeticId, TYPE_NULL, 0);
Identifier gSpecialId = makeIdentifier("$special");
Declaration gSpecial(TREEALLOCATOR_NULL, 0, gSpecialId, TYPE_NULL, 0);
Identifier gClassId = makeIdentifier("$class");
Declaration gClass(TREEALLOCATOR_NULL, 0, gClassId, TYPE_NULL, 0);
Identifier gEnumId = makeIdentifier("$enum");
Declaration gEnum(TREEALLOCATOR_NULL, 0, gEnumId, TYPE_NULL, 0);

// types
Identifier gNamespaceId = makeIdentifier("$namespace");
Declaration gNamespace(TREEALLOCATOR_NULL, 0, gNamespaceId, TYPE_NULL, 0);

Identifier gCtorId = makeIdentifier("$ctor");
Declaration gCtor(TREEALLOCATOR_NULL, 0, gCtorId, TYPE_SPECIAL, 0);
Identifier gEnumeratorId = makeIdentifier("$enumerator");
Declaration gEnumerator(TREEALLOCATOR_NULL, 0, gEnumeratorId, TYPE_SPECIAL, 0);
Identifier gUnknownId = makeIdentifier("$unknown");
Declaration gUnknown(TREEALLOCATOR_NULL, 0, gUnknownId, TYPE_SPECIAL, 0);

// fundamental types
Identifier gCharId = makeIdentifier("char");
BuiltInTypeDeclaration gCharDeclaration(gCharId);
ObjectTypeId gChar(&gCharDeclaration, TREEALLOCATOR_NULL);
Identifier gSignedCharId = makeIdentifier("signed char");
BuiltInTypeDeclaration gSignedCharDeclaration(gSignedCharId);
ObjectTypeId gSignedChar(&gSignedCharDeclaration, TREEALLOCATOR_NULL);
Identifier gUnsignedCharId = makeIdentifier("unsigned char");
BuiltInTypeDeclaration gUnsignedCharDeclaration(gUnsignedCharId);
ObjectTypeId gUnsignedChar(&gUnsignedCharDeclaration, TREEALLOCATOR_NULL);
Identifier gSignedShortIntId = makeIdentifier("short");
BuiltInTypeDeclaration gSignedShortIntDeclaration(gSignedShortIntId);
ObjectTypeId gSignedShortInt(&gSignedShortIntDeclaration, TREEALLOCATOR_NULL);
Identifier gUnsignedShortIntId = makeIdentifier("unsigned short");
BuiltInTypeDeclaration gUnsignedShortIntDeclaration(gUnsignedShortIntId);
ObjectTypeId gUnsignedShortInt(&gUnsignedShortIntDeclaration, TREEALLOCATOR_NULL);
Identifier gSignedIntId = makeIdentifier("int");
BuiltInTypeDeclaration gSignedIntDeclaration(gSignedIntId);
ObjectTypeId gSignedInt(&gSignedIntDeclaration, TREEALLOCATOR_NULL);
Identifier gUnsignedIntId = makeIdentifier("unsigned int");
BuiltInTypeDeclaration gUnsignedIntDeclaration(gUnsignedIntId);
ObjectTypeId gUnsignedInt(&gUnsignedIntDeclaration, TREEALLOCATOR_NULL);
Identifier gSignedLongIntId = makeIdentifier("long");
BuiltInTypeDeclaration gSignedLongIntDeclaration(gSignedLongIntId);
ObjectTypeId gSignedLongInt(&gSignedLongIntDeclaration, TREEALLOCATOR_NULL);
Identifier gUnsignedLongIntId = makeIdentifier("unsigned long");
BuiltInTypeDeclaration gUnsignedLongIntDeclaration(gUnsignedLongIntId);
ObjectTypeId gUnsignedLongInt(&gUnsignedLongIntDeclaration, TREEALLOCATOR_NULL);
Identifier gSignedLongLongIntId = makeIdentifier("long long");
BuiltInTypeDeclaration gSignedLongLongIntDeclaration(gSignedLongLongIntId);
ObjectTypeId gSignedLongLongInt(&gSignedLongLongIntDeclaration, TREEALLOCATOR_NULL);
Identifier gUnsignedLongLongIntId = makeIdentifier("unsigned long long");
BuiltInTypeDeclaration gUnsignedLongLongIntDeclaration(gUnsignedLongLongIntId);
ObjectTypeId gUnsignedLongLongInt(&gUnsignedLongLongIntDeclaration, TREEALLOCATOR_NULL);
Identifier gWCharId = makeIdentifier("wchar_t");
BuiltInTypeDeclaration gWCharTDeclaration(gWCharId);
ObjectTypeId gWCharT(&gWCharTDeclaration, TREEALLOCATOR_NULL);
Identifier gBoolId = makeIdentifier("bool");
BuiltInTypeDeclaration gBoolDeclaration(gBoolId);
ObjectTypeId gBool(&gBoolDeclaration, TREEALLOCATOR_NULL);
Identifier gFloatId = makeIdentifier("float");
BuiltInTypeDeclaration gFloatDeclaration(gFloatId);
ObjectTypeId gFloat(&gFloatDeclaration, TREEALLOCATOR_NULL);
Identifier gDoubleId = makeIdentifier("double");
BuiltInTypeDeclaration gDoubleDeclaration(gDoubleId);
ObjectTypeId gDouble(&gDoubleDeclaration, TREEALLOCATOR_NULL);
Identifier gLongDoubleId = makeIdentifier("long double");
BuiltInTypeDeclaration gLongDoubleDeclaration(gLongDoubleId);
ObjectTypeId gLongDouble(&gLongDoubleDeclaration, TREEALLOCATOR_NULL);
Identifier gVoidId = makeIdentifier("void");
BuiltInTypeDeclaration gVoidDeclaration(gVoidId, TYPE_SPECIAL);
ObjectTypeId gVoid(&gVoidDeclaration, TREEALLOCATOR_NULL);

StringLiteralTypeId gStringLiteral(&gCharDeclaration, TREEALLOCATOR_NULL);
StringLiteralTypeId gWideStringLiteral(&gWCharTDeclaration, TREEALLOCATOR_NULL);

// template placeholders
Identifier gDependentTypeId = makeIdentifier("$type");
Declaration gDependentType(TREEALLOCATOR_NULL, 0, gDependentTypeId, TYPE_SPECIAL, 0);
const DeclarationInstance gDependentTypeInstance(&gDependentType);
Identifier gDependentObjectId = makeIdentifier("$object");
Declaration gDependentObject(TREEALLOCATOR_NULL, 0, gDependentObjectId, TYPE_UNKNOWN, 0);
const DeclarationInstance gDependentObjectInstance(&gDependentObject);
Identifier gDependentTemplateId = makeIdentifier("$template");
Declaration gDependentTemplate(TREEALLOCATOR_NULL, 0, gDependentTemplateId, TYPE_SPECIAL, 0, DeclSpecifiers(), true);
const DeclarationInstance gDependentTemplateInstance(&gDependentTemplate);
Identifier gDependentNestedId = makeIdentifier("$nested");
Declaration gDependentNested(TREEALLOCATOR_NULL, 0, gDependentNestedId, TYPE_SPECIAL, 0);
const DeclarationInstance gDependentNestedInstance(&gDependentNested);

Identifier gParamId = makeIdentifier("$param");
Declaration gParam(TREEALLOCATOR_NULL, 0, gParamId, TYPE_CLASS, 0);
Identifier gNonTypeId = makeIdentifier("$non-type");
Declaration gNonType(TREEALLOCATOR_NULL, 0, gNonTypeId, TYPE_UNKNOWN, 0);

// objects
Name gOperatorNewId = Name("operator new");
Name gOperatorDeleteId = Name("operator delete");
Name gOperatorNewArrayId = Name("operator new[]");
Name gOperatorDeleteArrayId = Name("operator delete[]");
Name gOperatorPlusId = Name("operator+");
Name gOperatorMinusId = Name("operator-");
Name gOperatorStarId = Name("operator*");
Name gOperatorDivideId = Name("operator/");
Name gOperatorPercentId = Name("operator%");
Name gOperatorXorId = Name("operator^");
Name gOperatorAndId = Name("operator&");
Name gOperatorOrId = Name("operator|");
Name gOperatorComplId = Name("operator~");
Name gOperatorNotId = Name("operator!");
Name gOperatorAssignId = Name("operator=");
Name gOperatorLessId = Name("operator<");
Name gOperatorGreaterId = Name("operator>");
Name gOperatorPlusAssignId = Name("operator+=");
Name gOperatorMinusAssignId = Name("operator-=");
Name gOperatorStarAssignId = Name("operator*=");
Name gOperatorDivideAssignId = Name("operator/=");
Name gOperatorPercentAssignId = Name("operator%=");
Name gOperatorXorAssignId = Name("operator^=");
Name gOperatorAndAssignId = Name("operator&=");
Name gOperatorOrAssignId = Name("operator|=");
Name gOperatorShiftLeftId = Name("operator<<");
Name gOperatorShiftRightId = Name("operator>>");
Name gOperatorShiftRightAssignId = Name("operator>>=");
Name gOperatorShiftLeftAssignId = Name("operator<<=");
Name gOperatorEqualId = Name("operator==");
Name gOperatorNotEqualId = Name("operator!=");
Name gOperatorLessEqualId = Name("operator<=");
Name gOperatorGreaterEqualId = Name("operator>=");
Name gOperatorAndAndId = Name("operator&&");
Name gOperatorOrOrId = Name("operator||");
Name gOperatorPlusPlusId = Name("operator++");
Name gOperatorMinusMinusId = Name("operator--");
Name gOperatorCommaId = Name("operator,");
Name gOperatorArrowStarId = Name("operator->*");
Name gOperatorArrowId = Name("operator->");
Name gOperatorFunctionId = Name("operator()");
Name gOperatorArrayId = Name("operator[]");

Identifier gConversionFunctionId = makeIdentifier("operator T");
Identifier gOperatorFunctionTemplateId = makeIdentifier("operator () <>");
// TODO: don't declare if id is anonymous?
Identifier gAnonymousId = makeIdentifier("$anonymous");


struct PointerTypeId : ObjectTypeId
{
	PointerTypeId(Declaration* declaration, const TreeAllocator<int>& allocator)
		: ObjectTypeId(declaration, allocator)
	{
		value = pushBuiltInType(value, DeclaratorPointerType());
	}
};

struct PointerPointerTypeId : ObjectTypeId
{
	PointerPointerTypeId(Declaration* declaration, const TreeAllocator<int>& allocator)
		: ObjectTypeId(declaration, allocator)
	{
		value = pushBuiltInType(value, DeclaratorPointerType());
		value = pushBuiltInType(value, DeclaratorPointerType());
	}
};

PointerTypeId gVoidPointer(&gVoidDeclaration, TREEALLOCATOR_NULL);
PointerTypeId gSignedIntPointer(&gSignedIntDeclaration, TREEALLOCATOR_NULL);
PointerTypeId gSignedCharPointer(&gSignedCharDeclaration, TREEALLOCATOR_NULL);
PointerTypeId gWCharTPointer(&gWCharTDeclaration, TREEALLOCATOR_NULL);
PointerPointerTypeId gSignedIntPointerPointer(&gSignedIntDeclaration, TREEALLOCATOR_NULL);


Identifier gBaseClassId = makeIdentifier("$base");
Scope gBaseClassScope(TREEALLOCATOR_NULL, gBaseClassId, SCOPETYPE_CLASS);
Declaration gBaseClassDeclaration(TREEALLOCATOR_NULL, 0, gBaseClassId, TYPE_CLASS, &gBaseClassScope);
ObjectTypeId gBaseClass(&gBaseClassDeclaration, TREEALLOCATOR_NULL);

Types2::Pointer::Value gBaseClassNode = Types2::Node(Type(&gBaseClassDeclaration, TREEALLOCATOR_NULL));

struct DerivedClassTypeId : ObjectTypeId
{
	DerivedClassTypeId(Declaration* declaration, const TreeAllocator<int>& allocator)
		: ObjectTypeId(declaration, allocator)
	{
		declaration->enclosed->bases.head.next = &gBaseClassNode;
	}
};

Identifier gDerivedClassId = makeIdentifier("$derived");
Scope gDerivedClassScope(TREEALLOCATOR_NULL, gDerivedClassId, SCOPETYPE_CLASS);
Declaration gDerivedClassDeclaration(TREEALLOCATOR_NULL, 0, gDerivedClassId, TYPE_CLASS, &gDerivedClassScope);
DerivedClassTypeId gDerivedClass(&gDerivedClassDeclaration, TREEALLOCATOR_NULL);

PointerTypeId gBaseClassPointer(&gBaseClassDeclaration, TREEALLOCATOR_NULL);
PointerTypeId gDerivedClassPointer(&gDerivedClassDeclaration, TREEALLOCATOR_NULL);


struct Base
{
};
struct Derived : Base
{
};

template<typename T>
struct MakeType
{
private:
	static UniqueTypeWrapper apply();
};

template<>
struct MakeType<bool>
{
	static UniqueTypeWrapper apply()
	{
		return gBool;
	}
};

template<>
struct MakeType<int>
{
	static UniqueTypeWrapper apply()
	{
		return gSignedInt;
	}
};

template<>
struct MakeType<float>
{
	static UniqueTypeWrapper apply()
	{
		return gFloat;
	}
};

template<>
struct MakeType<char>
{
	static UniqueTypeWrapper apply()
	{
		return gChar;
	}
};

template<>
struct MakeType<wchar_t>
{
	static UniqueTypeWrapper apply()
	{
		return gWCharT;
	}
};

template<>
struct MakeType<void>
{
	static UniqueTypeWrapper apply()
	{
		return gVoid;
	}
};

template<>
struct MakeType<Base>
{
	static UniqueTypeWrapper apply()
	{
		return gBaseClass;
	}
};

template<>
struct MakeType<Derived>
{
	static UniqueTypeWrapper apply()
	{
		return gDerivedClass;
	}
};

struct MakeConst
{
	static UniqueTypeWrapper apply(UniqueTypeWrapper inner)
	{
		inner.value.setQualifiers(CvQualifiers(true, false));
		return inner;
	}
};

template<typename T>
struct MakeType<const T>
{
	static UniqueTypeWrapper apply()
	{
		return MakeConst::apply(MakeType<T>::apply());
	}
};

struct MakeVolatile
{
	static UniqueTypeWrapper apply(UniqueTypeWrapper inner)
	{
		inner.value.setQualifiers(CvQualifiers(false, true));
		return inner;
	}
};

template<typename T>
struct MakeType<volatile T>
{
	static UniqueTypeWrapper apply()
	{
		return MakeVolatile::apply(MakeType<T>::apply());
	}
};

struct MakeConstVolatile
{
	static UniqueTypeWrapper apply(UniqueTypeWrapper inner)
	{
		inner.value.setQualifiers(CvQualifiers(true, true));
		return inner;
	}
};

template<typename T>
struct MakeType<const volatile T>
{
	static UniqueTypeWrapper apply()
	{
		return MakeConstVolatile::apply(MakeType<T>::apply());
	}
};

template<typename T>
struct MakeType<T*>
{
	static UniqueTypeWrapper apply()
	{
		return UniqueTypeWrapper(pushBuiltInType(MakeType<T>::apply().value, DeclaratorPointerType()));
	}
};

template<typename T>
struct MakeType<T&>
{
	static UniqueTypeWrapper apply()
	{
		return UniqueTypeWrapper(pushBuiltInType(MakeType<T>::apply().value, DeclaratorReferenceType()));
	}
};

template<std::size_t size>
struct MakeArray
{
	static UniqueTypeWrapper apply(UniqueTypeWrapper inner)
	{
		return UniqueTypeWrapper(pushBuiltInType(inner.value, DeclaratorArrayType(size)));
	}
};

template<typename T>
struct MakeType<T[]>
{
	static UniqueTypeWrapper apply()
	{
		return MakeArray<0>::apply(MakeType<T>::apply());
	}
};

template<typename T, std::size_t size>
struct MakeType<T[size]>
{
	static UniqueTypeWrapper apply()
	{
		return MakeArray<size>::apply(MakeType<T>::apply());
	}
};

struct MakeFunction
{
	static UniqueTypeWrapper apply(UniqueTypeWrapper inner)
	{
		return UniqueTypeWrapper(pushBuiltInType(inner.value, DeclaratorFunctionType(Parameters(), CvQualifiers())));
	}
};

template<typename T>
struct MakeType<T()>
{
	static UniqueTypeWrapper apply()
	{
		return MakeFunction::apply(MakeType<T>::apply());
	}
};

template<typename C>
struct MakeMemberPointer
{
	static UniqueTypeWrapper apply(UniqueTypeWrapper inner)
	{
		const TypeInstance& instance = getObjectType(MakeType<C>::apply().value);
		DeclaratorMemberPointerType element(Type(instance.declaration, TREEALLOCATOR_NULL), CvQualifiers());
		element.instance = &instance;
		return UniqueTypeWrapper(pushBuiltInType(inner.value, element));
	}
};

template<typename T, typename C>
struct MakeType<T C::*>
{
	static UniqueTypeWrapper apply()
	{
		return MakeMemberPointer<C>::apply(MakeType<T>::apply());
	}
};

template<typename T, typename C>
struct MakeType<T (C::*)()>
{
	static UniqueTypeWrapper apply()
	{
		return MakeMemberPointer<C>::apply(MakeFunction::apply(MakeType<T>::apply()));
	}
};

template<typename T, typename C>
struct MakeType<T (C::*)() const>
{
	static UniqueTypeWrapper apply()
	{
		return MakeMemberPointer<C>::apply(MakeConst::apply(MakeFunction::apply(MakeType<T>::apply())));
	}
};


UniqueTypeWrapper gSignedIntMemberPointer = MakeType<int Base::*>::apply();


typedef const UniqueTypeId* UniqueTypeIdConstPointer;
UniqueTypeIdConstPointer gArithmeticTypes[] = {
	&gChar,
	&gSignedChar,
	&gUnsignedChar,
	&gSignedShortInt,
	&gUnsignedShortInt,
	&gSignedInt,
	&gUnsignedInt,
	&gSignedLongInt,
	&gUnsignedLongInt,
	&gSignedLongLongInt,
	&gUnsignedLongLongInt,
	&gWCharT,
	&gBool,
	&gFloat,
	&gDouble,
	&gLongDouble,
};

UniqueTypeIdConstPointer gFloatingTypes[] = {
	&gFloat,
	&gDouble,
	&gLongDouble,
};

UniqueTypeIdConstPointer gIntegerTypes[] = {
	&gChar,
	&gSignedChar,
	&gUnsignedChar,
	&gSignedShortInt,
	&gUnsignedShortInt,
	&gSignedInt,
	&gUnsignedInt,
	&gSignedLongInt,
	&gUnsignedLongInt,
	&gSignedLongLongInt,
	&gUnsignedLongLongInt,
	&gWCharT,
	&gBool,
};

UniqueTypeIdConstPointer gPointerTypes[] = {
	&gVoidPointer,
	&gSignedIntPointer,
	&gSignedIntPointerPointer,
	&gSignedCharPointer,
	&gWCharTPointer,
	&gBaseClassPointer,
	&gDerivedClassPointer,
};

UniqueTypeIdConstPointer gSimplePointerTypes[] = {
	&gSignedIntPointer,
	&gSignedCharPointer,
	&gWCharTPointer,
	&gBaseClassPointer,
	&gDerivedClassPointer,
};

UniqueTypeIdConstPointer gMemberPointerTypes[] = {
	&gSignedIntMemberPointer,
};

IcsRank getArithmeticIcsRank(const UniqueTypeId& to, const UniqueTypeId& from)
{
	if(&to == &from)
	{
		return ICSRANK_STANDARDEXACT;
	}
	if(&to == &gSignedInt)
	{
		if(&from == &gChar
			|| &from == &gSignedChar
			|| &from == &gUnsignedChar
			|| &from == &gSignedShortInt
			|| &from == &gUnsignedShortInt
			|| &from == &gWCharT
			|| &from == &gBool)
		{
			return ICSRANK_STANDARDPROMOTION;
		}
	}
	// TODO bitfield -> integer
	if(&to == &gDouble)
	{
		if(&from == &gFloat)
		{
			return ICSRANK_STANDARDPROMOTION;
		}
	}
	return ICSRANK_STANDARDCONVERSION;
}

template<typename To, typename From>
struct TestIcsRank
{
	static void apply(IcsRank expected, bool isNullPointerConstant = false, bool isLvalue = false)
	{
		IcsRank rank = getIcsRank(MakeType<To>::apply(), MakeType<From>::apply(), isNullPointerConstant, isLvalue);
		SYMBOLS_ASSERT(rank == expected);
	}
};

inline void testIcsRank()
{
	typedef const int ConstInt;
	typedef ConstInt* PtrToConstInt;
	typedef int* PtrToInt;
	typedef const PtrToInt ConstPtrToInt;
	typedef const PtrToConstInt ConstPtrToConstInt;
	UniqueTypeWrapper test1 = MakeType<ConstInt>::apply();
	UniqueTypeWrapper test2 = MakeType<PtrToConstInt>::apply();
	UniqueTypeWrapper test3 = MakeType<ConstPtrToInt>::apply();
	UniqueTypeWrapper test4 = MakeType<ConstPtrToConstInt>::apply();

	UniqueTypeWrapper test5 = MakeType<int (*)()>::apply();
	UniqueTypeWrapper test6 = MakeType<int (Base::*)()>::apply();
	UniqueTypeWrapper test7 = MakeType<int (Base::*)() const>::apply();
	UniqueTypeWrapper test8 = MakeType<int (Base::*const)()>::apply();

	UniqueTypeWrapper test9 = MakeType<int[]>::apply();
	UniqueTypeWrapper test10 = MakeType<int[1]>::apply();

	for(const UniqueTypeIdConstPointer* i = gArithmeticTypes; i != ARRAY_END(gArithmeticTypes); ++i)
	{
		UniqueTypeIdConstPointer to = *i;
		for(const UniqueTypeIdConstPointer* i = gArithmeticTypes; i != ARRAY_END(gArithmeticTypes); ++i)
		{
			UniqueTypeIdConstPointer from = *i;
			IcsRank expected = getArithmeticIcsRank(*to, *from);
			IcsRank rank = getIcsRank(*to, *from);
			SYMBOLS_ASSERT(expected == rank);
		}
	}

	// 0 -> T*
	for(const UniqueTypeIdConstPointer* i = gIntegerTypes; i != ARRAY_END(gIntegerTypes); ++i)
	{
		UniqueTypeIdConstPointer type = *i;
		for(const UniqueTypeIdConstPointer* i = gPointerTypes; i != ARRAY_END(gPointerTypes); ++i)
		{
			UniqueTypeIdConstPointer other = *i;
			{
				IcsRank rank = getIcsRank(*other, *type, true);
				SYMBOLS_ASSERT(rank == ICSRANK_STANDARDCONVERSION);
			}
			{
				IcsRank rank = getIcsRank(*other, *type, false);
				SYMBOLS_ASSERT(rank == ICSRANK_INVALID);
			}
		}
	}

	// T* -> bool, T::* -> bool
	for(const UniqueTypeIdConstPointer* i = gIntegerTypes; i != ARRAY_END(gIntegerTypes); ++i)
	{
		UniqueTypeIdConstPointer type = *i;
		for(const UniqueTypeIdConstPointer* i = gPointerTypes; i != ARRAY_END(gPointerTypes); ++i)
		{
			UniqueTypeIdConstPointer other = *i;
			IcsRank rank = getIcsRank(*type, *other);
			SYMBOLS_ASSERT(rank == (type == &gBool ? ICSRANK_STANDARDCONVERSION : ICSRANK_INVALID));
		}
		for(const UniqueTypeIdConstPointer* i = gMemberPointerTypes; i != ARRAY_END(gMemberPointerTypes); ++i)
		{
			UniqueTypeIdConstPointer other = *i;
			IcsRank rank = getIcsRank(*type, *other);
			SYMBOLS_ASSERT(rank == (type == &gBool ? ICSRANK_STANDARDCONVERSION : ICSRANK_INVALID));
		}
	}

	// T* -> void* (where T is an object type)
	for(const UniqueTypeIdConstPointer* i = gSimplePointerTypes; i != ARRAY_END(gSimplePointerTypes); ++i)
	{
		UniqueTypeIdConstPointer type = *i;
		{
			IcsRank rank = getIcsRank(gVoidPointer, *type);
			SYMBOLS_ASSERT(rank == ICSRANK_STANDARDCONVERSION);
		}
	}
	for(const UniqueTypeIdConstPointer* i = gFloatingTypes; i != ARRAY_END(gFloatingTypes); ++i)
	{
		UniqueTypeIdConstPointer type = *i;
		{
			IcsRank rank = getIcsRank(gVoidPointer, *type, true);
			SYMBOLS_ASSERT(rank == ICSRANK_INVALID);
		}
		{
			IcsRank rank = getIcsRank(*type, gVoidPointer, true);
			SYMBOLS_ASSERT(rank == ICSRANK_INVALID);
		}
	}

	// T[] -> T*
	TestIcsRank<int*, int[]>::apply(ICSRANK_STANDARDEXACT);
	TestIcsRank<int*, int[1]>::apply(ICSRANK_STANDARDEXACT);
	TestIcsRank<int*, float[1]>::apply(ICSRANK_INVALID);
	TestIcsRank<int**, int[1]>::apply(ICSRANK_INVALID);
	TestIcsRank<int[], int*>::apply(ICSRANK_INVALID);
	TestIcsRank<const int*, int[]>::apply(ICSRANK_STANDARDEXACT);
	TestIcsRank<const int*, const int[]>::apply(ICSRANK_STANDARDEXACT);
	TestIcsRank<int*, const int[]>::apply(ICSRANK_INVALID);

	// T() -> T(*)()
	TestIcsRank<int(*)(), int()>::apply(ICSRANK_STANDARDEXACT);
	TestIcsRank<int(), int(*)()>::apply(ICSRANK_INVALID);

	// D -> B
	TestIcsRank<Base, Derived>::apply(ICSRANK_STANDARDCONVERSION);
	TestIcsRank<Derived, Base>::apply(ICSRANK_INVALID);

	// D* -> B*
	TestIcsRank<Base*, Derived*>::apply(ICSRANK_STANDARDCONVERSION);
	TestIcsRank<Derived*, Base*>::apply(ICSRANK_INVALID);
	TestIcsRank<const Base*, Derived*>::apply(ICSRANK_STANDARDCONVERSION);
	TestIcsRank<const Base*, const Derived*>::apply(ICSRANK_STANDARDCONVERSION);
	TestIcsRank<Base*, const Derived*>::apply(ICSRANK_INVALID);

	// T D::* -> T B::*
	TestIcsRank<int Base::*, int Derived::*>::apply(ICSRANK_STANDARDCONVERSION);
	TestIcsRank<int Derived::*, int Base::*>::apply(ICSRANK_INVALID);

	// T& -> T&
	TestIcsRank<Base&, Base&>::apply(ICSRANK_STANDARDEXACT);
	TestIcsRank<const Base&, Base&>::apply(ICSRANK_STANDARDEXACT);
	TestIcsRank<const Base&, const Base&>::apply(ICSRANK_STANDARDEXACT);
	TestIcsRank<Base&, const Base&>::apply(ICSRANK_INVALID);

	// D& -> B&
	TestIcsRank<Base&, Derived&>::apply(ICSRANK_STANDARDCONVERSION);
	TestIcsRank<Derived&, Base&>::apply(ICSRANK_INVALID);
	TestIcsRank<const Base&, Derived&>::apply(ICSRANK_STANDARDCONVERSION);
	TestIcsRank<const Base&, const Derived&>::apply(ICSRANK_STANDARDCONVERSION);
	TestIcsRank<Base&, const Derived&>::apply(ICSRANK_INVALID);

	// bind to reference
	TestIcsRank<const int&, int&>::apply(ICSRANK_STANDARDEXACT);
	TestIcsRank<const int&, char>::apply(ICSRANK_STANDARDPROMOTION);
	TestIcsRank<const float&, int>::apply(ICSRANK_STANDARDCONVERSION);
	TestIcsRank<const int&, float>::apply(ICSRANK_STANDARDCONVERSION);
	TestIcsRank<const bool&, Base*>::apply(ICSRANK_STANDARDCONVERSION);

	// convert from reference 
	TestIcsRank<int, int&>::apply(ICSRANK_STANDARDEXACT);
	TestIcsRank<int, char&>::apply(ICSRANK_STANDARDPROMOTION);
	TestIcsRank<int, float&>::apply(ICSRANK_STANDARDCONVERSION);
	TestIcsRank<Base, Derived&>::apply(ICSRANK_STANDARDCONVERSION);
	TestIcsRank<int*, int(&)[1]>::apply(ICSRANK_STANDARDEXACT);

	// lvalue T -> T&
	TestIcsRank<Base&, Base>::apply(ICSRANK_STANDARDEXACT, false, true);
	// rvalue T -> T&
	TestIcsRank<Base&, Base>::apply(ICSRANK_INVALID, false, false);
}

struct IcsRankTest
{
	IcsRankTest()
	{
		testIcsRank();
	}
} gIcsRankTest;


#if 0

namespace Test
{
	struct Base
	{
		int m();
	};

	template<typename T>
	struct MakeType
	{
	};

	template<>
	struct MakeType<int>
	{
		static void apply()
		{
		}
	};

	template<>
	struct MakeType<Base>
	{
		static void apply()
		{
		}
	};

	template<typename T>
	struct MakeType<const T>
	{
		static void apply()
		{
			return MakeType<T>::apply();
		}
	};

	template<typename T>
	struct MakeType<T*>
	{
		static void apply()
		{
			return MakeType<T>::apply();
		}
	};

	template<typename T>
	struct MakeType<T&>
	{
		static void apply()
		{
			return MakeType<T>::apply();
		}
	};

	template<typename T>
	struct MakeType<T()>
	{
		static void apply()
		{
			return MakeType<T>::apply();
		}
	};

	template<typename T, typename C>
	struct MakeType<T C::*>
	{
		static void apply()
		{
			return MakeType<T>::apply();
		}
	};

	void f()
	{
		MakeType<int (Base::*)()>::apply();
	}
}
#endif