
#ifndef INCLUDED_CPPPARSE_SEMA_SEMAIDEXPRESSION_H
#define INCLUDED_CPPPARSE_SEMA_SEMAIDEXPRESSION_H

#include "SemaCommon.h"
#include "SemaOperator.h"

struct SemaUnqualifiedId : public SemaBase
{
	SEMA_BOILERPLATE;

	LookupResultRef declaration;
	IdentifierPtr id;
	TemplateArguments arguments; // only used if the identifier is a template-name
	bool isIdentifier;
	bool isTemplate;
	SemaUnqualifiedId(const SemaState& state, bool isTemplate = false)
		: SemaBase(state), id(0), arguments(context), isIdentifier(false), isTemplate(isTemplate)
	{
	}
	SEMA_POLICY(cpp::identifier, SemaPolicyIdentityCached)
	void action(cpp::identifier* symbol)
	{
		id = &symbol->value;
		isIdentifier = true;
		if(allowNameLookup())
		{
			declaration = findDeclaration(*id, IsAny(), true);
		}
	}
	SEMA_POLICY_ARGS(cpp::simple_template_id, SemaPolicyPushCachedCheckedBool<struct SemaTemplateId>, isTemplate)
	bool action(cpp::simple_template_id* symbol, const SemaTemplateIdResult& walker)
	{
		if(allowNameLookup())
		{
			LookupResultRef declaration = findDeclaration(*walker.id, IsAny(), true);
			if(declaration == &gUndeclared
				|| !isTemplateName(*declaration))
			{
				return reportIdentifierMismatch(symbol, *walker.id, declaration, "template-name");
			}
			this->declaration = declaration;
		}
		id = walker.id;
		arguments = walker.arguments;
		return true;
	}
	SEMA_POLICY_ARGS(cpp::template_id_operator_function, SemaPolicyPushCachedCheckedBool<struct SemaTemplateId>, isTemplate)
	bool action(cpp::template_id_operator_function* symbol, const SemaTemplateIdResult& walker)
	{
		if(allowNameLookup())
		{
			LookupResultRef declaration = findDeclaration(*walker.id, IsAny(), true);
			if(declaration == &gUndeclared
				|| !isTemplateName(*declaration))
			{
				return reportIdentifierMismatch(symbol, *walker.id, declaration, "template-name");
			}
			this->declaration = declaration;
		}
		id = walker.id;
		arguments = walker.arguments;
		return true;
	}
	SEMA_POLICY(cpp::operator_function_id, SemaPolicyPushId<struct SemaOperatorFunctionId>)
	void action(cpp::operator_function_id* symbol, const SemaOperatorFunctionId& walker)
	{
		symbol->value.value = walker.name;
		id = &symbol->value;
		if(allowNameLookup())
		{
			declaration = findDeclaration(*id, IsAny(), true);
			if(declaration == &gUndeclared
				&& id->value == gOperatorAssignId)
			{
				// TODO: declare operator= if not already declared
				declaration = gCopyAssignmentOperatorInstance;
			}
		}
	}
	SEMA_POLICY(cpp::conversion_function_id, SemaPolicyPushIdCommit<struct SemaTypeId>)
	void action(cpp::conversion_function_id* symbol, const SemaTypeIdResult& walker)
	{
		walker.committed.test();
		symbol->value.value = gConversionFunctionId;
		id = &symbol->value;
		if(allowNameLookup())
		{
			declaration = findDeclaration(*id, IsAny(), true);
		}
	}
	SEMA_POLICY(cpp::destructor_id, SemaPolicyIdentityChecked)
	bool action(cpp::destructor_id* symbol)
	{
		// TODO: can destructor-id be dependent?
		id = &symbol->name->value;
		if(objectExpression.p == 0)
		{
			// destructor id can only appear in class member access expression
			return reportIdentifierMismatch(symbol, *id, declaration, "class member access expression");
		}
		declaration = gDestructorInstance;
		return true;
	}
};

struct SemaQualifiedId : public SemaQualified
{
	SEMA_BOILERPLATE;

	LookupResultRef declaration;
	IdentifierPtr id;
	TemplateArguments arguments; // only used if the identifier is a template-name
	bool isTemplate;
	SemaQualifiedId(const SemaState& state)
		: SemaQualified(state), id(0), arguments(context), isTemplate(false)
	{
	}

	void action(cpp::terminal<boost::wave::T_COLON_COLON> symbol)
	{
		setQualifyingGlobal();
	}
	SEMA_POLICY(cpp::nested_name_specifier, SemaPolicyPushCached<struct SemaNestedNameSpecifier>)
	void action(cpp::nested_name_specifier* symbol, const SemaQualifyingResult& walker)
	{
		swapQualifying(walker.qualifying);
	}
	void action(cpp::terminal<boost::wave::T_TEMPLATE> symbol)
	{
		isTemplate = true;
	}

	SEMA_POLICY_ARGS(cpp::unqualified_id, SemaPolicyPushBool<struct SemaUnqualifiedId>, isTemplate)
	void action(cpp::unqualified_id* symbol, const SemaUnqualifiedId& walker)
	{
		declaration = walker.declaration;
		id = walker.id;
		arguments = walker.arguments;
	}
	SEMA_POLICY_ARGS(cpp::qualified_id_suffix, SemaPolicyPushBool<struct SemaUnqualifiedId>, isTemplate)
	void action(cpp::qualified_id_suffix* symbol, const SemaUnqualifiedId& walker)
	{
		declaration = walker.declaration;
		id = walker.id;
		arguments = walker.arguments;
	}
};


struct SemaIdExpression : public SemaQualified
{
	SEMA_BOILERPLATE;

