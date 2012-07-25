
#include "symbols.h"

const TypeElementEmpty gTypeElementEmpty;

UniqueNames gUniqueNames;
UniqueTypes gUniqueTypes;

size_t gScopeCount = 0;


// special-case
Identifier gUndeclaredId = makeIdentifier("$undeclared");
Declaration gUndeclared(TREEALLOCATOR_NULL, 0, gUndeclaredId, TYPE_NULL, 0);

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
Identifier gCharId = makeIdentifier("$char");
BuiltInTypeDeclaration gCharDeclaration(gCharId);
UniqueTypeId gChar(&gCharDeclaration, TREEALLOCATOR_NULL);
Identifier gSignedCharId = makeIdentifier("$signed-char");
BuiltInTypeDeclaration gSignedCharDeclaration(gSignedCharId);
UniqueTypeId gSignedChar(&gSignedCharDeclaration, TREEALLOCATOR_NULL);
Identifier gUnsignedCharId = makeIdentifier("$unsigned-char");
BuiltInTypeDeclaration gUnsignedCharDeclaration(gUnsignedCharId);
UniqueTypeId gUnsignedChar(&gUnsignedCharDeclaration, TREEALLOCATOR_NULL);
Identifier gSignedShortIntId = makeIdentifier("$signed-short-int");
BuiltInTypeDeclaration gSignedShortIntDeclaration(gSignedShortIntId);
UniqueTypeId gSignedShortInt(&gSignedShortIntDeclaration, TREEALLOCATOR_NULL);
Identifier gUnsignedShortIntId = makeIdentifier("$unsigned-short-int");
BuiltInTypeDeclaration gUnsignedShortIntDeclaration(gUnsignedShortIntId);
UniqueTypeId gUnsignedShortInt(&gUnsignedShortIntDeclaration, TREEALLOCATOR_NULL);
Identifier gSignedIntId = makeIdentifier("$signed-int");
BuiltInTypeDeclaration gSignedIntDeclaration(gSignedIntId);
UniqueTypeId gSignedInt(&gSignedIntDeclaration, TREEALLOCATOR_NULL);
Identifier gUnsignedIntId = makeIdentifier("$unsigned-int");
BuiltInTypeDeclaration gUnsignedIntDeclaration(gUnsignedIntId);
UniqueTypeId gUnsignedInt(&gUnsignedIntDeclaration, TREEALLOCATOR_NULL);
Identifier gSignedLongIntId = makeIdentifier("$signed-long-int");
BuiltInTypeDeclaration gSignedLongIntDeclaration(gSignedLongIntId);
UniqueTypeId gSignedLongInt(&gSignedLongIntDeclaration, TREEALLOCATOR_NULL);
Identifier gUnsignedLongIntId = makeIdentifier("$unsigned-long-int");
BuiltInTypeDeclaration gUnsignedLongIntDeclaration(gUnsignedLongIntId);
UniqueTypeId gUnsignedLongInt(&gUnsignedLongIntDeclaration, TREEALLOCATOR_NULL);
Identifier gSignedLongLongIntId = makeIdentifier("$signed-long-long-int");
BuiltInTypeDeclaration gSignedLongLongIntDeclaration(gSignedLongLongIntId);
UniqueTypeId gSignedLongLongInt(&gSignedLongLongIntDeclaration, TREEALLOCATOR_NULL);
Identifier gUnsignedLongLongIntId = makeIdentifier("$unsigned-long-long-int");
BuiltInTypeDeclaration gUnsignedLongLongIntDeclaration(gUnsignedLongLongIntId);
UniqueTypeId gUnsignedLongLongInt(&gUnsignedLongLongIntDeclaration, TREEALLOCATOR_NULL);
Identifier gWCharId = makeIdentifier("$wchar_t");
BuiltInTypeDeclaration gWCharTDeclaration(gWCharId);
UniqueTypeId gWCharT(&gWCharTDeclaration, TREEALLOCATOR_NULL);
Identifier gBoolId = makeIdentifier("$bool");
BuiltInTypeDeclaration gBoolDeclaration(gBoolId);
UniqueTypeId gBool(&gBoolDeclaration, TREEALLOCATOR_NULL);
Identifier gFloatId = makeIdentifier("$float");
BuiltInTypeDeclaration gFloatDeclaration(gFloatId);
UniqueTypeId gFloat(&gFloatDeclaration, TREEALLOCATOR_NULL);
Identifier gDoubleId = makeIdentifier("$double");
BuiltInTypeDeclaration gDoubleDeclaration(gDoubleId);
UniqueTypeId gDouble(&gDoubleDeclaration, TREEALLOCATOR_NULL);
Identifier gLongDoubleId = makeIdentifier("$long-double");
BuiltInTypeDeclaration gLongDoubleDeclaration(gLongDoubleId);
UniqueTypeId gLongDouble(&gLongDoubleDeclaration, TREEALLOCATOR_NULL);
Identifier gVoidId = makeIdentifier("$void");
BuiltInTypeDeclaration gVoidDeclaration(gVoidId, TYPE_SPECIAL);
UniqueTypeId gVoid(&gVoidDeclaration, TREEALLOCATOR_NULL);

StringLiteralTypeId gStringLiteral(&gCharDeclaration, TREEALLOCATOR_NULL);
StringLiteralTypeId gWideStringLiteral(&gWCharTDeclaration, TREEALLOCATOR_NULL);

