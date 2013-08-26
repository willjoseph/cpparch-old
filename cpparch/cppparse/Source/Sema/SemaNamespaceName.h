
#ifndef INCLUDED_CPPPARSE_SEMA_SEMANAMESPACENAME_H
#define INCLUDED_CPPPARSE_SEMA_SEMANAMESPACENAME_H


#include "SemaCommon.h"



struct IsHiddenNamespaceName
{
	DeclarationPtr hidingType; // valid if the declaration is hidden by a type name

	IsHiddenNamespaceName()
		: hidingType(0)
	{
	}

	bool operator()(const Declaration& declaration)
	{
		if(isNamespaceName(declaration))
		{
			return true;
		}
		if(hidingType == 0
			&& isTypeName(declaration))
		{
			hidingType = const_cast<Declaration*>(&declaration); // TODO: fix const
		}
		return false;
	}
};


struct SemaNamespaceName : public SemaBase
{
	SEMA_BOILERPLATE;

	LookupResultRef declaration;
	IsHiddenNamespaceName filter;
	SemaNamespaceName(const SemaState& state)
		: SemaBase(state)
	{
	}
	SEMA_POLICY(cpp::identifier, SemaPolicyIdentityCachedChecked)
	bool action(cpp::identifier* symbol)
	{
		declaration = findDeclaration(symbol->value, makeLookupFilter(filter));
		if(declaration == &gUndeclared)
		{
			return reportIdentifierMismatch(symbol, symbol->value, declaration, "namespace-name");
		}
		setDecoration(&symbol->value, declaration);
		return true;
	}
};

#endif
