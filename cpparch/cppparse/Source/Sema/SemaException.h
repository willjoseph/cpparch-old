
#ifndef INCLUDED_CPPPARSE_SEMA_SEMAEXCEPTION_H
#define INCLUDED_CPPPARSE_SEMA_SEMAEXCEPTION_H

#include "SemaCommon.h"


struct SemaHandler : public SemaBase
{
	SEMA_BOILERPLATE;

	SemaHandler(const SemaState& state)
		: SemaBase(state)
	{
	}
	void action(cpp::terminal<boost::wave::T_CATCH> symbol)
	{
		pushScope(newScope(enclosing->getUniqueName(), SCOPETYPE_LOCAL));
	}
	SEMA_POLICY(cpp::exception_declaration_default, SemaPolicyPush<struct SemaSimpleDeclaration>)
	void action(cpp::exception_declaration_default* symbol, const SemaSimpleDeclarationResult& walker)
	{
	}
	SEMA_POLICY(cpp::compound_statement, SemaPolicyPush<struct SemaCompoundStatement>)
	void action(cpp::compound_statement* symbol, const SemaCompoundStatement& walker)
	{
	}
};

struct SemaHandlerSeq : public SemaBase
{
	SEMA_BOILERPLATE;

	SemaHandlerSeq(const SemaState& state)
		: SemaBase(state)
	{
	}
	SEMA_POLICY(cpp::handler, SemaPolicyPush<struct SemaHandler>)
	void action(cpp::handler* symbol, const SemaHandler& walker)
	{
	}
};

struct SemaTryBlock : public SemaBase
{
	SEMA_BOILERPLATE;

	SemaTryBlock(const SemaState& state)
		: SemaBase(state)
	{
	}

	SEMA_POLICY(cpp::compound_statement, SemaPolicyPush<struct SemaCompoundStatement>)
	void action(cpp::compound_statement* symbol, const SemaCompoundStatement& walker)
	{
	}
	SEMA_POLICY(cpp::handler_seq, SemaPolicyPush<struct SemaHandlerSeq>)
	void action(cpp::handler_seq* symbol, const SemaHandlerSeq& walker)
	{
	}
};


#endif
