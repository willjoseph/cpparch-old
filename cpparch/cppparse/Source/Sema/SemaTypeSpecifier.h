
#ifndef INCLUDED_CPPPARSE_SEMA_SEMATYPESPECIFIER_H
#define INCLUDED_CPPPARSE_SEMA_SEMATYPESPECIFIER_H

#include "SemaCommon.h"
#include "SemaTypeName.h"

struct SemaTypeSpecifier : public SemaQualified, SemaTypeSpecifierResult
{
	SEMA_BOILERPLATE;

	SemaTypeSpecifier(const SemaState& state)
		: SemaQualified(state), SemaTypeSpecifierResult(context)
	{
	}
	SEMA_POLICY(cpp::simple_type_specifier_name, SemaPolicyPush<struct SemaTypeSpecifier>)
	void action(cpp::simple_type_specifier_name* symbol, const SemaTypeSpecifierResult& walker)
	{
		type = walker.type;
		fundamental = walker.fundamental;
	}
	SEMA_POLICY(cpp::simple_type_specifier_template, SemaPolicyPush<struct SemaTypeSpecifier>)
	void action(cpp::simple_type_specifier_template* symbol, const SemaTypeSpecifierResult& walker) // X::template Y<Z>
	{
		type = walker.type;
		fundamental = walker.fundamental;
	}
	SEMA_POLICY(cpp::type_name, SemaPolicyPushChecked<struct SemaTypeName>)
	bool action(cpp::type_name* symbol, const SemaTypeName& walker) // simple_type_specifier_name
	{
		if(walker.filter.nonType != 0)
		{
			// 3.3.7: a type-name can be hidden by a non-type name in the same scope (this rule applies to a type-specifier)
			return reportIdentifierMismatch(symbol, walker.filter.nonType->getName(), walker.filter.nonType, "type-name");
		}
		SEMANTIC_ASSERT(walker.type.declaration != 0);
		type = walker.type;
		type.qualifying.swap(qualifying);
		setDependent(type.dependent, type.qualifying);
		return true;
	}
	void action(cpp::terminal<boost::wave::T_COLON_COLON> symbol)
	{
		setQualifyingGlobal();
	}
	SEMA_POLICY(cpp::nested_name_specifier, SemaPolicyPushCached<struct SemaNestedNameSpecifier>)
	void action(cpp::nested_name_specifier* symbol, const SemaQualifyingResult& walker) // simple_type_specifier_name | simple_type_specifier_template
	{
		swapQualifying(walker.qualifying);
	}
	SEMA_POLICY(cpp::simple_template_id, SemaPolicyPushCachedChecked<struct SemaTemplateId>)
	bool action(cpp::simple_template_id* symbol, const SemaTemplateIdResult& walker) // simple_type_specifier_template
	{
		// [temp]
		// A class template shall not have the same name as any other template, class, function, variable, enumeration,
		// enumerator, namespace, or type in the same scope
		LookupResultRef declaration = lookupTemplate(*walker.id, IsAny());
		if(declaration == &gUndeclared
			|| !isTypeName(*declaration)
			|| !isTemplateName(*declaration))
		{
			return reportIdentifierMismatch(symbol, *walker.id, declaration, "class-template-name");
		}
		if(declaration == &gDependentTemplate)
		{
			// dependent type, are you missing a 'typename' keyword?
			return reportIdentifierMismatch(symbol, *walker.id, &gUndeclared, "typename");
		}
		setDecoration(walker.id, declaration);
		type.declaration = declaration;
		type.templateArguments = walker.arguments;
		type.qualifying.swap(qualifying);
		setDependent(type); // a template-id is dependent if the template-name is a template-parameter
		setDependent(type.dependent, type.templateArguments); // a template-id is dependent if any of the template arguments are dependent
		setDependent(type.dependent, type.qualifying);
		return true;
	}
	SEMA_POLICY(cpp::simple_type_specifier_builtin, SemaPolicyIdentity)
	void action(cpp::simple_type_specifier_builtin* symbol)
	{
		fundamental = combineFundamental(0, symbol->id);
	}
};



inline bool isUnqualified(cpp::elaborated_type_specifier_default* symbol)
{
	return symbol != 0
		&& symbol->isGlobal.value.empty()
		&& symbol->context.get() == 0;
}


struct SemaElaboratedTypeSpecifier : public SemaQualified, SemaElaboratedTypeSpecifierResult
{
	SEMA_BOILERPLATE;

