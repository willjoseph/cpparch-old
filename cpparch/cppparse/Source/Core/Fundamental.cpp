
#include "Fundamental.h"

#include "Ast/AstAllocator.h"

// fundamental types
Identifier gCharId = makeIdentifier("char");
BuiltInTypeDeclaration gCharDeclaration(gCharId);
BuiltInTypeId gChar(&gCharDeclaration, AST_ALLOCATOR_NULL);
Identifier gSignedCharId = makeIdentifier("signed char");
BuiltInTypeDeclaration gSignedCharDeclaration(gSignedCharId);
BuiltInTypeId gSignedChar(&gSignedCharDeclaration, AST_ALLOCATOR_NULL);
Identifier gUnsignedCharId = makeIdentifier("unsigned char");
BuiltInTypeDeclaration gUnsignedCharDeclaration(gUnsignedCharId);
BuiltInTypeId gUnsignedChar(&gUnsignedCharDeclaration, AST_ALLOCATOR_NULL);
Identifier gSignedShortIntId = makeIdentifier("short");
BuiltInTypeDeclaration gSignedShortIntDeclaration(gSignedShortIntId);
BuiltInTypeId gSignedShortInt(&gSignedShortIntDeclaration, AST_ALLOCATOR_NULL);
Identifier gUnsignedShortIntId = makeIdentifier("unsigned short");
BuiltInTypeDeclaration gUnsignedShortIntDeclaration(gUnsignedShortIntId);
BuiltInTypeId gUnsignedShortInt(&gUnsignedShortIntDeclaration, AST_ALLOCATOR_NULL);
Identifier gSignedIntId = makeIdentifier("int");
BuiltInTypeDeclaration gSignedIntDeclaration(gSignedIntId);
BuiltInTypeId gSignedInt(&gSignedIntDeclaration, AST_ALLOCATOR_NULL);
Identifier gUnsignedIntId = makeIdentifier("unsigned int");
BuiltInTypeDeclaration gUnsignedIntDeclaration(gUnsignedIntId);
BuiltInTypeId gUnsignedInt(&gUnsignedIntDeclaration, AST_ALLOCATOR_NULL);
Identifier gSignedLongIntId = makeIdentifier("long");
BuiltInTypeDeclaration gSignedLongIntDeclaration(gSignedLongIntId);
BuiltInTypeId gSignedLongInt(&gSignedLongIntDeclaration, AST_ALLOCATOR_NULL);
Identifier gUnsignedLongIntId = makeIdentifier("unsigned long");
BuiltInTypeDeclaration gUnsignedLongIntDeclaration(gUnsignedLongIntId);
BuiltInTypeId gUnsignedLongInt(&gUnsignedLongIntDeclaration, AST_ALLOCATOR_NULL);
Identifier gSignedLongLongIntId = makeIdentifier("long long");
BuiltInTypeDeclaration gSignedLongLongIntDeclaration(gSignedLongLongIntId);
BuiltInTypeId gSignedLongLongInt(&gSignedLongLongIntDeclaration, AST_ALLOCATOR_NULL);
Identifier gUnsignedLongLongIntId = makeIdentifier("unsigned long long");
BuiltInTypeDeclaration gUnsignedLongLongIntDeclaration(gUnsignedLongLongIntId);
BuiltInTypeId gUnsignedLongLongInt(&gUnsignedLongLongIntDeclaration, AST_ALLOCATOR_NULL);
Identifier gWCharId = makeIdentifier("wchar_t");
BuiltInTypeDeclaration gWCharTDeclaration(gWCharId);
BuiltInTypeId gWCharT(&gWCharTDeclaration, AST_ALLOCATOR_NULL);
Identifier gBoolId = makeIdentifier("bool");
BuiltInTypeDeclaration gBoolDeclaration(gBoolId);
BuiltInTypeId gBool(&gBoolDeclaration, AST_ALLOCATOR_NULL);
Identifier gFloatId = makeIdentifier("float");
BuiltInTypeDeclaration gFloatDeclaration(gFloatId);
BuiltInTypeId gFloat(&gFloatDeclaration, AST_ALLOCATOR_NULL);
Identifier gDoubleId = makeIdentifier("double");
BuiltInTypeDeclaration gDoubleDeclaration(gDoubleId);
BuiltInTypeId gDouble(&gDoubleDeclaration, AST_ALLOCATOR_NULL);
Identifier gLongDoubleId = makeIdentifier("long double");
BuiltInTypeDeclaration gLongDoubleDeclaration(gLongDoubleId);
BuiltInTypeId gLongDouble(&gLongDoubleDeclaration, AST_ALLOCATOR_NULL);
Identifier gVoidId = makeIdentifier("void");
BuiltInTypeDeclaration gVoidDeclaration(gVoidId, TYPE_SPECIAL);
BuiltInTypeId gVoid(&gVoidDeclaration, AST_ALLOCATOR_NULL);

StringLiteralTypeId gStringLiteral(&gCharDeclaration, AST_ALLOCATOR_NULL);
StringLiteralTypeId gWideStringLiteral(&gWCharTDeclaration, AST_ALLOCATOR_NULL);