// template placeholders
Identifier gDependentTypeId = makeIdentifier("$type");
Declaration gDependentType(TREEALLOCATOR_NULL, 0, gDependentTypeId, TYPE_SPECIAL, 0);
Identifier gDependentObjectId = makeIdentifier("$object");
Declaration gDependentObject(TREEALLOCATOR_NULL, 0, gDependentObjectId, TYPE_UNKNOWN, 0);
Identifier gDependentTemplateId = makeIdentifier("$template");
Declaration gDependentTemplate(TREEALLOCATOR_NULL, 0, gDependentTemplateId, TYPE_SPECIAL, 0, DeclSpecifiers(), true);
Identifier gDependentNestedId = makeIdentifier("$nested");
Declaration gDependentNested(TREEALLOCATOR_NULL, 0, gDependentNestedId, TYPE_SPECIAL, 0);

Identifier gParamId = makeIdentifier("$param");
Declaration gParam(TREEALLOCATOR_NULL, 0, gParamId, TYPE_CLASS, 0);

// objects
Identifier gOperatorFunctionId = makeIdentifier("operator <op>");
Identifier gConversionFunctionId = makeIdentifier("operator T");
Identifier gOperatorFunctionTemplateId = makeIdentifier("operator () <>");
// TODO: don't declare if id is anonymous?
Identifier gAnonymousId = makeIdentifier("$anonymous");

struct PointerTypeId : UniqueTypeId
{
	PointerTypeId(Declaration* declaration, const TreeAllocator<int>& allocator)
		: UniqueTypeId(declaration, allocator)
	{
		uniqueType.push_front(DeclaratorPointer(false));
	}
};
struct PointerPointerTypeId : UniqueTypeId
{
	PointerPointerTypeId(Declaration* declaration, const TreeAllocator<int>& allocator)
		: UniqueTypeId(declaration, allocator)
	{
		uniqueType.push_front(DeclaratorPointer(false));
		uniqueType.push_front(DeclaratorPointer(false));
	}
};
struct ArrayTypeId : UniqueTypeId
{
	ArrayTypeId(Declaration* declaration, const TreeAllocator<int>& allocator)
		: UniqueTypeId(declaration, allocator)
	{
		uniqueType.push_front(DeclaratorArray());
	}
};
struct MemberPointerTypeId : UniqueTypeId
{
	MemberPointerTypeId(Declaration* declaration, const TreeAllocator<int>& allocator)
		: UniqueTypeId(declaration, allocator)
	{
		uniqueType.push_front(DeclaratorMemberPointer(0));
	}
};

PointerTypeId gVoidPointer(&gVoidDeclaration, TREEALLOCATOR_NULL);
PointerTypeId gSignedIntPointer(&gSignedIntDeclaration, TREEALLOCATOR_NULL);
PointerTypeId gSignedCharPointer(&gSignedCharDeclaration, TREEALLOCATOR_NULL);
PointerTypeId gWCharTPointer(&gWCharTDeclaration, TREEALLOCATOR_NULL);
PointerPointerTypeId gSignedIntPointerPointer(&gSignedIntDeclaration, TREEALLOCATOR_NULL);
ArrayTypeId gSignedIntArray(&gSignedIntDeclaration, TREEALLOCATOR_NULL);
ArrayTypeId gFloatArray(&gFloatDeclaration, TREEALLOCATOR_NULL);
MemberPointerTypeId gSignedIntMemberPointer(&gSignedIntDeclaration, TREEALLOCATOR_NULL);


Identifier gBaseClassId = makeIdentifier("$base");
Scope gBaseClassScope(TREEALLOCATOR_NULL, gBaseClassId, SCOPETYPE_CLASS);
Declaration gBaseClassDeclaration(TREEALLOCATOR_NULL, 0, gBaseClassId, TYPE_CLASS, &gBaseClassScope);
UniqueTypeId gBaseClass(&gBaseClassDeclaration, TREEALLOCATOR_NULL);

Types2::Pointer::Value gBaseClassNode = Types2::Node(gBaseClass);

struct DerivedClassTypeId : UniqueTypeId
{
	DerivedClassTypeId(Declaration* declaration, const TreeAllocator<int>& allocator)
		: UniqueTypeId(declaration, allocator)
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

inline void testIcsRank()
{
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
	{
		IcsRank rank = getIcsRank(gSignedIntPointer, gSignedIntArray);
		SYMBOLS_ASSERT(rank == ICSRANK_STANDARDCONVERSION);
	}
	{
		IcsRank rank = getIcsRank(gSignedIntPointer, gFloatArray);
		SYMBOLS_ASSERT(rank == ICSRANK_INVALID);
	}
	{
		IcsRank rank = getIcsRank(gSignedIntPointerPointer, gSignedIntArray);
		SYMBOLS_ASSERT(rank == ICSRANK_INVALID);
	}

	// D -> B
	{
		IcsRank rank = getIcsRank(gBaseClass, gDerivedClass);
		SYMBOLS_ASSERT(rank == ICSRANK_STANDARDCONVERSION);
	}
	{
		IcsRank rank = getIcsRank(gDerivedClass, gBaseClass);
		SYMBOLS_ASSERT(rank == ICSRANK_INVALID);
	}

	// D* -> B*
	{
		IcsRank rank = getIcsRank(gBaseClassPointer, gDerivedClassPointer);
		SYMBOLS_ASSERT(rank == ICSRANK_STANDARDCONVERSION);
	}
	{
		IcsRank rank = getIcsRank(gDerivedClassPointer, gBaseClassPointer);
		SYMBOLS_ASSERT(rank == ICSRANK_INVALID);
	}

	// TODO: D::* -> B::*

}

struct TestIcsRank
{
	TestIcsRank()
	{
		testIcsRank();
	}
} gTestIcsRank;
