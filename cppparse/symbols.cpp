
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
Declaration gChar(TREEALLOCATOR_NULL, 0, gCharId, TYPE_SPECIAL, 0);
Identifier gSignedCharId = makeIdentifier("$signed-char");
Declaration gSignedChar(TREEALLOCATOR_NULL, 0, gSignedCharId, TYPE_SPECIAL, 0);
Identifier gUnsignedCharId = makeIdentifier("$unsigned-char");
Declaration gUnsignedChar(TREEALLOCATOR_NULL, 0, gUnsignedCharId, TYPE_SPECIAL, 0);
Identifier gSignedShortIntId = makeIdentifier("$signed-short-int");
Declaration gSignedShortInt(TREEALLOCATOR_NULL, 0, gSignedShortIntId, TYPE_SPECIAL, 0);
Identifier gUnsignedShortIntId = makeIdentifier("$unsigned-short-int");
Declaration gUnsignedShortInt(TREEALLOCATOR_NULL, 0, gUnsignedShortIntId, TYPE_SPECIAL, 0);
Identifier gSignedIntId = makeIdentifier("$signed-int");
Declaration gSignedInt(TREEALLOCATOR_NULL, 0, gSignedIntId, TYPE_SPECIAL, 0);
Identifier gUnsignedIntId = makeIdentifier("$unsigned-int");
Declaration gUnsignedInt(TREEALLOCATOR_NULL, 0, gUnsignedIntId, TYPE_SPECIAL, 0);
Identifier gSignedLongIntId = makeIdentifier("$signed-long-int");
Declaration gSignedLongInt(TREEALLOCATOR_NULL, 0, gSignedLongIntId, TYPE_SPECIAL, 0);
Identifier gUnsignedLongIntId = makeIdentifier("$unsigned-long-int");
Declaration gUnsignedLongInt(TREEALLOCATOR_NULL, 0, gUnsignedLongIntId, TYPE_SPECIAL, 0);
Identifier gSignedLongLongIntId = makeIdentifier("$signed-long-long-int");
Declaration gSignedLongLongInt(TREEALLOCATOR_NULL, 0, gSignedLongLongIntId, TYPE_SPECIAL, 0);
Identifier gUnsignedLongLongIntId = makeIdentifier("$unsigned-long-long-int");
Declaration gUnsignedLongLongInt(TREEALLOCATOR_NULL, 0, gUnsignedLongLongIntId, TYPE_SPECIAL, 0);
Identifier gWCharId = makeIdentifier("$wchar_t");
Declaration gWCharT(TREEALLOCATOR_NULL, 0, gWCharId, TYPE_SPECIAL, 0);
Identifier gBoolId = makeIdentifier("$bool");
Declaration gBool(TREEALLOCATOR_NULL, 0, gBoolId, TYPE_SPECIAL, 0);
Identifier gFloatId = makeIdentifier("$float");
Declaration gFloat(TREEALLOCATOR_NULL, 0, gFloatId, TYPE_SPECIAL, 0);
Identifier gDoubleId = makeIdentifier("$double");
Declaration gDouble(TREEALLOCATOR_NULL, 0, gDoubleId, TYPE_SPECIAL, 0);
Identifier gLongDoubleId = makeIdentifier("$long-double");
Declaration gLongDouble(TREEALLOCATOR_NULL, 0, gLongDoubleId, TYPE_SPECIAL, 0);
Identifier gVoidId = makeIdentifier("$void");
Declaration gVoid(TREEALLOCATOR_NULL, 0, gVoidId, TYPE_SPECIAL, 0);

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
