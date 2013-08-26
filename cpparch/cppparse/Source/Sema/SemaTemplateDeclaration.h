
#ifndef INCLUDED_CPPPARSE_SEMA_SEMATEMPLATEDECLARATION_H
#define INCLUDED_CPPPARSE_SEMA_SEMATEMPLATEDECLARATION_H

#include "SemaCommon.h"
#include "SemaPrimaryExpression.h"

struct SemaTypeParameter : public SemaBase
{
	SEMA_BOILERPLATE;

	TemplateParameter param; // result
	IdentifierPtr id;
	TemplateArgument argument; // the default argument for this param
	TemplateParameters params; // the template parameters for this param (if template-template-param)
	size_t templateParameter;
	Once committed;
	SemaTypeParameter(const SemaState& state, size_t templateParameter)
		: SemaBase(state), param(context), id(&gAnonymousId), argument(context), params(context), templateParameter(templateParameter)
	{
	}
	void commit()
	{
		committed();
		SEMANTIC_ASSERT(param.declaration == 0); // may only be called once, after parse of type-parameter succeeds
		DeclarationInstanceRef instance = pointOfDeclaration(context, enclosing, *id, TYPE_PARAM, 0, DECLSPEC_TYPEDEF, !params.empty(), params, false, TEMPLATEARGUMENTS_NULL, templateParameter);
#ifdef ALLOCATOR_DEBUG
		trackDeclaration(instance);
#endif
		if(id != &gAnonymousId)
		{
			setDecoration(id, instance);
		}
		param = instance;
		setDependent(param);
		makeUniqueTypeSafe(param);
		param.argument.swap(argument);
	}
	SEMA_POLICY(cpp::identifier, SemaPolicyIdentity)
	void action(cpp::identifier* symbol)
	{
		id = &symbol->value;
	}
	SEMA_POLICY(cpp::type_id, SemaPolicyPushCommit<struct SemaTypeId>)
	void action(cpp::type_id* symbol, const SemaTypeIdResult& walker)
	{
		SEMANTIC_ASSERT(params.empty());
		walker.committed.test();
		argument.type = walker.type;
	}
	SEMA_POLICY(cpp::template_parameter_clause, SemaPolicyPush<struct SemaTemplateParameterClause>)
	void action(cpp::template_parameter_clause* symbol, const SemaTemplateParameterClauseResult& walker)
	{
		params = walker.params;
	}
	SEMA_POLICY(cpp::id_expression, SemaPolicyPushChecked<struct SemaIdExpression>)
	bool action(cpp::id_expression* symbol, const SemaIdExpression& walker) // the default argument for a template-template-parameter
	{
		LookupResultRef declaration = walker.declaration;
		if(declaration != 0) // TODO: error, lookup failed?
		{
			if(declaration == &gUndeclared
				|| !isTemplateName(*declaration))
			{
				return reportIdentifierMismatch(symbol, *walker.id, declaration, "template-name");
			}
			setDecoration(walker.id, declaration);
		}
		return true;
	}
};

struct SemaTemplateParameterList : public SemaBase
{
	SEMA_BOILERPLATE;