	DeclarationPtr key;
	SemaElaboratedTypeSpecifier(const SemaState& state)
		: SemaQualified(state), SemaElaboratedTypeSpecifierResult(context), key(0)
	{
	}
	void action(cpp::terminal<boost::wave::T_COLON_COLON> symbol)
	{
		setQualifyingGlobal();
	}
	SEMA_POLICY(cpp::elaborated_type_specifier_default, SemaPolicyPush<struct SemaElaboratedTypeSpecifier>)
	void action(cpp::elaborated_type_specifier_default* symbol, const SemaElaboratedTypeSpecifierResult& walker)
	{
		type = walker.type;
		id = walker.id;

		if(!isUnqualified(symbol)
			|| !isClassKey(*type.declaration))
		{
			id = 0;
		}
	}
	SEMA_POLICY(cpp::elaborated_type_specifier_template, SemaPolicyPush<struct SemaElaboratedTypeSpecifier>)
	void action(cpp::elaborated_type_specifier_template* symbol, const SemaElaboratedTypeSpecifierResult& walker)
	{
		type = walker.type;
		id = walker.id;
	}
	SEMA_POLICY(cpp::nested_name_specifier, SemaPolicyPushCached<struct SemaNestedNameSpecifier>)
	void action(cpp::nested_name_specifier* symbol, const SemaQualifyingResult& walker) // elaborated_type_specifier_default | elaborated_type_specifier_template
	{
		swapQualifying(walker.qualifying);
	}
	SEMA_POLICY(cpp::class_key, SemaPolicyIdentityCached)
	void action(cpp::class_key* symbol)
	{
		key = &gClass;
	}
	SEMA_POLICY(cpp::enum_key, SemaPolicyIdentity)
	void action(cpp::enum_key* symbol)
	{
		key = &gEnum;
	}
	SEMA_POLICY(cpp::simple_template_id, SemaPolicyPushCachedChecked<struct SemaTemplateId>)
	bool action(cpp::simple_template_id* symbol, const SemaTemplateIdResult& walker) // elaborated_type_specifier_default | elaborated_type_specifier_template
	{
		SEMANTIC_ASSERT(key == &gClass);
		// 3.4.4-2: when looking up 'identifier' in elaborated-type-specifier, ignore any non-type names that have been declared. 
		LookupResultRef declaration = lookupTemplate(*walker.id, IsTypeName());
		if(declaration == &gUndeclared)
		{
			return reportIdentifierMismatch(symbol, *walker.id, &gUndeclared, "type-name");
		}
		if(declaration == &gDependentTemplate)
		{
			// dependent type, are you missing a 'typename' keyword?
			return reportIdentifierMismatch(symbol, *walker.id, &gUndeclared, "typename");
		}
		setDecoration(walker.id, declaration);
		id = walker.id;
		type.declaration = declaration;
		type.templateArguments = walker.arguments;
		type.qualifying.swap(qualifying);
		return true;
	}
	SEMA_POLICY(cpp::identifier, SemaPolicyIdentityCached)
	void action(cpp::identifier* symbol)
	{
		/* 3.4.4-2
		If the elaborated-type-specifier has no nested-name-specifier ...
		... the identifier is looked up according to 3.4.1 but ignoring any non-type names that have been declared. If
		the elaborated-type-specifier is introduced by the enum keyword and this lookup does not find a previously
		declared type-name, the elaborated-type-specifier is ill-formed. If the elaborated-type-specifier is introduced by
		the class-key and this lookup does not find a previously declared type-name ...
		the elaborated-type-specifier is a declaration that introduces the class-name as described in 3.3.1.
		*/
		id = &symbol->value;
		LookupResultRef declaration = findDeclaration(symbol->value, IsTypeName());
		if(declaration == &gUndeclared // if there is no existing declaration
			|| isTypedef(*declaration) // or the existing declaration is a typedef
			|| declaration->isTemplate // or the existing declaration is a template class
			|| templateParams != 0 // or we are forward-declaring a template class
			|| (key == &gClass && declaration->scope == getEtsScope())) // or this is a forward-declaration of a class/struct
		{
			if(key != &gClass)
			{
				SEMANTIC_ASSERT(key == &gEnum);
				printPosition(symbol->value.source);
				std::cout << "'" << symbol->value.value.c_str() << "': elaborated-type-specifier refers to undefined enum" << std::endl;
				throw SemanticError();
			}
			type = key;
		}
		else
		{
#if 0 // elaborated type specifier cannot refer to a template in a different scope - this case will be treated as a redeclaration
			// template<typename T> class C
			if(declaration->isSpecialization) // if the lookup found a template explicit/partial-specialization
			{
				SEMANTIC_ASSERT(declaration->isTemplate);
				declaration = findPrimaryTemplateLastDeclaration(declaration); // the name is a plain identifier, not a template-id, therefore the name refers to the primary template
			}
#endif
			setDecoration(&symbol->value, declaration);
			/* [dcl.type.elab]
			3.4.4 describes how name lookup proceeds for the identifier in an elaborated-type-specifier. If the identifier
			resolves to a class-name or enum-name, the elaborated-type-specifier introduces it into the declaration the
			same way a simple-type-specifier introduces its type-name. If the identifier resolves to a typedef-name, the
			elaborated-type-specifier is ill-formed.
			*/
#if 0 // allow hiding a typedef with a forward-declaration
			if(isTypedef(*declaration))
			{
				printPosition(symbol->value.source);
				std::cout << "'" << symbol->value.value.c_str() << "': elaborated-type-specifier refers to a typedef" << std::endl;
				printPosition(declaration->getName().source);
				throw SemanticError();
			}
#endif
			/* 7.1.6.3-3
			The class-key or enum keyword present in the elaborated-type-specifier shall agree in kind with the declaration
			to which the name in the elaborated-type-specifier refers.
			*/
			if(declaration->type.declaration != key)
			{
				printPosition(symbol->value.source);
				std::cout << "'" << symbol->value.value.c_str() << "': elaborated-type-specifier key does not match declaration" << std::endl;
				printPosition(declaration->getName().source);
				throw SemanticError();
			}
			type = declaration;
		}
	}
};


#endif
