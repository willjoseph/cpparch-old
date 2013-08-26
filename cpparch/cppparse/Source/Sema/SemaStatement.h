
#ifndef INCLUDED_CPPPARSE_SEMA_SEMASTATEMENT_H
#define INCLUDED_CPPPARSE_SEMA_SEMASTATEMENT_H

#include "SemaCommon.h"


struct SemaLabeledStatement : public SemaBase
{
	SEMA_BOILERPLATE;

	SemaLabeledStatement(const SemaState& state)
		: SemaBase(state)
	{
	}
	SEMA_POLICY(cpp::identifier, SemaPolicyIdentity)
	void action(cpp::identifier* symbol)
	{
		// TODO: goto label
	}
	SEMA_POLICY(cpp::constant_expression, SemaPolicyPush<struct SemaExpression>)
	void action(cpp::constant_expression* symbol, const SemaExpressionResult& walker)
	{
		SEMANTIC_ASSERT(walker.expression.isConstant); // TODO: non-fatal error: expected constant expression
	}
	SEMA_POLICY(cpp::statement, SemaPolicyPush<struct SemaStatement>)
	void action(cpp::statement* symbol, const SemaStatement& walker)
	{
	}
};

struct SemaStatement : public SemaBase
{
	SEMA_BOILERPLATE;

	SemaStatement(const SemaState& state)
		: SemaBase(state)
	{
	}
	SEMA_POLICY(cpp::simple_declaration, SemaPolicyPush<struct SemaSimpleDeclaration>)
	void action(cpp::simple_declaration* symbol, const SemaSimpleDeclarationResult& walker)
	{
	}
	SEMA_POLICY(cpp::try_block, SemaPolicyPush<struct SemaTryBlock>)
	void action(cpp::try_block* symbol, const SemaTryBlock& walker)
	{
	}
	SEMA_POLICY(cpp::namespace_alias_definition, SemaPolicyPush<struct SemaNamespaceAliasDefinition>)
	void action(cpp::namespace_alias_definition* symbol, const SemaNamespaceAliasDefinition& walker)
	{
	}
	SEMA_POLICY(cpp::selection_statement, SemaPolicyPush<struct SemaControlStatement>)
	void action(cpp::selection_statement* symbol, const SemaControlStatement& walker)
	{
	}
	SEMA_POLICY(cpp::iteration_statement, SemaPolicyPush<struct SemaControlStatement>)
	void action(cpp::iteration_statement* symbol, const SemaControlStatement& walker)
	{
	}
	SEMA_POLICY(cpp::compound_statement, SemaPolicyPush<struct SemaCompoundStatement>)
	void action(cpp::compound_statement* symbol, const SemaCompoundStatement& walker)
	{
	}
	SEMA_POLICY(cpp::expression_statement, SemaPolicyPush<struct SemaExpression>)
	void action(cpp::expression_statement* symbol, const SemaExpressionResult& walker)
	{
	}
	SEMA_POLICY(cpp::jump_statement_return, SemaPolicyPush<struct SemaExpression>)
	void action(cpp::jump_statement_return* symbol, const SemaExpressionResult& walker)
	{
	}
	SEMA_POLICY(cpp::jump_statement_goto, SemaPolicyIdentity)
	void action(cpp::jump_statement_goto* symbol)
	{
		// TODO
	}
	SEMA_POLICY(cpp::labeled_statement, SemaPolicyPush<struct SemaLabeledStatement>)
	void action(cpp::labeled_statement* symbol, const SemaLabeledStatement& walker)
	{
	}
	SEMA_POLICY(cpp::using_declaration, SemaPolicyPush<struct SemaUsingDeclaration>)
	void action(cpp::using_declaration* symbol, const SemaUsingDeclaration& walker)
	{
	}
	SEMA_POLICY(cpp::using_directive, SemaPolicyPush<struct SemaUsingDirective>)
	void action(cpp::using_directive* symbol, const SemaUsingDirective& walker)
	{
	}
};

struct SemaControlStatement : public SemaBase
{
	SEMA_BOILERPLATE;

	SemaControlStatement(const SemaState& state)
		: SemaBase(state)
	{
	}
	void action(cpp::terminal<boost::wave::T_LEFTPAREN> symbol)
	{
		pushScope(newScope(enclosing->getUniqueName(), SCOPETYPE_LOCAL));
	}
	SEMA_POLICY(cpp::condition_init, SemaPolicyPush<struct SemaSimpleDeclaration>)
	void action(cpp::condition_init* symbol, const SemaSimpleDeclarationResult& walker)
	{
	}
	SEMA_POLICY(cpp::simple_declaration, SemaPolicyPush<struct SemaSimpleDeclaration>)
	void action(cpp::simple_declaration* symbol, const SemaSimpleDeclarationResult& walker)
	{
	}
	SEMA_POLICY(cpp::statement, SemaPolicyPush<struct SemaStatement>)
	void action(cpp::statement* symbol, const SemaStatement& walker)
	{
	}
	SEMA_POLICY(cpp::expression, SemaPolicyPush<struct SemaExpression>)
	void action(cpp::expression* symbol, const SemaExpressionResult& walker)
	{
	}
};

struct SemaCompoundStatement : public SemaBase
{
	SEMA_BOILERPLATE;

	SemaCompoundStatement(const SemaState& state)
		: SemaBase(state)
	{
	}

	void action(cpp::terminal<boost::wave::T_LEFTBRACE> symbol)
	{
		pushScope(newScope(enclosing->getUniqueName(), SCOPETYPE_LOCAL));
	}
	SEMA_POLICY(cpp::statement, SemaPolicyPush<struct SemaStatement>)
	void action(cpp::statement* symbol, const SemaStatement& walker)
	{
	}
};

struct SemaStatementSeq : public SemaBase
{
	SEMA_BOILERPLATE;

	SemaStatementSeq(const SemaState& state)
		: SemaBase(state)
	{
	}
	SEMA_POLICY(cpp::statement, SemaPolicyPush<struct SemaStatement>)
	void action(cpp::statement* symbol, const SemaStatement& walker)
	{
	}
};


#endif
