
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
BuiltInTypeDeclaration gCharDeclaration(gCharId);
Type gChar(&gCharDeclaration, TREEALLOCATOR_NULL);
Identifier gSignedCharId = makeIdentifier("$signed-char");
BuiltInTypeDeclaration gSignedCharDeclaration(gSignedCharId);
Type gSignedChar(&gSignedCharDeclaration, TREEALLOCATOR_NULL);
Identifier gUnsignedCharId = makeIdentifier("$unsigned-char");
BuiltInTypeDeclaration gUnsignedCharDeclaration(gUnsignedCharId);
Type gUnsignedChar(&gUnsignedCharDeclaration, TREEALLOCATOR_NULL);
Identifier gSignedShortIntId = makeIdentifier("$signed-short-int");
BuiltInTypeDeclaration gSignedShortIntDeclaration(gSignedShortIntId);
Type gSignedShortInt(&gSignedShortIntDeclaration, TREEALLOCATOR_NULL);
Identifier gUnsignedShortIntId = makeIdentifier("$unsigned-short-int");
BuiltInTypeDeclaration gUnsignedShortIntDeclaration(gUnsignedShortIntId);
Type gUnsignedShortInt(&gUnsignedShortIntDeclaration, TREEALLOCATOR_NULL);
Identifier gSignedIntId = makeIdentifier("$signed-int");
BuiltInTypeDeclaration gSignedIntDeclaration(gSignedIntId);
Type gSignedInt(&gSignedIntDeclaration, TREEALLOCATOR_NULL);
Identifier gUnsignedIntId = makeIdentifier("$unsigned-int");
BuiltInTypeDeclaration gUnsignedIntDeclaration(gUnsignedIntId);
Type gUnsignedInt(&gUnsignedIntDeclaration, TREEALLOCATOR_NULL);
Identifier gSignedLongIntId = makeIdentifier("$signed-long-int");
BuiltInTypeDeclaration gSignedLongIntDeclaration(gSignedLongIntId);
Type gSignedLongInt(&gSignedLongIntDeclaration, TREEALLOCATOR_NULL);
Identifier gUnsignedLongIntId = makeIdentifier("$unsigned-long-int");
BuiltInTypeDeclaration gUnsignedLongIntDeclaration(gUnsignedLongIntId);
Type gUnsignedLongInt(&gUnsignedLongIntDeclaration, TREEALLOCATOR_NULL);
Identifier gSignedLongLongIntId = makeIdentifier("$signed-long-long-int");
BuiltInTypeDeclaration gSignedLongLongIntDeclaration(gSignedLongLongIntId);
Type gSignedLongLongInt(&gSignedLongLongIntDeclaration, TREEALLOCATOR_NULL);
Identifier gUnsignedLongLongIntId = makeIdentifier("$unsigned-long-long-int");
BuiltInTypeDeclaration gUnsignedLongLongIntDeclaration(gUnsignedLongLongIntId);
Type gUnsignedLongLongInt(&gUnsignedLongLongIntDeclaration, TREEALLOCATOR_NULL);
Identifier gWCharId = makeIdentifier("$wchar_t");
BuiltInTypeDeclaration gWCharTDeclaration(gWCharId);
Type gWCharT(&gWCharTDeclaration, TREEALLOCATOR_NULL);
Identifier gBoolId = makeIdentifier("$bool");
BuiltInTypeDeclaration gBoolDeclaration(gBoolId);
Type gBool(&gBoolDeclaration, TREEALLOCATOR_NULL);
Identifier gFloatId = makeIdentifier("$float");
BuiltInTypeDeclaration gFloatDeclaration(gFloatId);
Type gFloat(&gFloatDeclaration, TREEALLOCATOR_NULL);
Identifier gDoubleId = makeIdentifier("$double");
BuiltInTypeDeclaration gDoubleDeclaration(gDoubleId);
Type gDouble(&gDoubleDeclaration, TREEALLOCATOR_NULL);
Identifier gLongDoubleId = makeIdentifier("$long-double");
BuiltInTypeDeclaration gLongDoubleDeclaration(gLongDoubleId);
Type gLongDouble(&gLongDoubleDeclaration, TREEALLOCATOR_NULL);
Identifier gVoidId = makeIdentifier("$void");
BuiltInTypeDeclaration gVoidDeclaration(gVoidId);
Type gVoid(&gVoidDeclaration, TREEALLOCATOR_NULL);

Reference< SequenceNodeGeneric<DeclaratorArray, TypeElementVisitor> >::Value gDeclaratorArrayNode = DeclaratorArray();

Identifier gStringLiteralId = makeIdentifier("$string-literal");
StringLiteralDeclaration gStringLiteralDeclaration(gStringLiteralId, gSignedChar, &gDeclaratorArrayNode);
Type gStringLiteral(&gStringLiteralDeclaration, TREEALLOCATOR_NULL);
Identifier gWideStringLiteralId = makeIdentifier("$wide-string-literal");
StringLiteralDeclaration gWideStringLiteralDeclaration(gWideStringLiteralId, gWCharT, &gDeclaratorArrayNode);
Type gWideStringLiteral(&gWideStringLiteralDeclaration, TREEALLOCATOR_NULL);

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
