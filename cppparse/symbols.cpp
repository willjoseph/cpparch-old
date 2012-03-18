
#include "symbols.h"

UniqueNames gUniqueNames;

size_t gScopeCount = 0;



// special-case
Identifier gUndeclaredId = makeIdentifier("$undeclared");
Declaration gUndeclared(TREEALLOCATOR_NULL, 0, gUndeclaredId, TYPE_NULL, 0);

// meta types
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
Declaration gCharDeclaration(TREEALLOCATOR_NULL, 0, gCharId, TYPE_SPECIAL, 0);
Type gChar(&gCharDeclaration, TREEALLOCATOR_NULL);
Identifier gSignedCharId = makeIdentifier("$signed-char");
Declaration gSignedCharDeclaration(TREEALLOCATOR_NULL, 0, gSignedCharId, TYPE_SPECIAL, 0);
Type gSignedChar(&gSignedCharDeclaration, TREEALLOCATOR_NULL);
Identifier gUnsignedCharId = makeIdentifier("$unsigned-char");
Declaration gUnsignedCharDeclaration(TREEALLOCATOR_NULL, 0, gUnsignedCharId, TYPE_SPECIAL, 0);
Type gUnsignedChar(&gUnsignedCharDeclaration, TREEALLOCATOR_NULL);
Identifier gSignedShortIntId = makeIdentifier("$signed-short-int");
Declaration gSignedShortIntDeclaration(TREEALLOCATOR_NULL, 0, gSignedShortIntId, TYPE_SPECIAL, 0);
Type gSignedShortInt(&gSignedShortIntDeclaration, TREEALLOCATOR_NULL);
Identifier gUnsignedShortIntId = makeIdentifier("$unsigned-short-int");
Declaration gUnsignedShortIntDeclaration(TREEALLOCATOR_NULL, 0, gUnsignedShortIntId, TYPE_SPECIAL, 0);
Type gUnsignedShortInt(&gUnsignedShortIntDeclaration, TREEALLOCATOR_NULL);
Identifier gSignedIntId = makeIdentifier("$signed-int");
Declaration gSignedIntDeclaration(TREEALLOCATOR_NULL, 0, gSignedIntId, TYPE_SPECIAL, 0);
Type gSignedInt(&gSignedIntDeclaration, TREEALLOCATOR_NULL);
Identifier gUnsignedIntId = makeIdentifier("$unsigned-int");
Declaration gUnsignedIntDeclaration(TREEALLOCATOR_NULL, 0, gUnsignedIntId, TYPE_SPECIAL, 0);
Type gUnsignedInt(&gUnsignedIntDeclaration, TREEALLOCATOR_NULL);
Identifier gSignedLongIntId = makeIdentifier("$signed-long-int");
Declaration gSignedLongIntDeclaration(TREEALLOCATOR_NULL, 0, gSignedLongIntId, TYPE_SPECIAL, 0);
Type gSignedLongInt(&gSignedLongIntDeclaration, TREEALLOCATOR_NULL);
Identifier gUnsignedLongIntId = makeIdentifier("$unsigned-long-int");
Declaration gUnsignedLongIntDeclaration(TREEALLOCATOR_NULL, 0, gUnsignedLongIntId, TYPE_SPECIAL, 0);
Type gUnsignedLongInt(&gUnsignedLongIntDeclaration, TREEALLOCATOR_NULL);
Identifier gSignedLongLongIntId = makeIdentifier("$signed-long-long-int");
Declaration gSignedLongLongIntDeclaration(TREEALLOCATOR_NULL, 0, gSignedLongLongIntId, TYPE_SPECIAL, 0);
Type gSignedLongLongInt(&gSignedLongLongIntDeclaration, TREEALLOCATOR_NULL);
Identifier gUnsignedLongLongIntId = makeIdentifier("$unsigned-long-long-int");
Declaration gUnsignedLongLongIntDeclaration(TREEALLOCATOR_NULL, 0, gUnsignedLongLongIntId, TYPE_SPECIAL, 0);
Type gUnsignedLongLongInt(&gUnsignedLongLongIntDeclaration, TREEALLOCATOR_NULL);
Identifier gWCharId = makeIdentifier("$wchar_t");
Declaration gWCharTDeclaration(TREEALLOCATOR_NULL, 0, gWCharId, TYPE_SPECIAL, 0);
Type gWCharT(&gWCharTDeclaration, TREEALLOCATOR_NULL);
Identifier gBoolId = makeIdentifier("$bool");
Declaration gBoolDeclaration(TREEALLOCATOR_NULL, 0, gBoolId, TYPE_SPECIAL, 0);
Type gBool(&gBoolDeclaration, TREEALLOCATOR_NULL);
Identifier gFloatId = makeIdentifier("$float");
Declaration gFloatDeclaration(TREEALLOCATOR_NULL, 0, gFloatId, TYPE_SPECIAL, 0);
Type gFloat(&gFloatDeclaration, TREEALLOCATOR_NULL);
Identifier gDoubleId = makeIdentifier("$double");
Declaration gDoubleDeclaration(TREEALLOCATOR_NULL, 0, gDoubleId, TYPE_SPECIAL, 0);
Type gDouble(&gDoubleDeclaration, TREEALLOCATOR_NULL);
Identifier gLongDoubleId = makeIdentifier("$long-double");
Declaration gLongDoubleDeclaration(TREEALLOCATOR_NULL, 0, gLongDoubleId, TYPE_SPECIAL, 0);
Type gLongDouble(&gLongDoubleDeclaration, TREEALLOCATOR_NULL);
Identifier gVoidId = makeIdentifier("$void");
Declaration gVoidDeclaration(TREEALLOCATOR_NULL, 0, gVoidId, TYPE_SPECIAL, 0);
Type gVoid(&gVoidDeclaration, TREEALLOCATOR_NULL);

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