	/* 14.6.2.2-3
	An id-expression is type-dependent if it contains:
	- an identifier that was declared with a dependent type,
	- a template-id that is dependent,
	- a conversion-function-id that specifies a dependent type,
	- a nested-name-specifier or a qualified-id that names a member of an unknown specialization
	*/
	ExpressionWrapper expression;
	LookupResultRef declaration;
	IdentifierPtr id;
	TemplateArguments arguments; // only used if the identifier is a template-name
	Dependent typeDependent;
	Dependent valueDependent;
	bool isIdentifier;
	bool isUndeclared;
	bool isTemplate;
	SemaIdExpression(const SemaState& state, bool isTemplate = false)
		: SemaQualified(state), id(0), arguments(context), isIdentifier(false), isUndeclared(false), isTemplate(isTemplate)
	{
	}
	SEMA_POLICY(cpp::qualified_id_default, SemaPolicyPush<struct SemaQualifiedId>)
	void action(cpp::qualified_id_default* symbol, const SemaQualifiedId& walker)
	{
		declaration = walker.declaration;
		id = walker.id;
		arguments = walker.arguments;
		swapQualifying(walker.qualifying);
	}
	SEMA_POLICY(cpp::qualified_id_global, SemaPolicyPush<struct SemaQualifiedId>)
	void action(cpp::qualified_id_global* symbol, const SemaQualifiedId& walker)
	{
		declaration = walker.declaration;
		id = walker.id;
		arguments = walker.arguments;
		swapQualifying(walker.qualifying);
	}
	SEMA_POLICY(cpp::qualified_id, SemaPolicyIdentity)
	void action(cpp::qualified_id* symbol)
	{
		// [temp.dep.expr] An id-expression is type-dependent if it contains:- a nested-name-specifier that contains a class-name that names a dependent type
		setDependent(typeDependent, qualifying.get());
		setDependent(valueDependent, qualifying.get()); // it's clearly value-dependent too, because name lookup must be deferred
	}
	SEMA_POLICY_ARGS(cpp::unqualified_id, SemaPolicyPushBool<struct SemaUnqualifiedId>, isTemplate)
	void action(cpp::unqualified_id* symbol, const SemaUnqualifiedId& walker)
	{
		declaration = walker.declaration;
		id = walker.id;
		arguments = walker.arguments;
		isIdentifier = walker.isIdentifier;
	}
	bool commit()
	{
		UniqueTypeWrapper qualifyingType = makeUniqueQualifying(qualifying, getInstantiationContext(), isDependent(qualifying.get_ref()));

		TemplateArgumentsInstance templateArguments;
		makeUniqueTemplateArguments(arguments, templateArguments, getInstantiationContext(), isDependent(arguments));

		expression = ExpressionWrapper();

		if(isDependent(typeDependent)
			|| objectExpressionIsDependent())
		{
			setDecoration(id, gDependentObjectInstance);

			expression = makeExpression(DependentIdExpression(id->value, qualifyingType, templateArguments),
				true, // TODO: expression depending on template parameter may or may not be an integral constant expression
				true,
				true
			);
		}
		else if(isIdentifier // the expression is 'identifier'
			&& declaration == &gUndeclared) // the identifier was not previously declared
		{
			// defer name-lookup: this may be the id-expression in a dependent call to named function, to be found by ADL
			isUndeclared = true;
			setDecoration(id, gDependentObjectInstance);

			expression = makeExpression(DependentIdExpression(id->value, gUniqueTypeNull, TemplateArgumentsInstance()), false, true);
		}
		else
		{
			SEMANTIC_ASSERT(declaration != 0);

			if(declaration == &gUndeclared
				|| !isObject(*declaration))
			{
				return false;
			}

			// [temp.dep.expr] An id-expression is type-dependent if it contains:- an identifier that was declared with a dependent type
			addDependentType(typeDependent, declaration);
			// [temp.dep.expr] An id-expression is type-dependent if it contains: - a template-id that is dependent
			setDependent(typeDependent, arguments); // the id-expression may have an explicit template argument list
			// [temp.dep.expr] An id-expression is type-dependent if it contains: - an identifier associated by name lookup with one or more declarations declared with a dependent type,
			addDependentOverloads(typeDependent, declaration);

			// [temp.dep.constexpr] An identifier is value-dependent if it is:- a name declared with a dependent type
			addDependentType(valueDependent, declaration);
			// [temp.dep.constexpr] An identifier is value-dependent if it is:- the name of a non-type template parameter,
			// - a constant with integral or enumeration type and is initialized with an expression that is value-dependent.
			addDependentName(valueDependent, declaration); // adds 'declaration' if it names a non-type template-parameter; adds a dependent initializer


			setDecoration(id, declaration);

			SEMANTIC_ASSERT(!isDependent(qualifying.get_ref()));

			const SimpleType* qualifyingClass = qualifyingType == gUniqueTypeNull ? 0 : &getSimpleType(qualifyingType.value);
			SEMANTIC_ASSERT(qualifyingClass == this->qualifyingClass);

			SEMANTIC_ASSERT(declaration->templateParameter == INDEX_INVALID || qualifying.empty()); // template params cannot be qualified
			expression = declaration->templateParameter == INDEX_INVALID
				// TODO: check compliance: id-expression cannot be compared for equivalence unless it names a non-type template-parameter
				? makeExpression(IdExpression(declaration, qualifyingClass, templateArguments), false, isDependent(typeDependent), isDependent(valueDependent))
				: makeExpression(NonTypeTemplateParameter(declaration), true, isDependent(typeDependent), isDependent(valueDependent));

			expression.isNonStaticMemberName = isMember(*declaration) && !isStatic(*declaration);
			expression.isQualifiedNonStaticMemberName = expression.isNonStaticMemberName && qualifyingType != gUniqueTypeNull;
		}
		return true;
	}
};

#endif
