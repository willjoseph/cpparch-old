
#ifndef INCLUDED_CPPPARSE_SEMA_SEMANAMESPACE_H
#define INCLUDED_CPPPARSE_SEMA_SEMANAMESPACE_H

#include "SemaCommon.h"
#include "SemaIdExpression.h"
#include "SemaNamespaceName.h"

struct SemaUsingDeclaration : public SemaQualified
{
	SEMA_BOILERPLATE;

	bool isTypename;
	SemaUsingDeclaration(const SemaState& state)
		: SemaQualified(state), isTypename(false)
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
	SEMA_POLICY(cpp::unqualified_id, SemaPolicyPushChecked<struct SemaUnqualifiedId>)
	bool action(cpp::unqualified_id* symbol, const SemaUnqualifiedId& walker)
	{
		if(!isTypename
			&& !isDependentOld(qualifying_p))
		{
			LookupResultRef declaration = walker.declaration;
			if(declaration == &gUndeclared
				|| !(isObject(*declaration) || isTypeName(*declaration)))
			{
				return reportIdentifierMismatch(symbol, *walker.id, declaration, "object-name or type-name");
			}

			setDecoration(walker.id, declaration); // refer to the primary declaration of this name, rather than the one declared by this using-declaration

			DeclarationInstance instance(declaration);
			instance.name = walker.id;
			instance.overloaded = declaration.p;
			instance.redeclared = declaration.p;
			DeclarationInstanceRef redeclaration = enclosing->declarations.insert(instance);
			enclosing->declarationList.push_back(instance);
#ifdef ALLOCATOR_DEBUG
			trackDeclaration(redeclaration);
#endif

		}
		else
		{
			// TODO: introduce typename into enclosing namespace
			setDecoration(walker.id, gDependentTypeInstance);
		}
		return true;
	}
	void action(cpp::terminal<boost::wave::T_TYPENAME> symbol)
	{
		isTypename = true;
	}
};

struct SemaUsingDirective : public SemaQualified
{
	SEMA_BOILERPLATE;

	SemaUsingDirective(const SemaState& state)
		: SemaQualified(state)
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
	// [basic.lookup.udir]
	// When looking up a namespace-name in a using-directive or namespace-alias-definition, only namespace
	// names are considered.
	SEMA_POLICY(cpp::namespace_name, SemaPolicyPush<struct SemaNamespaceName>)
	void action(cpp::namespace_name* symbol, const SemaNamespaceName& walker)
	{
		if(!findScope(enclosing, walker.declaration->enclosed))
		{
			enclosing->usingDirectives.push_back(walker.declaration->enclosed);
		}
	}
};

struct SemaNamespaceAliasDefinition : public SemaQualified
{
	SEMA_BOILERPLATE;

	IdentifierPtr id;
	SemaNamespaceAliasDefinition(const SemaState& state)
		: SemaQualified(state), id(0)
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
	SEMA_POLICY(cpp::identifier, SemaPolicyIdentityChecked)
	bool action(cpp::identifier* symbol)
	{
		if(id == 0) // first identifier
		{
			id = &symbol->value;
		}
		else // second identifier
		{
			LookupResultRef declaration = findDeclaration(symbol->value, IsNamespaceName());
			if(declaration == &gUndeclared)
			{
				return reportIdentifierMismatch(symbol, symbol->value, declaration, "namespace-name");
			}

			// TODO: check for conflicts with earlier declarations
			declaration = pointOfDeclaration(context, enclosing, *id, TYPE_NAMESPACE, declaration->enclosed);
#ifdef ALLOCATOR_DEBUG
			trackDeclaration(declaration);
#endif
			setDecoration(id, declaration);
		}
		return true;
	}
};

struct SemaNamespace : public SemaBase, SemaNamespaceResult
{
	SEMA_BOILERPLATE;

	IdentifierPtr id;
	SemaNamespace(SemaContext& context)
		: SemaBase(context), id(0)
	{
		pushScope(&context.global);
	}

	SemaNamespace(const SemaState& state)
		: SemaBase(state), id(0)
	{
	}

	SEMA_POLICY(cpp::identifier, SemaPolicyIdentity)
	void action(cpp::identifier* symbol)
	{
		id = &symbol->value;
	}
	void action(cpp::terminal<boost::wave::T_LEFTBRACE> symbol)
	{
		if(id != 0)
		{
			DeclarationInstanceRef instance = pointOfDeclaration(context, enclosing, *id, TYPE_NAMESPACE, 0);
#ifdef ALLOCATOR_DEBUG
			trackDeclaration(instance);
#endif
			setDecoration(id, instance);
			declaration = instance;
			if(declaration->enclosed == 0)
			{
				declaration->enclosed = newScope(*id, SCOPETYPE_NAMESPACE);
			}
			pushScope(declaration->enclosed);
		}
	}
	SEMA_POLICY(cpp::declaration, SemaPolicyPushTop<struct SemaDeclaration>)
	void action(cpp::declaration* symbol, const SemaDeclarationResult& walker)
	{
	}
};

#endif
