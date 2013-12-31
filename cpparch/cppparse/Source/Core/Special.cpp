
#if BUILD_STANDALONE

#include "Special.h"


// special-case
Identifier gUndeclaredId = makeIdentifier("$undeclared");
Declaration gUndeclared(AST_ALLOCATOR_NULL, 0, gUndeclaredId, TYPE_NULL, 0);
const DeclarationInstance gUndeclaredInstance(&gUndeclared);

Identifier gGlobalId = makeIdentifier("$global");
Identifier gAnonymousId = makeIdentifier("$anonymous");

// special
Identifier gImplicitObjectParameterId = makeIdentifier("$implicit-object-parameter");
BuiltInTypeDeclaration gImplicitObjectParameterDeclaration(gImplicitObjectParameterId, TYPE_UNKNOWN);
BuiltInTypeId gImplicitObjectParameter(&gImplicitObjectParameterDeclaration, AST_ALLOCATOR_NULL);


struct BuiltInFunctionType : TypeId
{
	BuiltInFunctionType(Declaration* declaration, const AstAllocator<int>& allocator)
		: TypeId(declaration, allocator)
	{
		unique = pushBuiltInType(UNIQUETYPE_NULL, FunctionType());
	}
};

Identifier gSpecialMemberFunctionId = makeIdentifier("$special-member-function");
Scope gSpecialMemberFunctionScope(AST_ALLOCATOR_NULL, gSpecialMemberFunctionId, SCOPETYPE_CLASS);
BuiltInTypeDeclaration gSpecialMemberFunctionTypeDeclaration(gSpecialMemberFunctionId, TYPE_UNKNOWN);
BuiltInFunctionType gSpecialMemberFunctionType(&gSpecialMemberFunctionTypeDeclaration, AST_ALLOCATOR_NULL);

Identifier gCopyAssignmentOperatorId = makeIdentifier("$copy-assignment-operator");
Declaration gCopyAssignmentOperatorDeclaration(AST_ALLOCATOR_NULL, &gSpecialMemberFunctionScope, gCopyAssignmentOperatorId, gSpecialMemberFunctionType, 0);
const DeclarationInstance gCopyAssignmentOperatorInstance(&gCopyAssignmentOperatorDeclaration);

Identifier gDestructorId = makeIdentifier("$destructor");
Declaration gDestructorDeclaration(AST_ALLOCATOR_NULL, &gSpecialMemberFunctionScope, gDestructorId, gSpecialMemberFunctionType, 0);
const DeclarationInstance gDestructorInstance(&gDestructorDeclaration);

// built-in operator placeholders
Identifier gArithmeticPlaceholderId = makeIdentifier("$arithmetic-placeholder");
BuiltInTypeDeclaration gArithmeticPlaceholderDeclaration(gArithmeticPlaceholderId, TYPE_UNKNOWN);
BuiltInTypeId gArithmeticPlaceholder(&gArithmeticPlaceholderDeclaration, AST_ALLOCATOR_NULL);
Identifier gIntegralPlaceholderId = makeIdentifier("$integral-placeholder");
BuiltInTypeDeclaration gIntegralPlaceholderDeclaration(gIntegralPlaceholderId, TYPE_UNKNOWN);
BuiltInTypeId gIntegralPlaceholder(&gIntegralPlaceholderDeclaration, AST_ALLOCATOR_NULL);
Identifier gPromotedArithmeticPlaceholderId = makeIdentifier("$promoted-arithmetic-placeholder");
BuiltInTypeDeclaration gPromotedArithmeticPlaceholderDeclaration(gPromotedArithmeticPlaceholderId, TYPE_UNKNOWN);
BuiltInTypeId gPromotedArithmeticPlaceholder(&gPromotedArithmeticPlaceholderDeclaration, AST_ALLOCATOR_NULL);
Identifier gPromotedIntegralPlaceholderId = makeIdentifier("$promoted-integral-placeholder");
BuiltInTypeDeclaration gPromotedIntegralPlaceholderDeclaration(gPromotedIntegralPlaceholderId, TYPE_UNKNOWN);
BuiltInTypeId gPromotedIntegralPlaceholder(&gPromotedIntegralPlaceholderDeclaration, AST_ALLOCATOR_NULL);
Identifier gEnumerationPlaceholderId = makeIdentifier("$enumeration-placeholder");
BuiltInTypeDeclaration gEnumerationPlaceholderDeclaration(gEnumerationPlaceholderId, TYPE_UNKNOWN);
BuiltInTypeId gEnumerationPlaceholder(&gEnumerationPlaceholderDeclaration, AST_ALLOCATOR_NULL);
Identifier gPointerToAnyPlaceholderId = makeIdentifier("$pointer-to-any-placeholder");
BuiltInTypeDeclaration gPointerToAnyPlaceholderDeclaration(gPointerToAnyPlaceholderId, TYPE_UNKNOWN);
BuiltInTypeId gPointerToAnyPlaceholder(&gPointerToAnyPlaceholderDeclaration, AST_ALLOCATOR_NULL);
Identifier gPointerToObjectPlaceholderId = makeIdentifier("$pointer-to-object-placeholder");
BuiltInTypeDeclaration gPointerToObjectPlaceholderDeclaration(gPointerToObjectPlaceholderId, TYPE_UNKNOWN);
BuiltInTypeId gPointerToObjectPlaceholder(&gPointerToObjectPlaceholderDeclaration, AST_ALLOCATOR_NULL);
Identifier gPointerToClassPlaceholderId = makeIdentifier("$pointer-to-class-placeholder");
BuiltInTypeDeclaration gPointerToClassPlaceholderDeclaration(gPointerToClassPlaceholderId, TYPE_UNKNOWN);
BuiltInTypeId gPointerToClassPlaceholder(&gPointerToClassPlaceholderDeclaration, AST_ALLOCATOR_NULL);
Identifier gPointerToFunctionPlaceholderId = makeIdentifier("$pointer-to-function-placeholder");
BuiltInTypeDeclaration gPointerToFunctionPlaceholderDeclaration(gPointerToFunctionPlaceholderId, TYPE_UNKNOWN);
BuiltInTypeId gPointerToFunctionPlaceholder(&gPointerToFunctionPlaceholderDeclaration, AST_ALLOCATOR_NULL);
Identifier gPointerToMemberPlaceholderId = makeIdentifier("$member-pointer-placeholder");
BuiltInTypeDeclaration gPointerToMemberPlaceholderDeclaration(gPointerToMemberPlaceholderId, TYPE_UNKNOWN);
BuiltInTypeId gPointerToMemberPlaceholder(&gPointerToMemberPlaceholderDeclaration, AST_ALLOCATOR_NULL);



const SimpleType gDependentSimpleType(&gDependentType, 0);

#endif