	TemplateParameter param; // internal state
	TemplateParameters params; // result
	size_t count; // internal state
	Once committed;
	SemaTemplateParameterList(const SemaState& state, size_t count)
		: SemaBase(state), param(context), params(context), count(count)
	{
	}
	void commit()
	{
		committed();
		params.push_front(param);
	}
	SEMA_POLICY_ARGS(cpp::type_parameter_default, SemaPolicyPushIndexCommit<struct SemaTypeParameter>, count)
	void action(cpp::type_parameter_default* symbol, const SemaTypeParameter& walker)
	{
		walker.committed.test();
		param = walker.param;
		++count;
	}
	SEMA_POLICY_ARGS(cpp::type_parameter_template, SemaPolicyPushIndexCommit<struct SemaTypeParameter>, count)
	void action(cpp::type_parameter_template* symbol, const SemaTypeParameter& walker)
	{
		walker.committed.test();
		param = walker.param;
		++count;
	}
	SEMA_POLICY_ARGS(cpp::parameter_declaration, SemaPolicyParameterDeclaration<struct SemaSimpleDeclaration>, SemaDeclarationArgs(false, count))
	void action(cpp::parameter_declaration* symbol, const SemaSimpleDeclarationResult& walker)
	{
		SEMANTIC_ASSERT(walker.declaration != 0);
		param = walker.declaration;
		if(walker.defaultArgument != 0)
		{
			addDependent(param.argument.valueDependent, walker.declaration->valueDependent);
			param.argument.type = &gNonType;
			param.argument.expression = walker.declaration->initializer;
		}
		++count;
	}
	SEMA_POLICY_ARGS(cpp::template_parameter_list, SemaPolicyPushIndexCommit<struct SemaTemplateParameterList>, count)
	void action(cpp::template_parameter_list* symbol, const SemaTemplateParameterList& walker)
	{
		walker.committed.test();
		params = walker.params;
	}
};

struct SemaTemplateParameterClause : public SemaBase, SemaTemplateParameterClauseResult
{
	SEMA_BOILERPLATE;

	SemaTemplateParameterClause(const SemaState& state)
		: SemaBase(state), SemaTemplateParameterClauseResult(context)
	{
		// collect template-params into a new scope
		if(templateParamScope != 0)
		{
			pushScope(templateParamScope); // the existing template parameter scope encloses the new scope
		}
		pushScope(newScope(makeIdentifier("$template"), SCOPETYPE_TEMPLATE));
		clearTemplateParams();
		enclosing->templateDepth = templateDepth;
		enclosingDeferred = 0; // don't defer parse of default-argument for non-type template-parameter
	}
	SEMA_POLICY_ARGS(cpp::template_parameter_list, SemaPolicyPushIndexCommit<struct SemaTemplateParameterList>, 0)
	void action(cpp::template_parameter_list* symbol, const SemaTemplateParameterList& walker)
	{
		walker.committed.test();
		params = walker.params;
	}
};

struct SemaTemplateDeclaration : public SemaBase, SemaDeclarationResult
{
	SEMA_BOILERPLATE;

	TemplateParameters params; // internal state
	SemaTemplateDeclaration(const SemaState& state)
		: SemaBase(state), params(context)
	{
		++templateDepth;
		templateParams = &TEMPLATEPARAMETERS_NULL; // explicit specialization has empty template params: template<> struct S;
	}
	SEMA_POLICY(cpp::template_parameter_clause, SemaPolicyPush<struct SemaTemplateParameterClause>)
	void action(cpp::template_parameter_clause* symbol, const SemaTemplateParameterClause& walker)
	{
		templateParamScope = walker.enclosing;
		enclosing = walker.enclosing->parent;
		params = walker.params;
		templateParams = &params;
	}
	SEMA_POLICY(cpp::declaration, SemaPolicyPushSrc<struct SemaDeclaration>)
	void action(cpp::declaration* symbol, const SemaDeclarationResult& walker)
	{
		declaration = walker.declaration;
		SEMANTIC_ASSERT(declaration != 0);
	}
	SEMA_POLICY(cpp::member_declaration, SemaPolicyPush<struct SemaMemberDeclaration>)
	void action(cpp::member_declaration* symbol, const SemaMemberDeclarationResult& walker)
	{
		declaration = walker.declaration;
		SEMANTIC_ASSERT(declaration != 0);
	}
};


struct SemaExplicitInstantiation : public SemaBase, SemaExplicitInstantiationResult
{
	SEMA_BOILERPLATE;

	SemaExplicitInstantiation(const SemaState& state)
		: SemaBase(state)
	{
	}
	void action(cpp::terminal<boost::wave::T_TEMPLATE> symbol)
	{
		isExplicitInstantiation = true;
	}
	SEMA_POLICY(cpp::declaration, SemaPolicyPush<struct SemaDeclaration>)
	void action(cpp::declaration* symbol, const SemaDeclarationResult& walker)
	{
		declaration = walker.declaration;
	}
};

#endif
