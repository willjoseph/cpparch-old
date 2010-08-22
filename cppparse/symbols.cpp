
#include "symbols.h"

UniqueNames gUniqueNames;

size_t gScopeCount = 0;




// special-case
Declaration gUndeclared(TREEALLOCATOR_NULL, 0, makeIdentifier("$undeclared"), TYPE_NULL, 0);
Declaration gFriend(TREEALLOCATOR_NULL, 0, makeIdentifier("$friend"), TYPE_NULL, 0);

// meta types
Declaration gSpecial(TREEALLOCATOR_NULL, 0, makeIdentifier("$special"), TYPE_NULL, 0);
Declaration gClass(TREEALLOCATOR_NULL, 0, makeIdentifier("$class"), TYPE_NULL, 0);
Declaration gEnum(TREEALLOCATOR_NULL, 0, makeIdentifier("$enum"), TYPE_NULL, 0);

// types
Declaration gNamespace(TREEALLOCATOR_NULL, 0, makeIdentifier("$namespace"), TYPE_NULL, 0);

Declaration gCtor(TREEALLOCATOR_NULL, 0, makeIdentifier("$ctor"), TYPE_SPECIAL, 0);
Declaration gEnumerator(TREEALLOCATOR_NULL, 0, makeIdentifier("$enumerator"), TYPE_SPECIAL, 0);
Declaration gUnknown(TREEALLOCATOR_NULL, 0, makeIdentifier("$unknown"), TYPE_SPECIAL, 0);

// fundamental types
Declaration gChar(TREEALLOCATOR_NULL, 0, makeIdentifier("$char"), TYPE_SPECIAL, 0);
Declaration gSignedChar(TREEALLOCATOR_NULL, 0, makeIdentifier("$signed-char"), TYPE_SPECIAL, 0);
Declaration gUnsignedChar(TREEALLOCATOR_NULL, 0, makeIdentifier("$unsigned-char"), TYPE_SPECIAL, 0);
Declaration gSignedShortInt(TREEALLOCATOR_NULL, 0, makeIdentifier("$signed-short-int"), TYPE_SPECIAL, 0);
Declaration gUnsignedShortInt(TREEALLOCATOR_NULL, 0, makeIdentifier("$unsigned-short-int"), TYPE_SPECIAL, 0);
Declaration gSignedInt(TREEALLOCATOR_NULL, 0, makeIdentifier("$signed-int"), TYPE_SPECIAL, 0);
Declaration gUnsignedInt(TREEALLOCATOR_NULL, 0, makeIdentifier("$unsigned-int"), TYPE_SPECIAL, 0);
Declaration gSignedLongInt(TREEALLOCATOR_NULL, 0, makeIdentifier("$signed-long-int"), TYPE_SPECIAL, 0);
Declaration gUnsignedLongInt(TREEALLOCATOR_NULL, 0, makeIdentifier("$unsigned-long-int"), TYPE_SPECIAL, 0);
Declaration gSignedLongLongInt(TREEALLOCATOR_NULL, 0, makeIdentifier("$signed-long-long-int"), TYPE_SPECIAL, 0);
Declaration gUnsignedLongLongInt(TREEALLOCATOR_NULL, 0, makeIdentifier("$unsigned-long-long-int"), TYPE_SPECIAL, 0);
Declaration gWCharT(TREEALLOCATOR_NULL, 0, makeIdentifier("$wchar_t"), TYPE_SPECIAL, 0);
Declaration gBool(TREEALLOCATOR_NULL, 0, makeIdentifier("$bool"), TYPE_SPECIAL, 0);
Declaration gFloat(TREEALLOCATOR_NULL, 0, makeIdentifier("$float"), TYPE_SPECIAL, 0);
Declaration gDouble(TREEALLOCATOR_NULL, 0, makeIdentifier("$double"), TYPE_SPECIAL, 0);
Declaration gLongDouble(TREEALLOCATOR_NULL, 0, makeIdentifier("$long-double"), TYPE_SPECIAL, 0);
Declaration gVoid(TREEALLOCATOR_NULL, 0, makeIdentifier("$void"), TYPE_SPECIAL, 0);

// template placeholders
Declaration gDependentType(TREEALLOCATOR_NULL, 0, makeIdentifier("$type"), TYPE_SPECIAL, 0);
Declaration gDependentObject(TREEALLOCATOR_NULL, 0, makeIdentifier("$object"), TYPE_UNKNOWN, 0);
Declaration gDependentTemplate(TREEALLOCATOR_NULL, 0, makeIdentifier("$template"), TYPE_SPECIAL, 0, DeclSpecifiers(), true);
Declaration gDependentNested(TREEALLOCATOR_NULL, 0, makeIdentifier("$nested"), TYPE_SPECIAL, 0);

Declaration gParam(TREEALLOCATOR_NULL, 0, makeIdentifier("$param"), TYPE_CLASS, 0);

// objects
Identifier gOperatorFunctionId = makeIdentifier("operator <op>");
Identifier gConversionFunctionId = makeIdentifier("operator T");
Identifier gOperatorFunctionTemplateId = makeIdentifier("operator () <>");
// TODO: don't declare if id is anonymous?
Identifier gAnonymousId = makeIdentifier("$anonymous");
