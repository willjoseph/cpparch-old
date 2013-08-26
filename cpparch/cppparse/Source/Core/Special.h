
#ifndef INCLUDED_CPPPARSE_CORE_SPECIAL_H
#define INCLUDED_CPPPARSE_CORE_SPECIAL_H

#include "Ast/Type.h"

// special-case
extern Declaration gUndeclared;
extern const DeclarationInstance gUndeclaredInstance;


extern Identifier gGlobalId;
extern Identifier gAnonymousId;
extern Identifier gDestructorId;
extern Name gConversionFunctionId;
extern const DeclarationInstance gCopyAssignmentOperatorInstance;
extern const DeclarationInstance gDestructorInstance;
extern const SimpleType gDependentSimpleType;

extern BuiltInTypeId gImplicitObjectParameter;

extern BuiltInTypeId gArithmeticPlaceholder;
extern BuiltInTypeId gIntegralPlaceholder;
extern BuiltInTypeId gPromotedIntegralPlaceholder;
extern BuiltInTypeId gPromotedArithmeticPlaceholder;
extern BuiltInTypeId gEnumerationPlaceholder;
extern BuiltInTypeId gPointerToAnyPlaceholder;
extern BuiltInTypeId gPointerToObjectPlaceholder;
extern BuiltInTypeId gPointerToClassPlaceholder;
extern BuiltInTypeId gPointerToFunctionPlaceholder;
extern BuiltInTypeId gPointerToMemberPlaceholder;


#endif
