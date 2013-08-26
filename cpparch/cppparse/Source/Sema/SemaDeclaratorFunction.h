
#ifndef INCLUDED_CPPPARSE_SEMA_SEMADECLARATORFUNCTION_H
#define INCLUDED_CPPPARSE_SEMA_SEMADECLARATORFUNCTION_H

#include "SemaCommon.h"

struct SemaParameterDeclarationList : public SemaBase
{
	SEMA_BOILERPLATE;

	Parameters parameters;

	SemaParameterDeclarationList(const SemaState& state)
		: SemaBase(state)
	{
	}

	SEMA_POLICY_ARGS(cpp::parameter_declaration, SemaPolicyPushBool<struct SemaSimpleDeclaration>, true)
	void action(cpp::parameter_declaration* symbol, const SemaSimpleDeclarationResult& walker)
	{
		if(!isVoidParameter(walker.declaration->type))
		{
			parameters.push_back(Parameter(walker.declaration, walker.defaultArgument));
		}
	}
};

struct SemaParameterDeclarationClause : public SemaBase
{
	SEMA_BOILERPLATE;

	Parameters parameters;

	SemaParameterDeclarationClause(const SemaState& state)
		: SemaBase(state)
	{
		pushScope(newScope(makeIdentifier("$prototype"), SCOPETYPE_PROTOTYPE));
		if(templateParamScope != 0)
		{
			// insert the template-parameter scope to enclose the declarator scope
			templateParamScope->parent = enclosing->parent;
			enclosing->parent = templateParamScope;
			enclosing->templateDepth = templateParamScope->templateDepth;
		}
		clearTemplateParams();
	}

	SEMA_POLICY(cpp::parameter_declaration_list, SemaPolicyPush<struct SemaParameterDeclarationList>)
	void action(cpp::parameter_declaration_list* symbol, const SemaParameterDeclarationList& walker)
	{
		parameters = walker.parameters;
	}
	void action(cpp::terminal<boost::wave::T_ELLIPSIS> symbol)
	{
		parameters.isEllipsis = true;
	}
};

struct SemaExceptionSpecification : public SemaBase
{
	SEMA_BOILERPLATE;

	SemaExceptionSpecification(const SemaState& state)
		: SemaBase(state)
	{
	}
	SEMA_POLICY(cpp::type_id, SemaPolicyPushCommit<struct SemaTypeId>)
	void action(cpp::type_id* symbol, const SemaTypeIdResult& walker)
	{
	}
};

struct SemaCvQualifierSeq : SemaBase
{
	SEMA_BOILERPLATE;

	CvQualifiers qualifiers;
	SemaCvQualifierSeq(const SemaState& state)
		: SemaBase(state)
	{
	}
	SEMA_POLICY(cpp::cv_qualifier, SemaPolicyIdentity)
	void action(cpp::cv_qualifier* symbol)
	{
		if(symbol->id == cpp::cv_qualifier::CONST)
		{
			qualifiers.isConst = true;
		}
		else if(symbol->id == cpp::cv_qualifier::VOLATILE)
		{
			qualifiers.isVolatile = true;
		}
	}

};

struct SemaDeclaratorFunction : public SemaBase
{
	SEMA_BOILERPLATE;

	ScopePtr paramScope;
	Parameters parameters;
	CvQualifiers qualifiers;
	SemaDeclaratorFunction(const SemaState& state)
		: SemaBase(state), paramScope(0)
	{
	}

	SEMA_POLICY(cpp::parameter_declaration_clause, SemaPolicyPush<struct SemaParameterDeclarationClause>)
	void action(cpp::parameter_declaration_clause* symbol, const SemaParameterDeclarationClause& walker)
	{
		paramScope = walker.enclosing; // store reference for later resumption
		parameters = walker.parameters;
	}
	SEMA_POLICY(cpp::exception_specification, SemaPolicyPush<struct SemaExceptionSpecification>)
	void action(cpp::exception_specification* symbol, const SemaExceptionSpecification& walker)
	{
	}
	SEMA_POLICY(cpp::cv_qualifier_seq, SemaPolicyPush<struct SemaCvQualifierSeq>)
	void action(cpp::cv_qualifier_seq* symbol, const SemaCvQualifierSeq& walker)
	{
		qualifiers = walker.qualifiers;
	}
};

#endif
