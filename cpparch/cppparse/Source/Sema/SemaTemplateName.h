
#ifndef INCLUDED_CPPPARSE_SEMA_SEMATEMPLATENAME_H
#define INCLUDED_CPPPARSE_SEMA_SEMATEMPLATENAME_H

#include "SemaCommon.h"
#include "SemaOperator.h"
#include "SemaTypeName.h"

struct SemaTemplateArgumentList : public SemaBase
{
	SEMA_BOILERPLATE;

	TemplateArgument argument;
	TemplateArguments arguments;
	Once committed;

	SemaTemplateArgumentList(const SemaState& state)
		: SemaBase(state), argument(context), arguments(context)
	{
		clearQualifying();
	}
	void commit()
	{
		committed();
		arguments.push_front(argument); // allocates last element first!
	}
	SEMA_POLICY(cpp::type_id, SemaPolicyPushCommit<struct SemaTypeId>)
	void action(cpp::type_id* symbol, const SemaTypeIdResult& walker)
	{
		walker.committed.test();
		argument.type = walker.type;
		argument.source = getLocation();
	}
	SEMA_POLICY(cpp::assignment_expression, SemaPolicyPushChecked<struct SemaExpression>)
	bool action(cpp::assignment_expression* symbol, const SemaExpressionResult& walker)
	{
		if(walker.expression.isTemplateArgumentAmbiguity)
		{
			// [temp.arg] In a template argument, an ambiguity between a typeid and an expression is resolved to a typeid
			return false; // fail parse, will retry for a type-id
		}
		addDependent(argument.valueDependent, walker.valueDependent);
		argument.type = &gNonType;
		argument.expression = walker.expression;
		argument.source = getLocation();
		return true;
	}
	SEMA_POLICY(cpp::template_argument_list, SemaPolicyPushCommit<struct SemaTemplateArgumentList>)
	void action(cpp::template_argument_list* symbol, const SemaTemplateArgumentList& walker)
	{
		walker.committed.test();
		arguments = walker.arguments;
	}
};

struct SemaTemplateId : public SemaBase, SemaTemplateIdResult
{
	SEMA_BOILERPLATE;

	bool isTemplate; // true if the template-id is preceded by 'template'
	SemaTemplateId(const SemaState& state, bool isTemplate = false)
		: SemaBase(state), SemaTemplateIdResult(context), isTemplate(isTemplate)
	{
	}
	template<typename T>
	bool verifyTemplateName(T* symbol)
	{
		// [temp.names]
		// After name lookup (3.4) finds that a name is a template-name or that an operator-function-id or a literal-operator-
		// id refers to a set of overloaded functions any member of which is a function template if this is
		// followed by a <, the < is always taken as the delimiter of a template-argument-list and never as the less-than
		// operator.
		if(!isTemplate // if the name is not preceded by 'template'
			&& !isDependentOld(qualifying_p)) // and the name is not qualified by a dependent type
		{
			if(qualifyingClass == 0
				&& getQualifyingScope() != 0
				&& getQualifyingScope()->type == SCOPETYPE_CLASS) // special case for declarator qualified by type-name
			{
				// TODO: unify lookup for declarator qualified by type name - store dependent qualifyingClass
				return true; // don't bother checking for a template-name, the following cannot be 'less-than'.. we hope.
			}

			// search qualifying type/namespace, object-expression and/or enclosing scope depending on context
			LookupResultRef declaration = findDeclaration(symbol->value, IsAny());
			if(declaration == &gUndeclared
				|| !isTemplateName(*declaration))
			{
				return reportIdentifierMismatch(symbol, symbol->value, declaration, "template-name");
			}
		}
		return true;
	}
	SEMA_POLICY(cpp::identifier, SemaPolicyIdentityCachedChecked)
	bool action(cpp::identifier* symbol)
	{
		id = &symbol->value;
		return verifyTemplateName(symbol);
	}
	SEMA_POLICY(cpp::operator_function_id, SemaPolicyPushIdChecked<struct SemaOperatorFunctionId>)
	bool action(cpp::operator_function_id* symbol, SemaOperatorFunctionId& walker) 
	{
		symbol->value.value = walker.name;
		id = &symbol->value;
		return verifyTemplateName(symbol);
	}
	SEMA_POLICY(cpp::template_argument_clause, SemaPolicyPushCached<struct SemaTemplateArgumentList>)
	void action(cpp::template_argument_clause* symbol, SemaTemplateArgumentList& walker)
	{
		arguments = walker.arguments;
	}
};



struct SemaTypenameSpecifier : public SemaQualified, SemaTypenameSpecifierResult
{
	SEMA_BOILERPLATE;

	SemaTypenameSpecifier(const SemaState& state)
		: SemaQualified(state), SemaTypenameSpecifierResult(context)
	{
	}

	void action(cpp::terminal<boost::wave::T_TYPENAME> symbol)
	{
	}
	void action(cpp::terminal<boost::wave::T_COLON_COLON> symbol)
	{
		setQualifyingGlobal();
	}
	void action(cpp::terminal<boost::wave::T_TEMPLATE> symbol)
	{
		// TODO
	}
	SEMA_POLICY(cpp::nested_name_specifier, SemaPolicyPushCached<struct SemaNestedNameSpecifier>)
	void action(cpp::nested_name_specifier* symbol, const SemaQualifyingResult& walker)
	{
		swapQualifying(walker.qualifying);
	}
	SEMA_POLICY_ARGS(cpp::type_name, SemaPolicyPushCheckedBool<struct SemaTypeName>, true)
	bool action(cpp::type_name* symbol, const SemaTypeName& walker)
	{
		if(walker.filter.nonType != 0)
		{
			return reportIdentifierMismatch(symbol, walker.filter.nonType->getName(), walker.filter.nonType, "type-name");
		}
		type = walker.type;
		type.qualifying.swap(qualifying);
		setDependent(type.dependent, type.qualifying);
		return true;
	}
};

#endif
