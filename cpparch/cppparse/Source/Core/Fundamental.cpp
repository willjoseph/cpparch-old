
#include "Fundamental.h"

#include "Ast/AstAllocator.h"

static const TypeLayout LAYOUT_BOOL = TypeLayout(1, 1);
static const TypeLayout LAYOUT_CHAR = TypeLayout(1, 1);
static const TypeLayout LAYOUT_WCHART = TypeLayout(2, 2);
static const TypeLayout LAYOUT_SHORT = TypeLayout(2, 2);
static const TypeLayout LAYOUT_LONG = TypeLayout(4, 4);
static const TypeLayout LAYOUT_LONGLONG = TypeLayout(8, 8);
static const TypeLayout LAYOUT_FLOAT = TypeLayout(4, 4);
static const TypeLayout LAYOUT_DOUBLE = TypeLayout(8, 8);


// fundamental types
Identifier gCharId = makeIdentifier("char");
BuiltInTypeDeclaration gCharDeclaration(gCharId);
BuiltInTypeId gChar(&gCharDeclaration, AST_ALLOCATOR_NULL, LAYOUT_CHAR);
Identifier gSignedCharId = makeIdentifier("signed char");
BuiltInTypeDeclaration gSignedCharDeclaration(gSignedCharId);
BuiltInTypeId gSignedChar(&gSignedCharDeclaration, AST_ALLOCATOR_NULL, LAYOUT_CHAR);
Identifier gUnsignedCharId = makeIdentifier("unsigned char");
BuiltInTypeDeclaration gUnsignedCharDeclaration(gUnsignedCharId);
BuiltInTypeId gUnsignedChar(&gUnsignedCharDeclaration, AST_ALLOCATOR_NULL, LAYOUT_CHAR);
Identifier gSignedShortIntId = makeIdentifier("short");
BuiltInTypeDeclaration gSignedShortIntDeclaration(gSignedShortIntId);
BuiltInTypeId gSignedShortInt(&gSignedShortIntDeclaration, AST_ALLOCATOR_NULL, LAYOUT_SHORT);
Identifier gUnsignedShortIntId = makeIdentifier("unsigned short");
BuiltInTypeDeclaration gUnsignedShortIntDeclaration(gUnsignedShortIntId);
BuiltInTypeId gUnsignedShortInt(&gUnsignedShortIntDeclaration, AST_ALLOCATOR_NULL, LAYOUT_SHORT);
Identifier gSignedIntId = makeIdentifier("int");
BuiltInTypeDeclaration gSignedIntDeclaration(gSignedIntId);
BuiltInTypeId gSignedInt(&gSignedIntDeclaration, AST_ALLOCATOR_NULL, LAYOUT_LONG);
Identifier gUnsignedIntId = makeIdentifier("unsigned int");
BuiltInTypeDeclaration gUnsignedIntDeclaration(gUnsignedIntId);
BuiltInTypeId gUnsignedInt(&gUnsignedIntDeclaration, AST_ALLOCATOR_NULL, LAYOUT_LONG);
Identifier gSignedLongIntId = makeIdentifier("long");
BuiltInTypeDeclaration gSignedLongIntDeclaration(gSignedLongIntId);
BuiltInTypeId gSignedLongInt(&gSignedLongIntDeclaration, AST_ALLOCATOR_NULL, LAYOUT_LONG);
Identifier gUnsignedLongIntId = makeIdentifier("unsigned long");
BuiltInTypeDeclaration gUnsignedLongIntDeclaration(gUnsignedLongIntId);
BuiltInTypeId gUnsignedLongInt(&gUnsignedLongIntDeclaration, AST_ALLOCATOR_NULL, LAYOUT_LONG);
Identifier gSignedLongLongIntId = makeIdentifier("long long");
BuiltInTypeDeclaration gSignedLongLongIntDeclaration(gSignedLongLongIntId);
BuiltInTypeId gSignedLongLongInt(&gSignedLongLongIntDeclaration, AST_ALLOCATOR_NULL, LAYOUT_LONGLONG);
Identifier gUnsignedLongLongIntId = makeIdentifier("unsigned long long");
BuiltInTypeDeclaration gUnsignedLongLongIntDeclaration(gUnsignedLongLongIntId);
BuiltInTypeId gUnsignedLongLongInt(&gUnsignedLongLongIntDeclaration, AST_ALLOCATOR_NULL, LAYOUT_LONGLONG);
Identifier gWCharId = makeIdentifier("wchar_t");
BuiltInTypeDeclaration gWCharTDeclaration(gWCharId);
BuiltInTypeId gWCharT(&gWCharTDeclaration, AST_ALLOCATOR_NULL, LAYOUT_WCHART);
Identifier gBoolId = makeIdentifier("bool");
BuiltInTypeDeclaration gBoolDeclaration(gBoolId);
BuiltInTypeId gBool(&gBoolDeclaration, AST_ALLOCATOR_NULL, LAYOUT_BOOL);
Identifier gFloatId = makeIdentifier("float");
BuiltInTypeDeclaration gFloatDeclaration(gFloatId);
BuiltInTypeId gFloat(&gFloatDeclaration, AST_ALLOCATOR_NULL, LAYOUT_FLOAT);
Identifier gDoubleId = makeIdentifier("double");
BuiltInTypeDeclaration gDoubleDeclaration(gDoubleId);
BuiltInTypeId gDouble(&gDoubleDeclaration, AST_ALLOCATOR_NULL, LAYOUT_DOUBLE);
Identifier gLongDoubleId = makeIdentifier("long double");
BuiltInTypeDeclaration gLongDoubleDeclaration(gLongDoubleId);
BuiltInTypeId gLongDouble(&gLongDoubleDeclaration, AST_ALLOCATOR_NULL, LAYOUT_DOUBLE);
Identifier gVoidId = makeIdentifier("void");
BuiltInTypeDeclaration gVoidDeclaration(gVoidId, TYPE_SPECIAL);
BuiltInTypeId gVoid(&gVoidDeclaration, AST_ALLOCATOR_NULL, TYPELAYOUT_NONE);

StringLiteralTypeId gStringLiteral(&gCharDeclaration, AST_ALLOCATOR_NULL);
StringLiteralTypeId gWideStringLiteral(&gWCharTDeclaration, AST_ALLOCATOR_NULL);

