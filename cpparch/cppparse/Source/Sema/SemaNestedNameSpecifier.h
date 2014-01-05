
#ifndef INCLUDED_CPPPARSE_SEMA_SEMANESTEDNAMESPECIFIER_H
#define INCLUDED_CPPPARSE_SEMA_SEMANESTEDNAMESPECIFIER_H

#include "SemaCommon.h"
#include "SemaNamespaceName.h"
#include "SemaTypeName.h"

struct SemaNestedNameSpecifierSuffix : public SemaBase
{
	SEMA_BOILERPLATE;

	Type type;
	bool isDeclarator;
	bool isTemplate;
	SemaNestedNameSpecifierSuffix(const SemaState& state, bool isDeclarator = false)
		: SemaBase(state), type(0, context), isDeclarator(isDeclarator), isTemplate(false)
	{
	}
	void action(cpp::terminal<boost::wave::T_TEMPLATE> symbol)
	{
		isTemplate = true;
	}
	SEMA_POLICY(cpp::identifier, SemaPolicyIdentityCachedChecked)
	bool action(cpp::identifier* symbol)
	{
		LookupResultRef declaration = gDependentNestedInstance;
		if(isDeclarator
			|| !isDependentOld(qualifying_p))
		{
			declaration = lookupQualified(symbol->value, isDeclarator, IsNestedName());
			if(declaration == &gUndeclared)
			{
				return reportIdentifierMismatch(symbol, symbol->value, declaration, "nested-name");
			}
		}
		type = declaration;
		type.id = &symbol->value;
		setDecoration(&symbol->value, declaration);
		if(declaration != &gDependentNested)
		{
			setDependent(type); // a template-id is dependent if the 'identifier' is a template-parameter
		}
		return true;
	}
	SEMA_POLICY_ARGS(cpp::simple_template_id, SemaPolicyPushCachedCheckedBool<struct SemaTemplateId>, isTemplate)
	bool action(cpp::simple_template_id* symbol, const SemaTemplateIdResult& walker)
	{
		LookupResultRef declaration = gDependentNestedTemplateInstance;
		if(isDeclarator
			|| !isDependentOld(qualifying_p))
		{
			declaration = lookupQualified(*walker.id, isDeclarator, IsNestedName());
			if(declaration == &gUndeclared)
			{
				return reportIdentifierMismatch(symbol, *walker.id, declaration, "nested-name");
			}
		}
		type = declaration;
		type.id = walker.id;
		type.templateArguments = walker.arguments;
		if(declaration != &gDependentNestedTemplate)
		{
			setDependent(type); // a template-id is dependent if the 'identifier' is a template-parameter
		}
		setDependent(type.dependent, type.templateArguments); // a template-id is dependent if any of its arguments are dependent
		return true;
	}
};

// basic.lookup.qual
// During the lookup for a name preceding the :: scope resolution operator, object, function, and enumerator names are ignored.
struct SemaNestedNameSpecifierPrefix : public SemaBase
{
	SEMA_BOILERPLATE;

	Type type;
	bool isDeclarator;
	SemaNestedNameSpecifierPrefix(const SemaState& state, bool isDeclarator = false)
		: SemaBase(state), type(0, context), isDeclarator(isDeclarator)
	{
	}

#if 0 // for debugging parse-tree cache
	SEMA_POLICY_ARGS(cpp::nested_name, SemaPolicyPushCachedBool<struct SemaNestedNameSpecifierPrefix>, isDeclarator)
	void action(cpp::nested_name* symbol, const SemaNestedNameSpecifierPrefix& walker)
	{
		type = walker.type;
	}
#endif
	SEMA_POLICY(cpp::namespace_name, SemaPolicyPushChecked<struct SemaNamespaceName>) // considers only namespace names
	bool action(cpp::namespace_name* symbol, const SemaNamespaceName& walker)
	{
		if(walker.filter.hidingType != 0) // if the namespace name we found is hidden by a type name
		{
			return reportIdentifierMismatch(symbol, walker.filter.hidingType->getName(), walker.filter.hidingType, "namespace-name");
		}
		type.declaration = walker.declaration;
		return true;
	}
	SEMA_POLICY_ARGS(cpp::type_name, SemaPolicyPushCheckedBool<struct SemaTypeName>, true) // considers only type names
	bool action(cpp::type_name* symbol, const SemaTypeName& walker)
	{
		if(walker.filter.hidingNamespace != 0) // if the type name we found is hidden by a namespace name
		{
			return reportIdentifierMismatch(symbol, walker.filter.hidingNamespace->getName(), walker.filter.hidingNamespace, "type-name");
		}
		if(isDeclarator
			&& !isClass(*walker.type.declaration))
		{
			// the prefix of the nested-name-specifier in a qualified declarator-id must be a class-name (not a typedef)
			return reportIdentifierMismatch(symbol, walker.type.declaration->getName(), walker.type.declaration, "class-name");
		}
		type = walker.type;
		return true;
	}
	SEMA_POLICY(cpp::decltype_specifier, SemaPolicyPush<struct SemaDecltypeSpecifier>)
	void action(cpp::decltype_specifier* symbol, const SemaDecltypeSpecifierResult& walker)
	{
		type = walker.type;
	}
};

struct SemaNestedNameSpecifier : public SemaQualified
{
	SEMA_BOILERPLATE;

	bool isDeclarator;
	SemaNestedNameSpecifier(const SemaState& state, bool isDeclarator = false)
		: SemaQualified(state), isDeclarator(isDeclarator)
	{
	}
	SEMA_POLICY_ARGS(cpp::nested_name_specifier_prefix, SemaPolicyPushBool<struct SemaNestedNameSpecifierPrefix>, isDeclarator)
	void action(cpp::nested_name_specifier_prefix* symbol, const SemaNestedNameSpecifierPrefix& walker)
	{
		SEMANTIC_ASSERT(walker.type.declaration != 0);
		Type type = walker.type;
		makeUniqueTypeSafe(type);
		swapQualifying(type, isDeclarator);
		//disableBacktrack();
	}
	SEMA_POLICY_ARGS(cpp::nested_name_specifier_suffix_template, SemaPolicyPushBool<struct SemaNestedNameSpecifierSuffix>, isDeclarator)
	void action(cpp::nested_name_specifier_suffix_template* symbol, const SemaNestedNameSpecifierSuffix& walker)
	{
		SEMANTIC_ASSERT(walker.type.declaration != 0);
		Type type = walker.type;
		type.qualifying.swap(qualifying);
		setDependent(type.dependent, type.qualifying);
		makeUniqueTypeSafe(type);
		swapQualifying(type, isDeclarator);
		//disableBacktrack();
	}
	SEMA_POLICY_ARGS(cpp::nested_name_specifier_suffix_default, SemaPolicyPushBool<struct SemaNestedNameSpecifierSuffix>, isDeclarator)
	void action(cpp::nested_name_specifier_suffix_default* symbol, const SemaNestedNameSpecifierSuffix& walker)
	{
		SEMANTIC_ASSERT(walker.type.declaration != 0);
		Type type = walker.type;
		type.qualifying.swap(qualifying);
		setDependent(type.dependent, type.qualifying);
		makeUniqueTypeSafe(type);
		swapQualifying(type, isDeclarator);
		//disableBacktrack();
	}
};

#endif
