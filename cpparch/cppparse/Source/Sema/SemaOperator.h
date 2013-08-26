
#ifndef INCLUDED_CPPPARSE_SEMA_SEMAOPERATOR_H
#define INCLUDED_CPPPARSE_SEMA_SEMAOPERATOR_H

#include "SemaCommon.h"


struct SemaOverloadableOperator : public SemaBase
{
	Name name;
	SemaOverloadableOperator(const SemaState& state)
		: SemaBase(state)
	{
	}
	template<typename T>
	SemaPolicyIdentity makePolicy(T symbol)
	{
		return SemaPolicyIdentity();
	}
	template<typename T>
	void action(T* symbol)
	{
		name = getOverloadableOperatorId(symbol);
	}
	template<LexTokenId id>
	void action(cpp::terminal<id> symbol)
	{
	}
};

struct SemaOperatorFunctionId : public SemaBase
{
	SEMA_BOILERPLATE;

	Name name;
	SemaOperatorFunctionId(const SemaState& state)
		: SemaBase(state)
	{
	}
	SEMA_POLICY(cpp::overloadable_operator, SemaPolicyPush<struct SemaOverloadableOperator>)
	void action(cpp::overloadable_operator* symbol, SemaOverloadableOperator& walker)
	{
		name = walker.name;
	}
};

#endif
