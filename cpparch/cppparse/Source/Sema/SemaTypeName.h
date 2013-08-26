
#ifndef INCLUDED_CPPPARSE_SEMA_SEMATYPENAME_H
#define INCLUDED_CPPPARSE_SEMA_SEMATYPENAME_H

#include "SemaCommon.h"


inline const SimpleType* findEnclosingPrimaryTemplate(const SimpleType* enclosing, Declaration* declaration)
{
	Declaration* primary = findPrimaryTemplate(declaration);
	SYMBOLS_ASSERT(primary->isTemplate);
	SYMBOLS_ASSERT(!primary->isSpecialization);
	for(const SimpleType* i = enclosing; i != 0; i = (*i).enclosing)
	{
		SYMBOLS_ASSERT(!(*i).primary->isSpecialization);
		if((*i).primary->isTemplate
			&& (*i).primary == primary)
		{
			return i;
		}
	}
	return 0;
}


// ----------------------------------------------------------------------------

struct IsHiddenTypeName
{
	DeclarationPtr nonType; // valid if the declaration is hidden by a non-type name
	DeclarationPtr hidingNamespace; // valid if the declaration is hidden by a namespace name

	IsHiddenTypeName()
		: nonType(0), hidingNamespace(0)
	{
	}

	bool operator()(const Declaration& declaration)
	{
		if(isTypeName(declaration))
		{
			return true;
		}
		if(nonType == 0
			&& isAny(declaration))
		{
			nonType = const_cast<Declaration*>(&declaration); // TODO: fix const
		}
		if(hidingNamespace == 0
			&& isNamespaceName(declaration))
		{
			hidingNamespace = const_cast<Declaration*>(&declaration); // TODO: fix const
		}
		return false;
	}
};


struct SemaTypeName : public SemaBase
{
	SEMA_BOILERPLATE;

	Type type;
	IsHiddenTypeName filter; // allows type-name to be parsed without knowing whether it is the prefix of a nested-name-specifier (in which case it cannot be hidden by a non-type name)
	bool isTypename; // true if a type is expected in this context; e.g. following 'typename', preceding '::'
	SemaTypeName(const SemaState& state, bool isTypename = false)
		: SemaBase(state), type(0, context), isTypename(isTypename)
	{
	}

	SEMA_POLICY(cpp::identifier, SemaPolicyIdentityCachedChecked)
	bool action(cpp::identifier* symbol)
	{
		LookupResultRef declaration = gDependentTypeInstance;
		if(!isDependent(qualifying_p))
		{
			declaration = findDeclaration(symbol->value, makeLookupFilter(filter));
			if(declaration == &gUndeclared)
			{
				return reportIdentifierMismatch(symbol, symbol->value, declaration, "type-name");
			}
		}
		else if(!isTypename)
		{
			// dependent type, are you missing a 'typename' keyword?
			return reportIdentifierMismatch(symbol, symbol->value, &gUndeclared, "typename");
		}
		type.id = &symbol->value;
		type.declaration = declaration;
		if(type.declaration->isTemplate)
		{
			type.isImplicitTemplateId = true; // this is either a template-name or an implicit template-id
			const SimpleType* enclosingTemplate = findEnclosingPrimaryTemplate(enclosingType, type.declaration);
			if(enclosingTemplate != 0) // if this is the name of an enclosing class-template definition (which may be an explicit/partial specialization)
			{
				type.isEnclosingClass = true; // this is an implicit template-id
				type.declaration = enclosingTemplate->declaration; // the type should refer to the enclosing class-template definition (which may be an explicit/partial specialization)
			}
		}
		setDecoration(&symbol->value, declaration);
		setDependent(type);
#if 1 // temp hack, imitate previous isDependent behaviour
		if(type.declaration->isTemplate
			&& type.declaration->templateParameter == INDEX_INVALID) // ignore template-template-parameter
		{
			if(type.declaration->isSpecialization)
			{
				setDependent(type.dependent, type.declaration->templateArguments);
			}
			else
			{
				SEMANTIC_ASSERT(!type.declaration->templateParams.empty());
				setDependent(type.dependent, *type.declaration->templateParams.front().declaration); // depend on first template param
			}
		}
#endif
		return true;
	}

	SEMA_POLICY(cpp::simple_template_id, SemaPolicyPushCachedChecked<struct SemaTemplateId>)
	bool action(cpp::simple_template_id* symbol, const SemaTemplateIdResult& walker)
	{
		LookupResultRef declaration = lookupTemplate(*walker.id, makeLookupFilter(filter));
		if(declaration == &gUndeclared)
		{
			return reportIdentifierMismatch(symbol, *walker.id, &gUndeclared, "type-name");
		}
		if(declaration == &gDependentTemplate
			&& !isTypename)
		{
			// dependent type, are you missing a 'typename' keyword?
			return reportIdentifierMismatch(symbol, *walker.id, &gUndeclared, "typename");
		}

		setDecoration(walker.id, declaration);
		type.id = walker.id;
		type.declaration = findOverloaded(declaration); // NOTE: stores the declaration from which all explicit/partial specializations are visible via 'Declaration::overloaded'
		type.templateArguments = walker.arguments;
		setDependent(type); // a template-id is dependent if the 'identifier' is a template-parameter
		setDependent(type.dependent, type.templateArguments); // a template-id is dependent if any of its arguments are dependent
		return true;
	}
};


#endif
